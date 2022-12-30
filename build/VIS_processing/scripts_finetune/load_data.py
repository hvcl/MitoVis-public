import numpy as np
import os
from skimage import io


def load_data(Dir):
    category = {'axon': 0, 'axon_user': 0, 'dend_user': 1, 'dend': 1, 'mixed': 2}
    data_dict = {
        'axon'     : {},
        'axon_user': {},
        'dend'     : {},
        'dend_user': {},
        'mixed'    : {},
        'whole_ds' : {},
        'train_ds' : {}
    }
    
    len_before, len_cur = 0, 0
    len_class = [0] * len(category)
    for i, (data, label) in enumerate(category.items()):
        img = io.imread(os.path.join(Dir,f'{data}.tif'))
        idx = np.loadtxt(os.path.join(Dir, f'{data}_patch.txt')).astype('int32')
        neurite_idx = np.loadtxt(os.path.join(Dir, f'{data}_neurite.txt'))
        sort_idx = np.argsort(idx)
        
        img   = img[sort_idx]
        idx   = idx[sort_idx]
        user  = len(idx) * [True if 'user' in data else False]
        label = len(idx) * [label]
        neurite_idx = neurite_idx[sort_idx]
        
        len_cur += len(idx)
        
        len_class[i] = np.arange(len_before, len_cur)
        len_before = len_cur
        # len_class[i+1] = len(idx) + len_class[i] if i>0 else len(idx)
        
        print(data, ' ', img.shape)
        data_dict[data].update({'data': img, 'user': user, 'patch_idx': idx, 
                                'label': label, 'neurite_idx': neurite_idx})

    data_dict['whole_ds'].update({
        'data': np.concatenate((data_dict['axon']['data'], data_dict['axon_user']['data'],
                                data_dict['dend']['data'], data_dict['dend_user']['data'],
                                data_dict['mixed']['data']), axis=0),
        'user': np.concatenate((data_dict['axon']['user'], data_dict['axon_user']['user'],
                                data_dict['dend']['user'], data_dict['dend_user']['user'],
                                data_dict['mixed']['user']), axis=0),
        'patch_idx': np.concatenate((data_dict['axon']['patch_idx'], data_dict['axon_user']['patch_idx'],
                                     data_dict['dend']['patch_idx'], data_dict['dend_user']['patch_idx'],
                                     data_dict['mixed']['patch_idx']), axis=0),
        'neurite_idx': np.concatenate((data_dict['axon']['neurite_idx'], data_dict['axon_user']['neurite_idx'], 
                                       data_dict['dend']['neurite_idx'], data_dict['dend_user']['neurite_idx'], 
                                       data_dict['mixed']['neurite_idx']), axis=0),
        })

    data_dict['train_ds'].update({
        'data': np.concatenate((data_dict['axon']['data'], data_dict['axon_user']['data'],
                                data_dict['dend']['data'], data_dict['dend_user']['data'],), axis=0),
        'user': np.concatenate((data_dict['axon']['user'], data_dict['axon_user']['user'],
                                data_dict['dend']['user'], data_dict['dend_user']['user'],), axis=0),
        'label': np.concatenate((data_dict['axon']['label'], data_dict['axon_user']['label'], 
                                 data_dict['dend']['label'], data_dict['dend_user']['label']), axis=0),
        'patch_idx': np.concatenate((data_dict['axon']['patch_idx'], data_dict['axon_user']['patch_idx'], 
                                     data_dict['dend']['patch_idx'], data_dict['dend_user']['patch_idx']), axis=0),
        'neurite_idx': np.concatenate((data_dict['axon']['neurite_idx'], data_dict['axon_user']['neurite_idx'], 
                                       data_dict['dend']['neurite_idx'], data_dict['dend_user']['neurite_idx'], ), axis=0),
        })

    data_dict['axon_all'] = {
        'data': np.concatenate((data_dict['axon']['data'], data_dict['axon_user']['data']), axis=0),
        'user': np.concatenate((data_dict['axon']['user'], data_dict['axon_user']['user']), axis=0),
        'patch_idx': np.concatenate((data_dict['axon']['patch_idx'], data_dict['axon_user']['patch_idx']), axis=0),
        'neurite_idx': np.concatenate((data_dict['axon']['neurite_idx'], data_dict['axon_user']['neurite_idx']), axis=0),
        }

    data_dict['dend_all'] = {
        'data': np.concatenate((data_dict['dend']['data'], data_dict['dend_user']['data']), axis=0),
        'user': np.concatenate((data_dict['dend']['user'], data_dict['dend_user']['user']), axis=0),
        'patch_idx': np.concatenate((data_dict['dend']['patch_idx'], data_dict['dend_user']['patch_idx']), axis=0),
        'neurite_idx': np.concatenate((data_dict['dend']['neurite_idx'], data_dict['dend_user']['neurite_idx']), axis=0),
        }


    class_idxs = []
    for i in range(len(len_class)-1):
        class_idxs.append(len_class[i])
    # print(len_class[0])
    # print(len_class[1])
    # print(len_class[2])
    # print(len_class[3])
    # exit()
    size_smaller = min(len(data_dict['axon_user']['label']), len(data_dict['dend_user']['label']))
    return data_dict, size_smaller, class_idxs
