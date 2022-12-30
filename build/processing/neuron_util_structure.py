import torch
import numpy as np
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
def preprocessing(nd2Dir='./#12_3.nd2',normalize_flag='0'):

    pro_img = io.imread(nd2Dir)
    #image_size=pro_img.size
    #pro_img = np.array(pro_img.getdata())
    #pro_img = np.reshape(pro_img, image_size)

    if pro_img.ndim>2:
        pro_img = np.max(pro_img, axis=0)


    max_v=np.max(pro_img)
    print(max_v)
    if normalize_flag=='1':
        pro_img = (pro_img - np.min(pro_img)) / (max_v - np.min(pro_img))
        np.clip(pro_img, 0, 1.0, out=pro_img)
        pro_img = pro_img * 65535


    #    pro_img = (pro_img - np.min(pro_img)) / (max_v - np.min(pro_img))
#    np.clip(pro_img, 0, 1.0, out=pro_img)
#    pro_img = pro_img * 65535
    pro_img = pro_img.astype('uint16')

    return pro_img
