import numpy as np
import os
from skimage import io


def load_data(Dir):
    category = {'axon': 0, 'dend': 1, 'mixed': 2}
    data_dict = {
        'axon'    : {},
        'dend'    : {},
        'mixed'   : {},
        'whole_ds': {},
        'train_ds': {}
    }
    for i, (data, label) in enumerate(category.items()):
        img = io.imread(os.path.join(Dir,f'{data}.tif'))
        idx = np.loadtxt(os.path.join(Dir, f'{data}_patch.txt')).astype('int32')
        neurite_idx = np.loadtxt(os.path.join(Dir, f'{data}_neurite.txt'))
        sort_idx = np.argsort(idx)
        
        img   = img[sort_idx, :, :]
        idx   = idx[sort_idx]

        label = len(idx) * [label]
        neurite_idx = neurite_idx[sort_idx]
        data_dict[data].update({'data': img, 'patch_idx': idx, 'label': label, 'neurite_idx': neurite_idx})
    
    data_dict['whole_ds'].update({
        'data': np.concatenate((data_dict['axon']['data'], 
                                data_dict['dend']['data'], 
                                data_dict['mixed']['data']), axis=0),
        'patch_idx': np.concatenate((data_dict['axon']['patch_idx'], 
                                     data_dict['dend']['patch_idx'], 
                                     data_dict['mixed']['patch_idx']), axis=0),
        'neurite_idx': np.concatenate((data_dict['axon']['neurite_idx'], 
                                       data_dict['dend']['neurite_idx'], 
                                       data_dict['mixed']['neurite_idx']), axis=0),
        })
    
    data_dict['train_ds'].update({
        'data': np.concatenate((data_dict['axon']['data'], data_dict['dend']['data']), axis=0),
        'label': np.concatenate((data_dict['axon']['label'], data_dict['dend']['label']), axis=0),
        'patch_idx': np.concatenate((data_dict['axon']['patch_idx'], data_dict['dend']['patch_idx']), axis=0),
        'neurite_idx': np.concatenate((data_dict['axon']['neurite_idx'], data_dict['dend']['neurite_idx']), axis=0),
        })
    
    class_idxs = [np.arange(len(data_dict['axon']['label'])), 
                  np.arange(len(data_dict['axon']['label']), len(data_dict['axon']['label'])+len(data_dict['dend']['label']))]
    size_smaller = min(len(data_dict['axon']['label']), len(data_dict['dend']['label']))

    return data_dict, size_smaller, class_idxs