import torch
import numpy as np
import os
from tqdm import tqdm, trange
#from torch.utils.tensorboard import SummaryWriter
from torch.nn.functional import normalize
from utils import init_logger
from scripts_pretrain.patch_contrastive_loss import contrastive_loss

class Trainer():
    total_train_iter = 0
    
    def __init__(self, encoder, classifier, train_loader, axon_loader, dend_loader, mixed_loader, whole_loader,
                 optimizer_e, optimizer_c, scheduler_e, scheduler_c, cfg):
        # Config
        self.cfg = cfg
        
        # Model & Device
        self.device, device_ids = self._prepare_device(cfg.n_gpu)
        ''' encoder '''
        self.encoder = self._model_to_device(encoder, self.device, device_ids)
        self.optimizer_e = optimizer_e
        self.scheduler_e = scheduler_e
        ''' classifier '''
        self.classifier = self._model_to_device(classifier, self.device, device_ids)
        self.optimizer_c = optimizer_c
        self.scheduler_c = scheduler_c
        
        # Dataloader
        self.train_loader = train_loader
        self.axon_loader  = axon_loader
        self.dend_loader  = dend_loader
        self.mixed_loader = mixed_loader
        self.whole_loader = whole_loader
        
        # Objective functions
        self.ce_loss  = torch.nn.CrossEntropyLoss().to(self.device)

    def train_classifier(self):
        if self.cfg.mode=='no_cont':
            self.encoder.train()
        elif self.cfg.mode=='cont':
            self.encoder.eval()
            for p in self.encoder.parameters():
                p.requires_grad = False
                
        self.classifier.train()
        
        train_progress = tqdm(self.train_loader, leave=False)
        for input, label, patch_idx, neurite_idx in train_progress:
            input = input.to(self.device)
            label = label.to(self.device).long()
            patch_idx = patch_idx.to(self.device)
            neurite_idx = neurite_idx.to(self.device).long()

            self.optimizer_c.zero_grad()
            feature = self.encoder(input)
            output = self.classifier(feature)
            output = torch.nn.functional.softmax(output, dim=1)
            
            ce_loss = self.ce_loss(output, label)
            loss = ce_loss
                        
            loss.backward()
            if self.cfg.mode=='base':
                self.optimizer_e.step()
                self.scheduler_e.step()
            self.optimizer_c.step()
            self.scheduler_c.step()
            
    def train_encoder(self):
        self.encoder.train()        
        train_progress = tqdm(self.train_loader, leave=False)
        for input, label, patch_idx, neurite_idx in train_progress:
            input = input.to(self.device)
            label = label.to(self.device).long()
            patch_idx = patch_idx.to(self.device)
            neurite_idx = neurite_idx.to(self.device).long()

            self.optimizer_e.zero_grad()
            feature = self.encoder(input)            
            
            cont_loss = contrastive_loss(feature, label, self.cfg.temperature)
            loss = cont_loss
            
            loss.backward()
            self.optimizer_e.step()
            self.scheduler_e.step()
            
            
    def train(self):
        # encoder training
        if self.cfg.mode=='cont':
            progress_bar = trange(1, self.cfg.epochs_encoder+1, desc='# Feature-learning')
            for epoch in progress_bar:
                self.train_encoder()
        
        # classifier training
        progress_bar = trange(1, self.cfg.epochs_classifier+1, desc='# Training')
        for epoch in progress_bar:
            # Train
            self.train_classifier()
                        
        # Feature & Prediction save
        self.post_processing()
        self.feature_save()
        
        # param save
        torch.save({
            'encoder_state_dict': self.encoder.state_dict(),
            'classifier_state_dict': self.classifier.state_dict(),
            'optimizer_e_state_dict': self.optimizer_e.state_dict(),
            'scheduler_e_state_dict': self.scheduler_e.state_dict(),
            'optimizer_c_state_dict': self.optimizer_c.state_dict(),
            'scheduler_c_state_dict': self.scheduler_c.state_dict()
            }, f'{self.cfg.dir_log}/pretrained.pth')
            
    def load_params(self, pretrained_path):
        pretrained = torch.load(pretrained_path)
        self.encoder.load_state_dict(pretrained['encoder_state_dict'])
        self.classifier.load_state_dict(pretrained['classifier_state_dict'])
        self.optimizer_e.load_state_dict(pretrained['optimizer_e_state_dict'])
        self.scheduler_e.load_state_dict(pretrained['scheduler_e_state_dict'])
        self.optimizer_c.load_state_dict(pretrained['optimizer_c_state_dict'])
        self.scheduler_c.load_state_dict(pretrained['scheduler_c_state_dict'])
        
    def post_processing(self):
        self.pred_to_txt(self.axon_loader,  'axon')
        self.pred_to_txt(self.dend_loader,  'dend')
        self.pred_to_txt(self.mixed_loader, 'mixed')
        
    def pred_to_txt(self, loader, data_to_process='axon'):
        inference_progress = tqdm(loader, leave=False)
        self.encoder.eval()
        with torch.no_grad():
            for input, patch_idx, neurite_idx in inference_progress:
                input = input.to(self.device)
                patch_idx = patch_idx
                neurite_idx = neurite_idx
                
                feature = self.encoder(input)
                
                self.save_pred(feature, patch_idx, neurite_idx, data_to_process)                
                self.save_similarity(feature, neurite_idx, data_to_process)
    
    def save_pred(self, feature, patch_idx, neurite_idx, data_to_process='axon'):
        save_dir = f'{self.cfg.dir_log}/pred'
        os.makedirs(save_dir, exist_ok=True)

        self.classifier.eval()
        with torch.no_grad():
            output = self.classifier(feature)
            output = torch.nn.functional.softmax(output, dim=1)
            pred = torch.argmax(output, dim=1)
        
        pred_dict = {}
        pred_output = torch.cat((pred.unsqueeze(-1), output), dim=-1)
        for i in range(pred.shape[0]):
            pred_dict.update({f'{int(neurite_idx[i])} {int(patch_idx[i])}': pred_output[i].cpu()})
        
        fn = f'{save_dir}/pred.txt'
        fn_pred = open(fn, 'w') if data_to_process=='axon' else open(fn, 'a')
        
        for neurite_patch_idx, prediction in pred_dict.items():
            fn_pred.write(neurite_patch_idx + f" {int(prediction[0]):1d} {prediction[1]:.3f} {prediction[2]:.3f}")
            fn_pred.write("\n")
        fn_pred.close()
        
    def save_similarity(self, feature, neurite_idx, data_to_process='axon'):
        save_dir = f'{self.cfg.dir_log}/feature'
        os.makedirs(save_dir, exist_ok=True)
        feature = normalize(feature, p=2, dim=1)
        neu_idx = torch.unique(neurite_idx)
        feature_mean = torch.zeros((len(neu_idx), feature.shape[1]))
        feature_dict = {}
        for idx, i in enumerate(neu_idx):
            feature_mean[idx, :] = torch.mean(feature[neurite_idx==i,:], axis=0)       
            feature_dict.update({f'{int(i)}': feature_mean[idx,:]})
            
        fn_others = open(f'{save_dir}/feature_{data_to_process}.txt', 'w')
        for key in feature_dict.keys():
            fn_others.write(str(key) + " ")
            for value in feature_dict[key].numpy():
                fn_others.write(str(value) + " ")
            fn_others.write("\n")
        fn_others.close()        
    
    def feature_save(self):
        save_dir = f'{self.cfg.dir_log}/feature'
        self.encoder.eval()
        self.classifier.eval()
        inference_progress = tqdm(self.whole_loader, leave=False)
        with torch.no_grad():
            for input, patch_idx, neurite_idx in inference_progress:
                input = input.to(self.device)
                patch_idx = patch_idx.to(self.device)
                neurite_idx = neurite_idx.to(self.device)
                
                feature = self.encoder(input)
                feature = normalize(feature, p=2, dim=1)
                output = self.classifier(feature)                

                neu_idx = torch.unique(neurite_idx)
                # self.accuracy_per_neurite(output, neurite_idx, neu_idx)

                feature_mean = torch.zeros((len(neu_idx), feature.shape[1]))
                feature_dict = {}
                for idx, i in enumerate(neu_idx):
                    feature_mean[idx, :] = torch.mean(feature[neurite_idx==i,:], axis=0)
                    feature_dict.update({f'{int(i)}': feature_mean[idx,:]})
                                    
                feature_matrix = torch.matmul(feature_mean, feature_mean.T).cpu().detach().numpy()
                neu_idx = neu_idx.cpu().detach().numpy().astype('int32')
                np.savetxt(f'{save_dir}/feature_matrix.txt', feature_matrix, fmt='%.2e', newline='\n')
                np.savetxt(f'{save_dir}/neurite_idx.txt', neu_idx, fmt='%d')

    def _prepare_device(self, n_gpu):
        if not torch.cuda.is_available():
            assert False, "CUDA not available"
        device = torch.device('cuda:0')
        device_ids = list(range(n_gpu))
        return device, device_ids

    def _model_to_device(self, model, device, device_ids):
        if torch.cuda.device_count() > 1:
            print(f'GPU in use: {torch.cuda.device_count()}')
            model = torch.nn.DataParallel(model)
        model = model.to(device)
        return model
    
    def log(self, text):
        self.logger.info(text)
        
    def accuracy(self, output, label):
        pred = torch.argmax(output, dim=1)
        correct = (pred==label).sum()
        score = correct / pred.shape[0]
        return score

    def accuracy_per_neurite(self, output, neurite_idx, neu_idx):
        output = torch.nn.functional.softmax(output, dim=1)
        pred = torch.argmax(output, dim=1).unsqueeze(-1)
        output = output[pred]
        print(pred)
        print(pred.shape, output.shape)
        exit()
        pred = output[:,pred] > 0.8
        # print(pred)
        # print(pred.shape)
        exit()
        # pred