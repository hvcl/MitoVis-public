import torch
import numpy as np
from PIL import Image
import os

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
def preprocessing(fileDir='./#12_3.nd2',normalize_flag='0'):

    pro_img = Image.open(fileDir+'temp.tif')
    image_size=pro_img.size
    pro_img = np.array(pro_img.getdata())
    pro_img = np.reshape(pro_img, image_size)

    max_v=np.max(pro_img)
    print(max_v)
    if normalize_flag=='1':
        pro_img = (pro_img - np.min(pro_img)) / (max_v - np.min(pro_img))
        np.clip(pro_img, 0, 1.0, out=pro_img)
        pro_img = pro_img * 65535

    pro_img = pro_img.astype('uint16')

    user_input_img = Image.open(fileDir+'input.tif')
    user_input_img = np.array(user_input_img.getdata())
    user_input_img = np.reshape(user_input_img, image_size)
    user_input_img = user_input_img.astype('uint8')

    attention_mask = np.copy(user_input_img)
    attention_mask[user_input_img!=0] = 1
    attention_mask = np.reshape(attention_mask, (image_size[0],image_size[1],1))

    label = np.copy(user_input_img)

    label[user_input_img==10] = 0
    label[user_input_img==11] = 0
    label[user_input_img==12] = 1
    label[user_input_img==13] = 1
    label = np.reshape(label, (image_size[0],image_size[1],1))




    pre_label = Image.open(fileDir+'mitochondria_label.tif')
    pre_label = np.array(pre_label.getdata())
    pre_label = np.reshape(pre_label, image_size)
    pre_label = pre_label.astype('uint8')
    pre_label = np.reshape(pre_label, (image_size[0],image_size[1],1))


    return pro_img,label,attention_mask,pre_label
