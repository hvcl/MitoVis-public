import numpy as np
import albumentations as A
from albumentations.pytorch.transforms import ToTensorV2
from torch.utils.data import Dataset


# augment
def transform_ds(valid=False):
    # transformations = {}
    if valid==False:
        transformations = A.Compose([
            # A.ShiftScaleRotate(p=0.7),
            # A.HorizontalFlip(p=0.5),
            # A.VerticalFlip(p=0.5),
            A.RandomBrightnessContrast(p=0.5),
            A.Normalize(mean=(0.), std=(1.), p=1.0),
            ToTensorV2(p=1.0)
        ])
    elif valid==True:
        transformations = A.Compose([
            # A.Normalize(p=1.0),
            A.Normalize(mean=(0.), std=(1.), p=1.0),
            ToTensorV2(p=1.0)            
        ])
    return transformations

class NeuroTrainDataset(Dataset):
    def __init__(self, data_dict):
        super().__init__()
        self.data_dict = data_dict['train_ds']
        self.do_augment = transform_ds(valid=False)
        
    def __len__(self):
        return len(self.data_dict['label'])
    
    def __getitem__(self, idx):
        image       = self.data_dict['data'][idx,:,:]
        user        = self.data_dict['user'][idx]
        label       = self.data_dict['label'][idx]
        patch_idx   = self.data_dict['patch_idx'][idx]
        neurite_idx = self.data_dict['neurite_idx'][idx]
        
        # augmentation
        image = self.do_augment(image=image)['image']

        return image, user, label, patch_idx, neurite_idx

class NeuroCategoryDataset(Dataset):
    def __init__(self, data_dict, category):
        super().__init__()
        self.category = category
        self.data_dict = data_dict[category]
        self.do_augment = transform_ds(valid=True)
                
    def __len__(self):
        return len(self.data_dict['patch_idx'])
    
    def __getitem__(self, idx):
        image       = self.data_dict['data'][idx,:,:]
        user        = self.data_dict['user'][idx]
        patch_idx   = self.data_dict['patch_idx'][idx]
        neurite_idx = self.data_dict['neurite_idx'][idx]

        image = self.do_augment(image=image)['image']        
        
        return image, user, patch_idx, neurite_idx