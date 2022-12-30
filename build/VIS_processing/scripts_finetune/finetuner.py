import torch
import numpy as np
import os
from tqdm import tqdm, trange
#from torch.utils.tensorboard import SummaryWriter
from torch.nn.functional import normalize
from utils import init_logger
from scripts_finetune.patch_contrastive_loss import contrastive_loss_50to50 #, contrastive_loss_only_user

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
        self.ce_other_loss = torch.nn.CrossEntropyLoss().to(self.device)
        self.ce_user_loss  = torch.nn.CrossEntropyLoss().to(self.device)

        # Log
        self.iter = self.cfg.iter
        self.logger = init_logger(self.cfg.dir_log, 'finetune.log')
        self.log('\n'.join([f"{k} = {v}" for k, v in self.cfg.__dict__.items()]))
        
        
    def train_classifier(self):
        self.encoder.eval()
        for p in self.encoder.parameters():
            p.requires_grad = False
                
        self.classifier.train()
        
        train_progress = tqdm(self.train_loader, leave=False)
        for input, user, label, patch_idx, neurite_idx in train_progress:
            input = input.to(self.device)
            label = label.to(self.device).long()

            self.optimizer_c.zero_grad()
            feature = self.encoder(input)
            output = self.classifier(feature)
            output = torch.nn.functional.softmax(output, dim=1)
            
            ce_user_loss  = self.ce_user_loss(output[user==True], label[user==True])
            ce_other_loss = self.ce_other_loss(output[user==False], label[user==False])
            loss = ce_user_loss + ce_other_loss
            
            loss.backward()
            if self.cfg.mode=='no_cont':
                self.optimizer_e.step()
                self.scheduler_e.step()
            self.optimizer_c.step()
            self.scheduler_c.step()
            
    def train_encoder(self):
        self.encoder.train()
        train_progress = tqdm(self.train_loader, leave=False)
        
        for input, user, label, patch_idx, neurite_idx in train_progress:
            input = input.to(self.device)
            label = label.to(self.device).long()            

            self.optimizer_e.zero_grad()
            feature = self.encoder(input)            
            
            cont_loss = self.cfg.w_cont * contrastive_loss_50to50(feature, user, label, self.cfg.temperature)
            loss = cont_loss            
            
            loss.backward()
            self.optimizer_e.step()
            self.scheduler_e.step()

            
    def train(self):        
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
            'encoder_state_dict'    : self.encoder.state_dict(),
            'classifier_state_dict' : self.classifier.state_dict(),
            'optimizer_e_state_dict': self.optimizer_e.state_dict(),
            'scheduler_e_state_dict': self.scheduler_e.state_dict(),
            'optimizer_c_state_dict': self.optimizer_c.state_dict(),
            'scheduler_c_state_dict': self.scheduler_c.state_dict()
            }, f'{self.cfg.dir_log}/finetuned{self.iter:02d}.pth')

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
            for input, user, patch_idx, neurite_idx in inference_progress:
                input = input.to(self.device)
                patch_idx = patch_idx
                neurite_idx = neurite_idx
                
                feature = self.encoder(input)
                
                self.save_pred(feature, patch_idx, neurite_idx, data_to_process)                
                self.save_similarity(feature, user, neurite_idx, data_to_process)
    
    def save_pred(self, feature, patch_idx, neurite_idx, data_to_process='axon'):
        save_dir = f'{self.cfg.dir_log}/pred'
        os.makedirs(save_dir, exist_ok=True)
        save_dir += f'/{self.iter:02d}'
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
        
    def save_similarity(self, feature, user, neurite_idx, data_to_process='axon'):
        save_dir = f'{self.cfg.dir_log}/feature'
        os.makedirs(save_dir, exist_ok=True)
        save_dir += f'/{self.iter:02d}'
        os.makedirs(save_dir, exist_ok=True)
        
        feature = normalize(feature, p=2, dim=1)
        feature_user, feature_other = feature[user==True,:], feature[user==False,:]
        neurite_user, neurite_other = neurite_idx[user==True], neurite_idx[user==False]

        neu_idx = torch.unique(neurite_user)
        if len(neu_idx)>0:
            feature_mean = torch.zeros((len(neu_idx), feature_user.shape[1]))
            feature_dict = {}
            for idx, i in enumerate(neu_idx):
                feature_mean[idx, :] = torch.mean(feature_user[neurite_user==i,:], axis=0)       
                feature_dict.update({f'{int(i)}': feature_mean[idx,:]})
                
            fn = open(f'{save_dir}/feature_{data_to_process}_user.txt', 'w')
            for key in feature_dict.keys():
                fn.write(str(key) + " ")
                for value in feature_dict[key].numpy():
                    fn.write(str(value) + " ")
                fn.write("\n")
            fn.close()

        neu_idx = torch.unique(neurite_other)
        if len(neu_idx)>0:
            feature_mean = torch.zeros((len(neu_idx), feature_other.shape[1]))
            feature_dict = {}
            for idx, i in enumerate(neu_idx):
                feature_mean[idx, :] = torch.mean(feature_other[neurite_other==i,:], axis=0)       
                feature_dict.update({f'{int(i)}': feature_mean[idx,:]})
                
            fn = open(f'{save_dir}/feature_{data_to_process}.txt', 'w')
            for key in feature_dict.keys():
                fn.write(str(key) + " ")
                for value in feature_dict[key].numpy():
                    fn.write(str(value) + " ")
                fn.write("\n")
            fn.close()

    def feature_save(self):
        save_dir = f'{self.cfg.dir_log}/feature/{self.iter:02d}'
        self.encoder.eval()
        self.classifier.eval()
        inference_progress = tqdm(self.whole_loader, leave=False)
        with torch.no_grad():
            for input, user, patch_idx, neurite_idx in inference_progress:
                input = input.to(self.device)
                patch_idx = patch_idx.to(self.device)
                neurite_idx = neurite_idx.to(self.device)
                
                feature = self.encoder(input)
                feature = normalize(feature, p=2, dim=1)
                
                neu_idx = torch.unique(neurite_idx)
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

    def accuracy_neurite(self):
        0