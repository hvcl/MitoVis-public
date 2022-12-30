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
    attention_mask=np.concatenate((attention_mask,attention_mask,attention_mask,attention_mask),axis=2)

    label0 = np.copy(user_input_img)
    label1 = np.copy(user_input_img)
    label2 = np.copy(user_input_img)
    label3 = np.copy(user_input_img)

    label0[user_input_img==4] = 1
    label0[user_input_img!=4] = 0
    label1[user_input_img==1] = 1
    label1[user_input_img!=1] = 0
    label2[user_input_img==2] = 1
    label2[user_input_img!=2] = 0
    label3[user_input_img==3] = 1
    label3[user_input_img!=3] = 0

    label0 = np.reshape(label0, (image_size[0],image_size[1],1))
    label1 = np.reshape(label1, (image_size[0],image_size[1],1))
    label2 = np.reshape(label2, (image_size[0],image_size[1],1))
    label3 = np.reshape(label3, (image_size[0],image_size[1],1))

    label=np.concatenate((label0,label1,label2,label3),axis=2)


    pre_label = Image.open(fileDir+'structure_label.tif')
    pre_label = np.array(pre_label.getdata())
    pre_label = np.reshape(pre_label, image_size)
    pre_label = pre_label.astype('uint8')

    pre_label0 = np.copy(pre_label)
    pre_label1 = np.copy(pre_label)
    pre_label2 = np.copy(pre_label)
    pre_label3 = np.copy(pre_label)

    pre_label0[pre_label==0] = 1
    pre_label0[pre_label!=0] = 0
    pre_label1[pre_label==1] = 1
    pre_label1[pre_label!=1] = 0
    pre_label2[pre_label==2] = 1
    pre_label2[pre_label!=2] = 0
    pre_label3[pre_label==3] = 1
    pre_label3[pre_label!=3] = 0

    pre_label0 = np.reshape(pre_label0, (image_size[0],image_size[1],1))
    pre_label1 = np.reshape(pre_label1, (image_size[0],image_size[1],1))
    pre_label2 = np.reshape(pre_label2, (image_size[0],image_size[1],1))
    pre_label3 = np.reshape(pre_label3, (image_size[0],image_size[1],1))

    pre_label=np.concatenate((pre_label0,pre_label1,pre_label2,pre_label3),axis=2)



    return pro_img,label,attention_mask,pre_label
