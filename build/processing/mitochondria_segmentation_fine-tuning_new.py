import os

os.environ["KMP_DUPLICATE_LIB_OK"] = "TRUE"
import skimage.io
from torchvision import transforms
from neuron_util_mitochondria_fineTune import *

import segmentation_models_pytorch as smp
import datetime
import sys
import torch.nn as nn
import time
from torch.utils.data import DataLoader

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

    def fixed_forward(self, x):
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

        return dec1_1





class FocusLoss(nn.Module):
    def __init__(self):
        super(FocusLoss, self).__init__()
#        self.loss = torch.nn.MSELoss()
        self.loss = torch.nn.BCELoss()

    def forward(self, L_, U, M, L):
#        factor1=torch.sum(1-mask)
#        factor2=torch.sum(mask)
        factor1=100
        factor2=1
        t1=torch.mul(L_,M)
        t2=torch.mul(U,M)
        interaction_loss=self.loss(torch.squeeze(t1),torch.squeeze(t2))
        t3=torch.mul(L_,1-M)
        t4=torch.mul(L,1-M)
        original_loss=self.loss(torch.squeeze(t3),torch.squeeze(t4))
        return interaction_loss*factor1+original_loss



class fineTune_model(torch.nn.Module):
    def __init__(self):
        super(fineTune_model, self).__init__()
        self.classifer1 = nn.Conv2d(in_channels=64, out_channels=16,
                                 kernel_size=3, stride=1, padding=1, bias=True)
        self.classifier1_batNorm=nn.BatchNorm2d(num_features=16)
        self.classifier1_relu=nn.ReLU()

        self.classifer2 = nn.Conv2d(in_channels=16, out_channels=4,
                                 kernel_size=3, stride=1, padding=1, bias=True)
        self.classifier2_batNorm=nn.BatchNorm2d(num_features=4)
        self.classifier2_relu=nn.ReLU()

        self.classifer3 = nn.Conv2d(in_channels=4, out_channels=1,
                                 kernel_size=3, stride=1, padding=1, bias=True)
        self.classifier3_batNorm=nn.BatchNorm2d(num_features=1)
        self.classifier3_relu=nn.ReLU()


        self.classifer4 = nn.Conv2d(in_channels=4, out_channels=1,
                                 kernel_size=3, stride=1, padding=1, bias=True)
        self.classifier4_batNorm=nn.BatchNorm2d(num_features=1)
        self.classifier4_relu=nn.ReLU()

        self.activate=nn.Sigmoid()


    def forward(self, x):
        x = self.classifer1(x)
#        x = self.classifier1_batNorm(x)
        x = self.classifier1_relu(x)
        x = self.classifer2(x)
#        x = self.classifier2_batNorm(x)
        x = self.classifier2_relu(x)
        x = self.classifer3(x)
#        x = self.classifier3_batNorm(x)
#        x = self.classifier3_relu(x)
#        x = self.classifer4(x)
#        x = self.classifier4_batNorm(x)
        x=self.activate(x)

        return x


class Dataset(torch.utils.data.Dataset):
    def __init__(self, img,label,mask,pre_label, transform=None):
        self.o_input=img
        self.o_label=label
        self.o_mask=mask
        self.o_pre_label=pre_label
        self.transform = transform

        self.input_size=self.o_input.shape
