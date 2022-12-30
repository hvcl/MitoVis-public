import os
import torch
import random
import numpy as np
import logging
import sys
from pathlib  import Path
from datetime import datetime
from pytz     import timezone

def seed_everything(seed):
    random.seed(seed)
    os.environ['PYTHONHASHSEED'] = str(seed)
    np.random.seed(seed)
    torch.manual_seed(seed)
    torch.cuda.manual_seed(seed)
    torch.backends.cudnn.benchmark = True
    torch.backends.cudnn.enabled = True
    
def init_logger(directory, log_file_name):
    formatter = logging.Formatter('\n%(asctime)s\t%(message)s', datefmt='%m/%d/%Y %H:%M:%S')
    log_path = Path(directory, log_file_name)
    if not log_path.parent.exists():
        log_path.parent.mkdir(exist_ok=True, parents=True)
    handler = logging.FileHandler(filename=log_path)
    handler.setFormatter(formatter)

    logger = logging.getLogger(log_file_name)
    logger.setLevel(logging.INFO)
    logger.addHandler(handler)
    logger.addHandler(logging.StreamHandler(sys.stdout))
    return logger

def time_now(format):
    return datetime.now(timezone('Asia/Seoul')).strftime(format)

def create_dir(record_dir, finetune=''):
    dir_tb = f'{record_dir}/tb{finetune}'
    os.makedirs(record_dir, exist_ok=True)    
    os.makedirs(dir_tb, exist_ok=True)
    return dir_tb