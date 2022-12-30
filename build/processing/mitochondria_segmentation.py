import os
os.environ["KMP_DUPLICATE_LIB_OK"]="TRUE"
import skimage.io
from torchvision import transforms
from neuron_util_mitochondria import *

import segmentation_models_pytorch as smp
import datetime
import sys
import torch.nn as nn
import time

class UNet(nn.Module):
    def __init__(self):
        super(UNet, self).__init__()

        def CBR2d(in_channels, out_channels, kernel_size=3, stride=1, padding=1, bias=True):
            layers = []
            layers += [nn.Conv2d(in_channels=in_channels, out_channels=out_channels,
                                 kernel_size=kernel_size, stride=stride, padding=padding,
                                 bias=bias)]
            layers += [nn.BatchNorm2d(num_features=out_channels)]
            layers += [nn.ReLU()]

            cbr = nn.Sequential(*layers)  # *으로 list unpacking

            return cbr

        # Define layers

        # Contracting path
        self.enc1_1 = CBR2d(in_channels=1, out_channels=64)
        self.enc1_2 = CBR2d(in_channels=64, out_channels=64)
        self.pool1 = nn.MaxPool2d(kernel_size=2)

        self.enc2_1 = CBR2d(in_channels=64, out_channels=128)
        self.enc2_2 = CBR2d(in_channels=128, out_channels=128)
        self.pool2 = nn.MaxPool2d(kernel_size=2)

        self.enc3_1 = CBR2d(in_channels=128, out_channels=256)
        self.enc3_2 = CBR2d(in_channels=256, out_channels=256)
        self.pool3 = nn.MaxPool2d(kernel_size=2)

        self.enc4_1 = CBR2d(in_channels=256, out_channels=512)
        self.enc4_2 = CBR2d(in_channels=512, out_channels=512)
        self.pool4 = nn.MaxPool2d(kernel_size=2)

        self.enc5_1 = CBR2d(in_channels=512, out_channels=1024)
        self.enc5_2 = CBR2d(in_channels=1024, out_channels=1024)

        # CenterCropping
        #         self.crop4 = transforms.CenterCrop(56)
        #         self.crop3 = transforms.CenterCrop(104)
        #         self.crop2 = transforms.CenterCrop(200)
        #         self.crop1 = transforms.CenterCrop(388)  # input size 512*512 기준

        # Expanding path
        self.unpool4 = nn.ConvTranspose2d(in_channels=1024, out_channels=512,
                                          kernel_size=2, stride=2, padding=0, bias=True)
        self.dec4_2 = CBR2d(in_channels=1024, out_channels=512)
        self.dec4_1 = CBR2d(in_channels=512, out_channels=512)

        self.unpool3 = nn.ConvTranspose2d(in_channels=512, out_channels=256,
                                          kernel_size=2, stride=2, padding=0, bias=True)
        self.dec3_2 = CBR2d(in_channels=512, out_channels=256)
        self.dec3_1 = CBR2d(in_channels=256, out_channels=256)

        self.unpool2 = nn.ConvTranspose2d(in_channels=256, out_channels=128,
                                          kernel_size=2, stride=2, padding=0, bias=True)
        self.dec2_2 = CBR2d(in_channels=256, out_channels=128)
        self.dec2_1 = CBR2d(in_channels=128, out_channels=128)

        self.unpool1 = nn.ConvTranspose2d(in_channels=128, out_channels=64,
                                          kernel_size=2, stride=2, padding=0, bias=True)
        self.dec1_2 = CBR2d(in_channels=128, out_channels=64)
        self.dec1_1 = CBR2d(in_channels=64, out_channels=64)

        self.conv1 = nn.Conv2d(in_channels=64, out_channels=64,
                               kernel_size=1, stride=1, padding=0, bias=True)

        self.conv1a = nn.Conv2d(in_channels=64, out_channels=16,
                                kernel_size=1, stride=1, padding=0, bias=True)

        self.conv1b = nn.Conv2d(in_channels=16, out_channels=1,
                                kernel_size=1, stride=1, padding=0, bias=True)

        self.out_1 = nn.BatchNorm2d(num_features=1)
        self.out = nn.Sigmoid()

    # forwarding
    def forward(self, x):
        enc1_1 = self.enc1_1(x)
        enc1_2 = self.enc1_2(enc1_1)
        pool1 = self.pool1(enc1_2)
        enc2_1 = self.enc2_1(pool1)
        enc2_2 = self.enc2_2(enc2_1)
        pool2 = self.pool2(enc2_2)
        enc3_1 = self.enc3_1(pool2)
        enc3_2 = self.enc3_2(enc3_1)
        pool3 = self.pool3(enc3_2)
        enc4_1 = self.enc4_1(pool3)
        enc4_2 = self.enc4_2(enc4_1)
        pool4 = self.pool4(enc4_2)
        enc5_1 = self.enc5_1(pool4)
        enc5_2 = self.enc5_2(enc5_1)

        unpool4 = self.unpool4(enc5_2)
        # concatenate croped_enc4_2
        crop4 = enc4_2  # [:, :, 4:60, 4:60]
        refine4 = torch.cat((crop4, unpool4), dim=1)  # dim 0,1,2,3 = batch,channel,width,height
        dec4_2 = self.dec4_2(refine4)
        dec4_1 = self.dec4_1(dec4_2)
        unpool3 = self.unpool3(dec4_1)
        # concatenate croped_enc3_2
        crop3 = enc3_2  # [:, :, 16:120, 16:120]
        refine3 = torch.cat((crop3, unpool3), dim=1)
        dec3_2 = self.dec3_2(refine3)
        dec3_1 = self.dec3_1(dec3_2)
        unpool2 = self.unpool2(dec3_1)
        # concatenate croped_enc2_2
        crop2 = enc2_2  # [:, :, 40:240, 40:240]
        refine2 = torch.cat((crop2, unpool2), dim=1)
        dec2_2 = self.dec2_2(refine2)
        dec2_1 = self.dec2_1(dec2_2)
        unpool1 = self.unpool1(dec2_1)
        # concatenate croped_enc1_2
        crop1 = enc1_2  # [:, :, 88:480, 88:480]
        refine1 = torch.cat((crop1, unpool1), dim=1)
        dec1_2 = self.dec1_2(refine1)
        dec1_1 = self.dec1_1(dec1_2)

        conv1 = self.conv1(dec1_1)
        conv1a = self.conv1a(conv1)
        conv1b = self.conv1b(conv1a)
        out_1 = self.out_1(conv1b)
        out = self.out(out_1)

        return out