#        print(self.input_size)

        self.data_size=0
        self.input_list=[]
        self.label_list=[]
        self.mask_list=[]
        self.pre_label_list=[]
        dataN=(int)(np.ceil(max(self.input_size[0],self.input_size[1])/256))
        # random crop
        for i in range(dataN):
            for j in range(dataN):
                start_coord=((int)(self.input_size[0]/dataN*i),(int)(self.input_size[1]/dataN*j))
                if start_coord[0]+256>self.input_size[0] or start_coord[1]+256>self.input_size[1]:
                    continue

                new_mask=mask[start_coord[0]:start_coord[0]+256,start_coord[1]:start_coord[1]+256,:]

                new_input = img[start_coord[0]:start_coord[0] + 256, start_coord[1]:start_coord[1] + 256,:]
                new_label = label[start_coord[0]:start_coord[0] + 256, start_coord[1]:start_coord[1] + 256,:]
                new_pre_label = pre_label[start_coord[0]:start_coord[0] + 256, start_coord[1]:start_coord[1] + 256,:]
                self.input_list.append(new_input)
                self.label_list.append(new_label)
                self.mask_list.append(new_mask)
                self.pre_label_list.append(new_pre_label)
                self.data_size=self.data_size+1


    def __len__(self):
        return self.data_size

    # 데이터 load 파트
    def __getitem__(self, index):
        label = self.label_list[index]
        input = self.input_list[index]
        mask=self.mask_list[index]
        pre_label=self.pre_label_list[index]

        label = label.astype(np.float32)
        mask = mask.astype(np.float32)

        data = {'input': input, 'label': label, 'mask': mask, 'pre_label': pre_label}

        if self.transform:
            data = self.transform(data)

        return data


class ToTensor(object):
    def __call__(self, data):
        label, input, mask, pre_label = data['label'], data['input'], data['mask'], data['pre_label']

        label = label.transpose((2, 0, 1)).astype(np.float32)
        input = input.transpose((2, 0, 1)).astype(np.float32)
        mask = mask.transpose((2, 0, 1)).astype(np.float32)
        pre_label = pre_label.transpose((2, 0, 1)).astype(np.float32)

        data = {'label': torch.from_numpy(label), 'input': torch.from_numpy(input), 'mask': torch.from_numpy(mask),'pre_label': torch.from_numpy(pre_label)}

        return data


def save(ckpt_dir, net, optim, ver):
    if not os.path.exists(ckpt_dir):
        os.makedirs(ckpt_dir)

    torch.save({'net': net.state_dict(), 'optim': optim.state_dict()}, '%s/model_ver%d.pth' % (ckpt_dir, ver))


# 네트워크 불러오기
def load(ckpt_dir, net, optim):
    if not os.path.exists(ckpt_dir):  # 저장된 네트워크가 없다면 인풋을 그대로 반환
        ver = 0
        return net, optim, ver

    ckpt_lst = os.listdir(ckpt_dir)  # ckpt_dir 아래 있는 모든 파일 리스트를 받아온다
    if ckpt_lst==[]:
        ver = 0
        return net, optim, ver


    ckpt_lst.sort(key=lambda f: int(''.join(filter(str.isdigit, f))))

    dict_model = torch.load('%s/%s' % (ckpt_dir, ckpt_lst[-1]))

    net.load_state_dict(dict_model['net'])
    optim.load_state_dict(dict_model['optim'])
    ver = int(ckpt_lst[-1].split('ver')[1].split('.pth')[0])

    return net, optim, ver








