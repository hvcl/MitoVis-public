# public
import torch
import os
#os.environ["KMP_DUPLICATE_LIB_OK"] = "TRUE"
from datetime import datetime
from pytz     import timezone
from torch.optim import AdamW
from torch.utils.data import DataLoader
from torch.optim.lr_scheduler import CosineAnnealingWarmRestarts
# from github
from pytorch_balanced_sampler.sampler import SamplerFactory
# custom
from model    import Classifier, Encoder, Classifier_2layer
from scripts_pretrain.dataset  import NeuroTrainDataset, NeuroCategoryDataset
from scripts_pretrain.pretrainer import Trainer
from scripts_pretrain.load_data  import load_data
from utils    import seed_everything, create_dir


class Config():
    ''' Experiment settings '''
    seed = 42
    n_gpu = 1
    num_workers = n_gpu * 4
    ''' Params '''
    batch_size = 16392
    epochs_encoder = 100
    epochs_classifier = 100
    temperature = 0.1
    ''' Encoder '''
    lr_e = 3e-4 # 1e-3
    w_decay_e = 5e-5
    ''' Classifier '''
    lr_c = 3e-4
    w_decay_c = 5e-5
    ''' Objective function weights '''
    w_user = 1.0
    w_not_user = 0.5
    w_cont = 1.0
    ''' mode '''
    mode = 'no_cont' if w_cont==0 else 'cont'
    cont_mode = 'all' # all, intra, inter
    ''' Directory '''
    dir_ds = 'four_image_dataset'
    dir_log = f'{os.getcwd()}/log/{dir_ds}'


def main(cfg):
    # init. experiment
    cfg.start_timestamp = datetime.now(timezone('Asia/Seoul')).strftime('%y%m%d_%H%M%S')    
    if cfg.mode=='no_cont':
        cfg.dir_log = f'{cfg.dir_log}/no_cont_w{cfg.w_user}_w{cfg.w_not_user}'
    else:
        cfg.dir_log = f'{cfg.dir_log}/cont_w{cfg.w_user}_w{cfg.w_not_user}_w{cfg.w_cont}_t{cfg.temperature}'
    cfg.dir_tb = create_dir(f'{cfg.dir_log}')
    
    # seed
    seed_everything(cfg.seed)
    
    # init model
    encoder = Encoder()
    classifier = Classifier()
    # classifier = Classifier_2layer()

    # load data
    data_dict, len_train, class_idxs = load_data(Dir=f'./{cfg.dir_ds}/')
    cfg.batch_train = int(len_train) * 2

    train_ds = NeuroTrainDataset(data_dict)
    axon_ds  = NeuroCategoryDataset(data_dict, category='axon')
    dend_ds  = NeuroCategoryDataset(data_dict, category='dend')
    mixed_ds = NeuroCategoryDataset(data_dict, category='mixed')
    whole_ds = NeuroCategoryDataset(data_dict, category='whole_ds')
    
    balanced_sampler = SamplerFactory().get(
        class_idxs=class_idxs,
        batch_size=cfg.batch_train,
        n_batches=1,
        alpha=1,
        kind='random'
    )
    
    train_loader = DataLoader(train_ds, batch_sampler=balanced_sampler, num_workers=cfg.num_workers)
    axon_loader  = DataLoader(axon_ds,  batch_size=cfg.batch_size, shuffle=False, num_workers=cfg.num_workers)
    dend_loader  = DataLoader(dend_ds,  batch_size=cfg.batch_size, shuffle=False, num_workers=cfg.num_workers)
    mixed_loader = DataLoader(mixed_ds, batch_size=cfg.batch_size, shuffle=False, num_workers=cfg.num_workers)
    whole_loader = DataLoader(whole_ds, batch_size=cfg.batch_size, shuffle=False, num_workers=cfg.num_workers)

    # optimizer
    optimizer_e = AdamW(encoder.parameters(),    lr=cfg.lr_e, weight_decay=cfg.w_decay_e)
    optimizer_c = AdamW(classifier.parameters(), lr=cfg.lr_c, weight_decay=cfg.w_decay_c)
    
    # LR scheduler
    scheduler_e = CosineAnnealingWarmRestarts(optimizer_e, T_0=2, T_mult=2, eta_min=1e-6)
    scheduler_c = CosineAnnealingWarmRestarts(optimizer_c, T_0=2, T_mult=2, eta_min=1e-6)

    # train
    trainer = Trainer(encoder, classifier, train_loader, axon_loader, dend_loader, mixed_loader, whole_loader,
                      optimizer_e, optimizer_c, scheduler_e, scheduler_c, cfg)
    # pretrained_path = f'{cfg.dir_log}/pretrained.pth'
    # if os.path.isfile(pretrained_path):
    #     trainer.load_params(pretrained_path)
    # trainer.post_processing()
    # trainer.feature_save()
    trainer.train()
    
    


if __name__=='__main__':
    cfg = Config()
    main(cfg)