import torch
import numpy as np
from nd2reader import ND2Reader
from skimage import io

def decode_segmap(image, nc=4,name='full'):

    if name =='body':
        label_colors = np.array([(0, 0, 0),  # 0=background
                    # 1=cellbody, 2=dendrite, 3=axon
                    (254, 254, 0)])
    elif name =='dend':
        label_colors = np.array([(0,0,0),(254, 24, 254)])
    elif name =='axon':
        label_colors = np.array([(0,0,0), (0, 146, 146)])
    elif name == 'full':
        label_colors = np.array([(0,0,0),(254,254,0),(254,24,254),(0,146,146)])
    # print(label_colors.shape)
    r = np.zeros_like(image).astype(np.uint8)
    g = np.zeros_like(image).astype(np.uint8)
    b = np.zeros_like(image).astype(np.uint8)
    for l in range(0, nc):
        idx = image == l
        r[idx] = label_colors[l, 0]
        g[idx] = label_colors[l, 1]
        b[idx] = label_colors[l, 2]
    rgb = np.stack([r, g, b], axis=1)
    return rgb

def ch_channel(img):
    return  torch.argmax(img,dim=1).cpu().detach().numpy().astype('uint8')

##############preprocessing#################
def preprocessing(nd2Dir='./#12_3.nd2'):

    #normalizedImg = np.zeros((1024, 1024))
    #win_size = 256

    # with ND2Reader(nd2Dir) as images:
    # nd2 = nd2reader.Nd2(nd2Dir)

    #projection images
    pro_img = io.imread(nd2Dir+'/neuron_image.tif')
    pro_mito = io.imread(nd2Dir+'/mitochondria_image.tif')

#        max_v=np.min(pro_img.flatten()[np.argpartition(pro_img.flatten(), kth=-int(pro_img.flatten().size*0.01))[-int(pro_img.flatten().size*0.01):]])
#        print(max_v)
    max_v=np.max(pro_img)
    print(max_v)
    pro_img = (pro_img - np.min(pro_img)) / (max_v - np.min(pro_img))

#    o_mean=np.mean(pro_img[:])
#    o_std=np.std(pro_img[:])
#    pro_img=(pro_img-o_mean)*(0.19/o_std)
#    pro_img=pro_img+0.11
    np.clip(pro_img, 0, 1.0, out=pro_img)

    pro_img = pro_img * 65535
    pro_img = pro_img.astype('uint16')

#        max_v=np.min(pro_mito.flatten()[np.argpartition(pro_mito.flatten(), kth=-int(pro_mito.flatten().size*0.01))[-int(pro_mito.flatten().size*0.01):]])
#        print(max_v)
    max_v=np.max(pro_mito)
    print(max_v)
    pro_mito = (pro_mito - np.min(pro_mito)) / (max_v - np.min(pro_mito))

#    o_mean=np.mean(pro_mito[:])
#    o_std=np.std(pro_mito[:])
#    pro_mito=(pro_mito-o_mean)*(0.0725/o_std)
#    pro_mito=pro_mito+0.0215
#    np.clip(pro_mito, 0, 1.0, out=pro_mito)

    np.clip(pro_mito, 0, 1.0, out=pro_mito)
    pro_mito = pro_mito * 65535
    pro_mito = pro_mito.astype('uint16')

    return pro_img, pro_mito
