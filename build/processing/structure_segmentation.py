import os
os.environ["KMP_DUPLICATE_LIB_OK"]="TRUE"
import skimage.io
from torchvision import transforms
from neuron_util_structure import *

import segmentation_models_pytorch as smp
import datetime
import sys
import torch.nn as nn
import time


class pretrain_unet(torch.nn.Module):
    def __init__(self,in_channels=1,classes=4,active='sigmoid'):
        super(pretrain_unet,self).__init__()
        self.model = smp.Unet('resnet34',in_channels=1,classes=classes,activation=None,encoder_weights=None)
        if active == 'softmax':
            self.sigmoid = torch.nn.Softmax(dim=1)
        elif active == 'sigmoid':
            self.sigmoid = torch.nn.Sigmoid()
    def forward(self,x):
        x = self.model(x)
        result = self.sigmoid(x)
        return result,x

    def forward_for_feature(self,x):
        latent=self.model.ResNetEncoder(x)
        recon=self.model.UnetDecoder(latent)
        return recon

class structure_segmentation():
    def __init__(self,model_path='./DL_model/origin_structure.pt',nd2file='#16_2.nd2',result_path='./neuron_model/result',normalize_flag='0'):
        self.model_path = model_path
        self.newpath = result_path

        #set GPU
        self.device = torch.device("cpu")
    
        #select nd2 file
#        file_root = 'sample_file/'
        file_path=nd2file
        
        print('----- Preprocessing loading-------------')
        self.patchimg = preprocessing(file_path,normalize_flag)
        self.save_dict = dict()
        
    # def pretrain_unet(self,in_channel,out_channel=4):
    #     return smp.Unet('resnet34',in_channels=in_channel,classes=out_channel,activation=None,encoder_weights=None)

    def segmentation(self):
        #load pretrained segmentation model
        print('----- Structure Segmentation model loading-------------')
        segmentation_model = pretrain_unet(1,4).to(self.device)

        checkpoint = torch.load(self.model_path, map_location = torch.device('cpu'))

        segmentation_model.load_state_dict(checkpoint['gen_model'])
        segmentation_model.eval()

        img = self.patchimg
        origin_img = img
        origin_size=(np.size(img,0),np.size(img,1))

        self.full_image=np.zeros([origin_size[0],origin_size[1],3],'uint8')
        self.label_image=np.zeros([origin_size[0],origin_size[1]],'uint8')
        self.probability=np.zeros([origin_size[0],origin_size[1],4],'uint16')

        iter0=0
        while(True):
            if iter0>=origin_size[0]:
                break

            iter1 = 0
            while(True):
                if iter1>=origin_size[1]:
                    break

                if iter0+1024>=origin_size[0]:
                    start0=iter0
                    end0=origin_size[0]
                else:
                    start0=iter0
                    end0=iter0+1024

                if iter1 + 1024 >= origin_size[1]:
                    start1 = iter1
                    end1 = origin_size[1]
                else:
                    start1 = iter1
                    end1 = iter1 + 1024

                img_patch = np.zeros([1024, 1024],origin_img.dtype)
                img_patch[0:end0-iter0,0:end1-iter1]=origin_img[iter0:end0,iter1:end1]
                t_full_image,t_label_image,t_probability=self.patch_deploy(img_patch,segmentation_model)
                self.full_image[iter0:end0,iter1:end1,0:3]=t_full_image[0:end0-iter0,0:end1-iter1,0:3]
                self.label_image[iter0:end0,iter1:end1]=t_label_image[0:end0-iter0,0:end1-iter1]
                self.probability[iter0:end0,iter1:end1,0:4]=(t_probability[0:end0-iter0,0:end1-iter1,0:4]-np.min(t_probability[0:end0-iter0,0:end1-iter1,0:4]))/(np.max(t_probability[0:end0-iter0,0:end1-iter1,0:4])-np.min(t_probability[0:end0-iter0,0:end1-iter1,0:4]))*65535


                iter1=iter1+1024

            iter0=iter0+1024


        self.save_dict.update({'structure_image':self.full_image,
                               'structure_label': self.label_image,
                               'neuron_image':np.array(origin_img)})
#                               'probability0':np.array(self.probability[:,:,0]),
#                               'probability1':np.array(self.probability[:,:,1]),
#                               'probability2':np.array(self.probability[:,:,2]),
#                               'probability3':np.array(self.probability[:,:,3])})

    def patch_deploy(self,img,segmentation_model):

        # change torch datatype

        if img.dtype == 'uint16':
            self.L2_transform = transforms.Compose([
                transforms.Lambda(lambda image: torch.from_numpy(np.array(image).astype(np.float32)).unsqueeze(0)),
                transforms.Normalize([0], [65535])])
        else:
            self.L2_transform = transforms.Compose([
                transforms.Lambda(lambda image: torch.from_numpy(np.array(image).astype(np.float32)).unsqueeze(0)),
                transforms.Normalize([0], [255])])
        img = self.L2_transform(img)

        # add batch axis
        img = img.unsqueeze(0)

        # evaluation
        out, out2 = segmentation_model(img)

        predict2=out.float()

        predict = out.float()
        sample = ch_channel(predict)
        # skimage.io.imsave('./test.tif',sample[0])
        v_pre = decode_segmap(ch_channel(predict), name='full')

        # update images
        v_pre = np.transpose(v_pre[0], [1, 2, 0])

        predict2=predict2.cpu().detach().numpy()
        predict2=np.transpose(predict2[0],[1,2,0])

        return np.array(v_pre),np.array(sample[0]),predict2


    def save_image(self):
        for num,img in enumerate(self.save_dict):
            skimage.io.imsave(self.newpath+str(img)+'.tif',self.save_dict[img])


def main(argv):
    if not os.path.exists(argv[2]):
        os.makedirs(argv[2])
    print('<Structure Segmentation>')
    print('---------- Initialization ------------')
    task1 = structure_segmentation(model_path=argv[4],nd2file=argv[1],result_path=argv[2],normalize_flag=argv[3]) #self,model_path='../neuron_model/',nd2file='#12_2.nd2'
    print('---------- Segmentation ------------')
    task1.segmentation()
    print('---------- Finish ------------')
    task1.save_image()

    print("-------------------------------")
    print("Done!")
    time.sleep(2)
    
if __name__ =='__main__':
    main(sys.argv)
