# public
import torch
import os
os.environ["KMP_DUPLICATE_LIB_OK"] = "TRUE"
# from datetime import datetime
# from pytz     import timezone
import time
from torch.optim import AdamW
from torch.utils.data import DataLoader
from torch.optim.lr_scheduler import CosineAnnealingWarmRestarts
# from github
from pytorch_balanced_sampler.sampler import SamplerFactory
# custom
from model    import Classifier, Encoder
from scripts_finetune.dataset  import NeuroTrainDataset, NeuroCategoryDataset
from scripts_finetune.finetuner import Trainer
from scripts_finetune.load_data  import load_data
from utils    import seed_everything, create_dir
import sys


class Config():
    ''' Experiment settings '''
    seed = 42
    n_gpu = 1
    num_workers = 8 # n_gpu * 4
    ''' Params '''
    batch_size = 16392
    epochs_encoder = 0
    epochs_classifier = 1
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
    dir_ds = f'{sys.argv[1]}/input_dataset'
    dir_log = f'{sys.argv[1]}/output_dataset'


def main(cfg):
    # init. experiment
    cfg.iter = int(sys.argv[2])

    if cfg.iter==1:
        pretrained_path = f'./VIS_processing/pretrained.pth'
    else:
        pretrained_path = f'./{sys.argv[1]}/output_dataset/finetuned{cfg.iter-1:02d}.pth'
        print(pretrained_path)
    
    cfg.dir_tb = create_dir(f'{cfg.dir_log}', finetune='_tune')
    # seed
    seed_everything(cfg.seed)
    
    # init model
    encoder = Encoder()
    classifier = Classifier()

    # load data
    Dir = f'./{cfg.dir_ds}'
    data_dict, len_train, class_idxs = load_data(Dir=Dir)
    cfg.batch_train = int(len_train) * 4
    # cfg.batch_train = 816
    train_ds = NeuroTrainDataset(data_dict)
    axon_ds  = NeuroCategoryDataset(data_dict, category='axon_all')
    dend_ds  = NeuroCategoryDataset(data_dict, category='dend_all')
    mixed_ds = NeuroCategoryDataset(data_dict, category='mixed')
    whole_ds = NeuroCategoryDataset(data_dict, category='whole_ds')

    balanced_sampler = SamplerFactory().get(
        class_idxs=class_idxs,
        batch_size=cfg.batch_train,
        n_batches=1,
        alpha=1,
        kind='fixed'
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
    scheduler_e = CosineAnnealingWarmRestarts(optimizer_e, T_0=10, eta_min=1e-6)
    scheduler_c = CosineAnnealingWarmRestarts(optimizer_c, T_0=10, eta_min=1e-6)

    # train
    trainer = Trainer(encoder, classifier, train_loader, axon_loader, dend_loader, mixed_loader, whole_loader, optimizer_e, optimizer_c, scheduler_e, scheduler_c, cfg)
    if os.path.isfile(pretrained_path):
        print('Loading trained model')
        trainer.load_params(pretrained_path)
    trainer.train()
    # trainer.post_processing()


if __name__=='__main__':
    cfg = Config()
    main(cfg)