class mitochondria_segmentation():
    def __init__(self, model_path='./DL_model/origin_mitochondria.pth', nd2file='#16_2.nd2', result_path='./neuron_model/result',normalize_flag='0'):
        self.model_path = model_path
        self.newpath = result_path

        # set GPU
        self.device = torch.device("cpu")

        # select nd2 file
        #        file_root = 'sample_file/'
        file_path = nd2file

        self.mitoimg = preprocessing(file_path,normalize_flag)
        self.save_dict = dict()


    def segmentation(self):
        # load pretrained segmentation model
        segmentation_model = UNet().to(self.device)

        dict_model = torch.load(self.model_path, map_location=torch.device('cpu'))

        segmentation_model.load_state_dict(dict_model['net'])
        segmentation_model.eval()

        img = self.mitoimg
        origin_img = img
        origin_size = (np.size(img, 0), np.size(img, 1))

        self.label_image = np.zeros([origin_size[0], origin_size[1]], 'uint8')
        self.proba_image = np.zeros([origin_size[0], origin_size[1]], 'float32')

        iter0 = 0
        while (True):
            if iter0 >= origin_size[0]:
                break

            iter1 = 0
            while (True):
                if iter1 >= origin_size[1]:
                    break

                if iter0 + 512 >= origin_size[0]:
                    start0 = iter0
                    end0 = origin_size[0]
                else:
                    start0 = iter0
                    end0 = iter0 + 512

                if iter1 + 512 >= origin_size[1]:
                    start1 = iter1
                    end1 = origin_size[1]
                else:
                    start1 = iter1
                    end1 = iter1 + 512

                img_patch = np.zeros([512, 512], origin_img.dtype)
                img_patch[0:end0 - iter0, 0:end1 - iter1] = origin_img[iter0:end0, iter1:end1]
                t_label_image,t_proba_image = self.patch_deploy(img_patch, segmentation_model)
                self.label_image[iter0:end0, iter1:end1] = t_label_image[0:end0 - iter0, 0:end1 - iter1]
                self.proba_image[iter0:end0, iter1:end1] = t_proba_image[0:end0 - iter0, 0:end1 - iter1]

                iter1 = iter1 + 512

            iter0 = iter0 + 512


        self.proba_image=(self.proba_image-np.min(self.proba_image))/(np.max(self.proba_image)-np.min(self.proba_image))
        self.proba_image=self.proba_image*65535
        self.proba_image=self.proba_image.astype('uint16')

        self.save_dict.update({'mitochondria_label': self.label_image,
                               'mitochondria_probability':self.proba_image,
                               'mitochondria_image':np.array(self.mitoimg)})
        #                               'probability0':np.array(self.probability[:,:,0]),
        #                               'probability1':np.array(self.probability[:,:,1]),
        #                               'probability2':np.array(self.probability[:,:,2]),
        #                               'probability3':np.array(self.probability[:,:,3])})

    def patch_deploy(self, img, segmentation_model):

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
        out = segmentation_model(img)

        fn_tonumpy = lambda x: x.to('cpu').detach().numpy().transpose(0, 2, 3, 1)
        out_probability = fn_tonumpy(out)
        thresh_v=(np.max(out_probability)+np.min(out_probability))*0.5
        print(thresh_v)
        fn_classifier = lambda x: 1.0 * (x > 0.5)
#        np.clip(out_probability, 0, 1.0, out=out_probability)
#        out_probability=out_probability*65535
#        out_probability = out_probability.astype(int)
        out = fn_tonumpy(fn_classifier(out))

        #        predict = out.float()
        #        sample = ch_channel(predict)
        # skimage.io.imsave('./test.tif',sample[0])

        return np.squeeze(out), np.squeeze(out_probability)

    def save_image(self):
        for num, img in enumerate(self.save_dict):
            skimage.io.imsave(self.newpath + str(img) + '.tif', self.save_dict[img])


def main(argv):
    if not os.path.exists(argv[2]):
        os.makedirs(argv[2])

    print('<Mitochondria Segmentation>')
    print('---------- Initialization ------------')
    task2 = mitochondria_segmentation(model_path=argv[4],nd2file=argv[1],result_path=argv[2],normalize_flag=argv[3])
    print('---------- Segmentation ------------')
    task2.segmentation()
    print('---------- Finish ------------')
    task2.save_image()

    print("-------------------------------")
    print("Done!")
    time.sleep(2)

    
if __name__ =='__main__':
    main(sys.argv)