class mitochondria_segmentation():
    def __init__(self, model_path='./neuron_model/', nd2file='#16_2.nd2', result_path='./neuron_model/result',
                 normalize_flag='0'):

        self.startT = time.time()

        self.model_path = model_path
        self.network_path = self.model_path + 'model/'
        self.newpath = result_path

        # set GPU
        self.device = torch.device("cpu")

        # select nd2 file
        #        file_root = 'sample_file/'
        file_path = nd2file

        self.segmentation_model = UNet().to(self.device)

        dict_model = torch.load(self.network_path + "model_for_mitochondria.pth", map_location=torch.device('cpu'))

        self.segmentation_model.load_state_dict(dict_model['net'])
        self.segmentation_model.eval()

        self.fineTune_part=fineTune_model()
        self.loss=FocusLoss()

        print('----- Preprocessing loading-------------')
        self.img,self.user_label,self.user_mask, self.pre_label = preprocessing(file_path, normalize_flag)
        self.save_dict = dict()


    def do_fineTune(self):
        if np.sum(self.user_mask)==0:
            return
        print('---------Start training----------------')
        lr = 0.005
        optim = torch.optim.AdamW(self.fineTune_part.parameters(), lr = lr, weight_decay= 0.01)

        self.fineTune_part, optim, ver = load(ckpt_dir=self.newpath+'temp_model_mito/', net=self.fineTune_part, optim=optim)

        num_train = (int)(100 * (1-np.log10(ver+1)/2))
        batch_size = 8
        dataset_train = Dataset(self.predeploy(self.img),self.user_label,self.user_mask,self.pre_label, transform=ToTensor())

        loader_train = DataLoader(dataset_train, batch_size=batch_size, shuffle=True)

        for epoch in range(1,num_train):
            if time.time()-self.startT>55:
                break

            self.fineTune_part.train()
            loss_arr = []

            for batch, data in enumerate(loader_train, 1):
                # forward
                label = data['label'].to('cpu')
                input = data['input'].to('cpu')
                mask = data['mask'].to('cpu')
                pre_label = data['pre_label'].to('cpu')
                output = self.fineTune_part(input)
                # backward
                optim.zero_grad()  # gradient 초기화
                loss = self.loss(output,label,mask,pre_label)
                loss.backward()  # gradient backpropagation
                optim.step()  # backpropa 된 gradient를 이용해서 각 layer의 parameters update

                # save loss
                print('iter: ' + str(epoch) + ' (error: ' + str(loss.item())+')')
                loss_arr += [loss.item()]

        save(ckpt_dir=self.newpath+'temp_model_mito/', net=self.fineTune_part, optim=optim, ver=ver+1)





    def segmentation(self):
        # load pretrained segmentation model
        print('----- Segmentation model loading-------------')


        img = self.img
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
                t_label_image, t_proba_image = self.patch_deploy(img_patch)
                self.label_image[iter0:end0, iter1:end1] = t_label_image[0:end0 - iter0, 0:end1 - iter1]
                self.proba_image[iter0:end0, iter1:end1] = t_proba_image[0:end0 - iter0, 0:end1 - iter1]

                iter1 = iter1 + 512

            iter0 = iter0 + 512

        self.proba_image = (self.proba_image - np.min(self.proba_image)) / (
                    np.max(self.proba_image) - np.min(self.proba_image))
        self.proba_image = self.proba_image * 65535
        self.proba_image = self.proba_image.astype('uint16')

        self.save_dict.update({'mitochondria_label_fineTune': self.label_image,
                               'mitochondria_probability': self.proba_image})
        #                               'probability0':np.array(self.probability[:,:,0]),
        #                               'probability1':np.array(self.probability[:,:,1]),
        #                               'probability2':np.array(self.probability[:,:,2]),
        #                               'probability3':np.array(self.probability[:,:,3])})



    def predeploy(self,img):

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
        out = self.segmentation_model.fixed_forward(img)
        predict=out.float()
        predict=predict.cpu().detach().numpy()
        predict=np.transpose(predict[0],[1,2,0])
        return predict


    def patch_deploy(self, img):

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
        if np.sum(self.user_mask)==0:
            out = self.segmentation_model(img)
        else:
            out = self.fineTune_part(self.segmentation_model.fixed_forward(img))


        fn_tonumpy = lambda x: x.to('cpu').detach().numpy().transpose(0, 2, 3, 1)
        out_probability = fn_tonumpy(out)
        thresh_v = (np.max(out_probability) + np.min(out_probability)) * 0.5
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

    task2 = mitochondria_segmentation(model_path='./neuron_model/', nd2file=argv[1], result_path=argv[2],
                                      normalize_flag=argv[3])
    task2.do_fineTune()
    task2.segmentation()
    task2.save_image()

    print("finish!")
    time.sleep(2)


if __name__ == '__main__':
    main(sys.argv)
