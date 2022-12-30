import numpy as np
from skimage import io


def load(Dir='../patch_based_dataset_test1'):
    axon = io.imread(Dir+'/axon.tif')
    axon_neurite=np.loadtxt(Dir+'/axon_neurite.txt', dtype='int', delimiter='\n')

    ind=np.argsort(axon_neurite)
    axon=axon[ind,:,:]
    axon_neurite=axon_neurite[ind]


    axon_user = io.imread(Dir+'/axon_user.tif')
    axon_user_neurite=np.loadtxt(Dir+'/axon_user_neurite.txt', dtype='int', delimiter='\n')


#    io.imsave(Dir+'/temp.tif',axon_user)

    ind=np.argsort(axon_user_neurite)
    axon_user=axon_user[ind,:,:]
    axon_user_neurite=axon_user_neurite[ind]

    mixed = io.imread(Dir + '/mixed.tif')
    mixed_neurite=np.loadtxt(Dir+'/mixed_neurite.txt', dtype='int', delimiter='\n')

    ind=np.argsort(mixed_neurite)
    mixed=mixed[ind,:,:]
    mixed_neurite=mixed_neurite[ind]


    dend = io.imread(Dir+'/dend.tif')
    dend_neurite=np.loadtxt(Dir+'/dend_neurite.txt', dtype='int', delimiter='\n')

    ind=np.argsort(dend_neurite)
    dend=dend[ind,:,:]
    dend_neurite=dend_neurite[ind]


    dend_user = io.imread(Dir+'/dend_user.tif')
    dend_user_neurite=np.loadtxt(Dir+'/dend_user_neurite.txt', dtype='int', delimiter='\n')

    ind=np.argsort(dend_user_neurite)
    dend_user=dend_user[ind,:,:]
    dend_user_neurite=dend_user_neurite[ind]

    # 특정 index neurite 추출하기!
    '''    
    target_index=123
    ind = np.where(axon_neurite == target_index)
    neurite = axon[ind]

    ind = np.where(axon_user_neurite == target_index)
    neurite=np.append(neurite,axon_user[ind],axis=0)

    ind = np.where(dend_neurite == target_index)
    neurite=np.append(neurite,dend[ind],axis=0)

    ind = np.where(dend_user_neurite == target_index)
    neurite=np.append(neurite,dend_user[ind],axis=0)

    ind = np.where(mixed_neurite == target_index)
    neurite=np.append(neurite,mixed[ind],axis=0)

    io.imsave(Dir + '/temp.tif', neurite)
    '''

    original_image=io.imread(Dir+'/neuron_image.tif')
    initial_label=io.imread(Dir+'/structure_label_initial.tif')
    gt_label=io.imread(Dir+'/structure_label_gt.tif')
    probability_map=[io.imread(Dir+'/probability0.tif'),io.imread(Dir+'/probability1.tif'),io.imread(Dir+'/probability2.tif'),io.imread(Dir+'/probability3.tif')]
    probability_map=np.array(probability_map)

    res={"axon":axon,"axon_neurite":axon_neurite,"axon_user":axon_user,"axon_user_neurite":axon_user_neurite,"dend":dend,"dend_neurite":dend_neurite,
         "dend_user":dend_user,"dend_user_neurite":dend_user_neurite,"mixed":mixed,"mixed_neurite":mixed_neurite,
         "original_image":original_image,"initial_label":initial_label,"gt_label":gt_label,"probability_map":probability_map}
    return res