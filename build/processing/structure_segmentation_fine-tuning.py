import os
os.environ["KMP_DUPLICATE_LIB_OK"]="TRUE"
import skimage.io
from torchvision import transforms
from neuron_util_structure_fineTune import *

import segmentation_models_pytorch as smp
import datetime
import sys
import torch.nn as nn
from torch.utils.data import DataLoader
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


class fixed_model(torch.nn.Module):
    def __init__(self,premodel=pretrain_unet(1,4)):
        super(fixed_model,self).__init__()
        self.model=premodel.model
        self.model.segmentation_head=nn.Identity()
#        print(self.model)
#        self.encoder=premodel.model.encoder
#        self.decoder=premodel.model.decoder

    def forward(self,x):
#        x = self.encoder(x)
#        x = self.decoder(x)
        x=self.model(x)
        return x


class FocusLoss(nn.Module):
    def __init__(self):
        super(FocusLoss, self).__init__()
        self.loss=torch.nn.BCELoss()
#        self.loss = torch.nn.BCELoss()
#        self.loss = torch.nn.BCEWithLogitsLoss()

    def weighted_binary_cross_entropy(self, output, target, weights=[0.2,0.8]):
        loss = weights[1] * (target * torch.log(output+0.0001)) + \
               weights[0] * ((1 - target) * torch.log(1 - output+0.0001))

        return torch.neg(torch.mean(loss))

    def forward(self, L_, U, M, L, UM):
        factor1=100
        factor2=1
        t1=torch.mul(L_,M)
        t1=torch.mul(t1,UM)
        t2=torch.mul(U,M)
        t2=torch.mul(t2,UM)
        interaction_loss=self.loss(torch.squeeze(t1),torch.squeeze(t2))
        t3=torch.mul(L_,1-M)
        t3=torch.mul(t3,UM)
        t4=torch.mul(L,1-M)
        t4=torch.mul(t4,UM)
        original_loss=self.loss(torch.squeeze(t3),torch.squeeze(t4))
        return interaction_loss*factor1+original_loss


class fineTune_model_for_background(torch.nn.Module):
    def __init__(self):
        super(fineTune_model_for_background, self).__init__()
        self.classifer1 = nn.Conv2d(in_channels=16, out_channels=1,
                                 kernel_size=3, stride=1, padding=1, bias=True)
        self.activate=nn.Sigmoid()


    def forward(self, x):
        x = self.classifer1(x)
        x=self.activate(x)
        return x


class fineTune_model(torch.nn.Module):
    def __init__(self):
        super(fineTune_model, self).__init__()
        self.classifer1 = nn.Conv2d(in_channels=16, out_channels=8,
                                 kernel_size=3, stride=1, padding=1, bias=True)
        self.classifier1_relu=nn.ReLU()

        self.classifer2 = nn.Conv2d(in_channels=8, out_channels=4,
                                 kernel_size=3, stride=1, padding=1, bias=True)
        self.classifier2_relu=nn.ReLU()

        self.classifer3 = nn.Conv2d(in_channels=4, out_channels=1,
                                 kernel_size=1, stride=1, padding=0, bias=True)

        self.activate=nn.Sigmoid()


    def forward(self, x):
        x = self.classifer1(x)
        x = self.classifier1_relu(x)
        x = self.classifer2(x)
        x = self.classifier2_relu(x)
        x = self.classifer3(x)

        x=self.activate(x)

        return x


class Dataset(torch.utils.data.Dataset):
    def __init__(self, img,label,mask,pre_label, transform=None):
        self.o_input=img
        self.o_label=label
        self.o_mask=mask
        self.o_pre_label=pre_label
        self.transform = transform

        print(label.shape)
        self.o_label1=np.zeros([label.shape[0],label.shape[1]])
        self.o_label1[label[:,:,0]==1]=0
        self.o_label1[label[:,:,2]==1]=1
        self.o_label1[label[:,:,3]==1]=1
        self.o_label1=np.reshape(self.o_label1, (label.shape[0],label.shape[1],1))

        self.o_label2=np.zeros([label.shape[0],label.shape[1]])
        self.o_label2[label[:,:,2]==1]=0
        self.o_label2[label[:,:,3]==1]=1
        self.o_label2=np.reshape(self.o_label2, (label.shape[0],label.shape[1],1))

        self.o_mask1=np.zeros([label.shape[0],label.shape[1]])
        self.o_mask1[label[:,:,0]==1]=1
        self.o_mask1[label[:,:,2]==1]=1
        self.o_mask1[label[:,:,3]==1]=1
        self.o_mask1=np.reshape(self.o_mask1, (label.shape[0],label.shape[1],1))

        self.o_mask2=np.zeros([label.shape[0],label.shape[1]])
        self.o_mask2[label[:,:,2]==1]=1
        self.o_mask2[label[:,:,3]==1]=1
        self.o_mask2=np.reshape(self.o_mask2, (label.shape[0],label.shape[1],1))

        self.o_pre_label1=np.zeros([pre_label.shape[0],pre_label.shape[1]])
        self.o_pre_label1[pre_label[:,:,0]==1]=0
        self.o_pre_label1[pre_label[:,:,2]==1]=1
        self.o_pre_label1[pre_label[:,:,3]==1]=1
        self.o_pre_label1=np.reshape(self.o_pre_label1, (pre_label.shape[0],pre_label.shape[1],1))

        self.o_pre_label2=np.zeros([pre_label.shape[0],pre_label.shape[1]])
        self.o_pre_label2[pre_label[:,:,2]==1]=0
        self.o_pre_label2[pre_label[:,:,3]==1]=1
        self.o_pre_label2=np.reshape(self.o_pre_label2, (pre_label.shape[0],pre_label.shape[1],1))

        self.o_update_mask1=np.ones([pre_label.shape[0],pre_label.shape[1]])
        self.o_update_mask1[pre_label[:,:,1]==1]=0
        self.o_update_mask1[label[:,:,1]==1]=0
        self.o_update_mask1=np.reshape(self.o_update_mask1, (pre_label.shape[0],pre_label.shape[1],1))

        self.o_update_mask2=np.copy(self.o_update_mask1)


        self.input_size=self.o_input.shape
#        print(self.input_size)

        self.data_size=0
        self.input_list1=[]
        self.label_list1=[]
        self.mask_list1=[]
        self.pre_label_list1=[]
        self.input_list2=[]
        self.label_list2=[]
        self.mask_list2=[]
        self.pre_label_list2=[]

        self.update_mask1=[]
        self.update_mask2=[]

        dataN=(int)(np.ceil(max(self.input_size[0],self.input_size[1])/256))
        # random crop
        for i in range(dataN):
            for j in range(dataN):
                start_coord=((int)(self.input_size[0]/dataN*i),(int)(self.input_size[1]/dataN*j))
                if start_coord[0]+256>self.input_size[0] or start_coord[1]+256>self.input_size[1]:
                    continue

                new_mask1=self.o_mask1[start_coord[0]:start_coord[0]+256,start_coord[1]:start_coord[1]+256,:]
                new_input1 = self.o_input[start_coord[0]:start_coord[0] + 256, start_coord[1]:start_coord[1] + 256,:]
                new_label1 = self.o_label1[start_coord[0]:start_coord[0] + 256, start_coord[1]:start_coord[1] + 256,:]
                new_pre_label1 = self.o_pre_label1[start_coord[0]:start_coord[0] + 256, start_coord[1]:start_coord[1] + 256,:]

                new_mask2=self.o_mask2[start_coord[0]:start_coord[0]+256,start_coord[1]:start_coord[1]+256,:]
                new_input2 = self.o_input[start_coord[0]:start_coord[0] + 256, start_coord[1]:start_coord[1] + 256,:]
                new_label2 = self.o_label2[start_coord[0]:start_coord[0] + 256, start_coord[1]:start_coord[1] + 256,:]
                new_pre_label2 = self.o_pre_label2[start_coord[0]:start_coord[0] + 256, start_coord[1]:start_coord[1] + 256,:]

                new_update_mask1=self.o_update_mask1[start_coord[0]:start_coord[0] + 256, start_coord[1]:start_coord[1] + 256]
                new_update_mask2=self.o_update_mask2[start_coord[0]:start_coord[0] + 256, start_coord[1]:start_coord[1] + 256]

                self.input_list1.append(new_input1)
                self.label_list1.append(new_label1)
                self.mask_list1.append(new_mask1)
                self.pre_label_list1.append(new_pre_label1)

                self.input_list2.append(new_input2)
                self.label_list2.append(new_label2)
                self.mask_list2.append(new_mask2)
                self.pre_label_list2.append(new_pre_label2)

                self.update_mask1.append(new_update_mask1)
                self.update_mask2.append(new_update_mask2)

                self.data_size=self.data_size+1

    def setMask2(self):
        dataN=(int)(np.ceil(max(self.input_size[0],self.input_size[1])/256))
        # random crop
        cnt=0
        for i in range(dataN):
            for j in range(dataN):
                start_coord=((int)(self.input_size[0]/dataN*i),(int)(self.input_size[1]/dataN*j))
                if start_coord[0]+256>self.input_size[0] or start_coord[1]+256>self.input_size[1]:
                    continue

                new_update_mask2=self.o_update_mask2[start_coord[0]:start_coord[0] + 256, start_coord[1]:start_coord[1] + 256]
                self.update_mask2[cnt]=new_update_mask2
                cnt+=1

    def __len__(self):
        return self.data_size

    # 데이터 load 파트
    def __getitem__(self, index):
        label1 = self.label_list1[index]
        input1 = self.input_list1[index]
        mask1=self.mask_list1[index]
        pre_label1=self.pre_label_list1[index]

        label2 = self.label_list2[index]
        input2 = self.input_list2[index]
        mask2=self.mask_list2[index]
        pre_label2=self.pre_label_list2[index]

        update_mask1=self.update_mask1[index]
        update_mask2=self.update_mask2[index]

        data = {'input1': input1, 'label1': label1, 'mask1': mask1, 'pre_label1': pre_label1,
                'input2': input2, 'label2': label2, 'mask2': mask2, 'pre_label2': pre_label2,
                'update_mask1':update_mask1,'update_mask2':update_mask2}

        if self.transform:
            data = self.transform(data)

        return data


class ToTensor(object):
    def __call__(self, data):
        label1, input1, mask1, pre_label1,update_mask1,label2, input2, mask2, pre_label2,update_mask2 = data['label1'], data['input1'], data['mask1'], data['pre_label1'],data['update_mask2'],data['label2'], data['input2'], data['mask2'], data['pre_label2'],data['update_mask2']

        label1 = label1.transpose((2, 0, 1)).astype(np.float32)
        input1 = input1.transpose((2, 0, 1)).astype(np.float32)
        mask1 = mask1.transpose((2, 0, 1)).astype(np.float32)
        pre_label1 = pre_label1.transpose((2, 0, 1)).astype(np.float32)
        update_mask1 = update_mask1.transpose((2, 0, 1)).astype(np.float32)

        label2 = label2.transpose((2, 0, 1)).astype(np.float32)
        input2 = input2.transpose((2, 0, 1)).astype(np.float32)
        mask2 = mask2.transpose((2, 0, 1)).astype(np.float32)
        pre_label2 = pre_label2.transpose((2, 0, 1)).astype(np.float32)
        update_mask2 = update_mask2.transpose((2, 0, 1)).astype(np.float32)

        data = {'label1': torch.from_numpy(label1), 'input1': torch.from_numpy(input1), 'mask1': torch.from_numpy(mask1),'pre_label1': torch.from_numpy(pre_label1),'update_mask1':torch.from_numpy(update_mask1),
                'label2': torch.from_numpy(label2), 'input2': torch.from_numpy(input2), 'mask2': torch.from_numpy(mask2),'pre_label2': torch.from_numpy(pre_label2),'update_mask2':torch.from_numpy(update_mask2)}

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


class structure_segmentation():
    def __init__(self,model_path='./neuron_model/',nd2file='#16_2.nd2',result_path='./neuron_model/result',normalize_flag='0'):

        self.startT=time.time()

        self.model_path = model_path
        self.network_path = self.model_path +'model/'
        self.newpath = result_path

        #set GPU
        self.device = torch.device("cpu")

        #select nd2 file
#        file_root = 'sample_file/'
        file_path=nd2file
        
        self.img,self.user_label,self.user_mask, self.pre_label = preprocessing(file_path,normalize_flag)
        self.save_dict = dict()

        #load pretrained segmentation model
        self.segmentation_model = pretrain_unet(1,4).to(self.device)

        checkpoint = torch.load(self.network_path + "model_for_structure.pt", map_location = torch.device('cpu'))

        self.segmentation_model.load_state_dict(checkpoint['gen_model'])
        self.segmentation_model.eval()

        if np.sum(self.user_mask)!=0:
            self.fixed_part=fixed_model(self.segmentation_model)
            self.fineTune_part_for_background=fineTune_model_for_background()
            self.fineTune_part=fineTune_model()
            self.loss=FocusLoss()
            self.dataset_train = Dataset(self.predeploy(self.img), self.user_label, self.user_mask, self.pre_label,
                                    transform=ToTensor())

#        self.save_fixed_feature()



    # def pretrain_unet(self,in_channel,out_channel=4):
    #     return smp.Unet('resnet34',in_channels=in_channel,classes=out_channel,activation=None,encoder_weights=None)

    def save_fixed_feature(self):
        print('-----save feature maps---------')
        fixed_features=self.predeploy(self.img)
        save_feature = dict()
        save_feature.update({'fixed_feature0': fixed_features[:,:,0],
                               'fixed_feature1': fixed_features[:,:,1],
                               'fixed_feature2': fixed_features[:,:,2],
                               'fixed_feature3': fixed_features[:,:,3],
                               'fixed_feature4': fixed_features[:,:,4],
                               'fixed_feature5': fixed_features[:,:,5],
                               'fixed_feature6': fixed_features[:,:,6],
                               'fixed_feature7': fixed_features[:,:,7],
                               'fixed_feature8': fixed_features[:,:,8],
                               'fixed_feature9': fixed_features[:,:,9],
                               'fixed_feature10': fixed_features[:,:,10],
                               'fixed_feature11': fixed_features[:,:,11],
                               'fixed_feature12': fixed_features[:,:,12],
                               'fixed_feature13': fixed_features[:,:,13],
                               'fixed_feature14': fixed_features[:,:,14],
                               'fixed_feature15': fixed_features[:,:,15]})

        for num,img in enumerate(save_feature):
            skimage.io.imsave(self.newpath+str(img)+'.tif',save_feature[img])


    def do_fineTune_for_background(self):
        if np.sum(self.user_mask)==0:
            return

        self.startT = time.time()
        print('---------Start training 1----------------')
        lr = 0.005
        optim = torch.optim.AdamW(self.fineTune_part_for_background.parameters(), lr = lr, weight_decay= 0.01)

        self.fineTune_part_for_background, optim, ver = load(ckpt_dir=self.newpath+'temp_model_structure_background/', net=self.fineTune_part_for_background, optim=optim)

        num_train = (int)(100 * (1-np.log10(ver+1)/2))
        batch_size = 8

        loader_train = DataLoader(self.dataset_train, batch_size=batch_size, shuffle=True)

        for epoch in range(1,num_train):

            if time.time()-self.startT>30:
                break

            self.fineTune_part_for_background.train()
            loss_arr = []

            for batch, data in enumerate(loader_train, 1):
                # forward
                label = data['label1'].to('cpu')
                input = data['input1'].to('cpu')
                mask = data['mask1'].to('cpu')
                pre_label = data['pre_label1'].to('cpu')
                update_mask=data['update_mask1'].to('cpu')
                output = self.fineTune_part_for_background(input)
                # backward
                optim.zero_grad()  # gradient 초기화
                loss = self.loss(output,label,mask,pre_label,update_mask)
                loss.backward()  # gradient backpropagation
                optim.step()  # backpropa 된 gradient를 이용해서 각 layer의 parameters update

                # save loss
                print('iter: ' + str(epoch) + ' (error: ' + str(loss.item())+')')
                loss_arr += [loss.item()]


        img=self.img
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


        out=self.fineTune_part_for_background(self.fixed_part(img))
        fn_tonumpy = lambda x: x.to('cpu').detach().numpy().transpose(0, 2, 3, 1)
        self.foreground_probability= np.squeeze(fn_tonumpy(out))
        fn_classifier = lambda x: 1.0 * (x > 0.5)
        out = fn_tonumpy(fn_classifier(out))

        self.o_update_mask2=out
        self.dataset_train.setMask2()

        save(ckpt_dir=self.newpath+'temp_model_structure_background/', net=self.fineTune_part_for_background, optim=optim, ver=ver+1)

    def do_fineTune(self):
        if np.sum(self.user_mask)==0:
            return

        self.startT = time.time()
        print('---------Start training 2----------------')
        lr = 0.005
        optim = torch.optim.AdamW(self.fineTune_part.parameters(), lr = lr, weight_decay= 0.01)

        self.fineTune_part, optim, ver = load(ckpt_dir=self.newpath+'temp_model_structure/', net=self.fineTune_part, optim=optim)

        num_train = (int)(100 * (1-np.log10(ver+1)/2))
        batch_size = 8
        loader_train = DataLoader(self.dataset_train, batch_size=batch_size, shuffle=True)

        for epoch in range(1,num_train):

            if time.time()-self.startT>30:
                break

            self.fineTune_part.train()
            loss_arr = []

            for batch, data in enumerate(loader_train, 1):
                # forward
                label = data['label2'].to('cpu')
                input = data['input2'].to('cpu')
                mask = data['mask2'].to('cpu')
                pre_label = data['pre_label2'].to('cpu')
                update_mask=data['update_mask2'].to('cpu')
                output = self.fineTune_part(input)
                # backward
                optim.zero_grad()  # gradient 초기화
                loss = self.loss(output,label,mask,pre_label,update_mask)
                loss.backward()  # gradient backpropagation
                optim.step()  # backpropa 된 gradient를 이용해서 각 layer의 parameters update

                # save loss
                print('iter: ' + str(epoch) + ' (error: ' + str(loss.item())+')')
                loss_arr += [loss.item()]

        save(ckpt_dir=self.newpath+'temp_model_structure/', net=self.fineTune_part, optim=optim, ver=ver+1)


    def segmentation(self):
        img = self.img
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
                t_label_image,t_probability=self.patch_deploy(img_patch)
                self.label_image[iter0:end0,iter1:end1]=t_label_image[0:end0-iter0,0:end1-iter1]
                self.probability[iter0:end0,iter1:end1,0:4]=(t_probability[0:end0-iter0,0:end1-iter1,0:4]-np.min(t_probability[0:end0-iter0,0:end1-iter1,0:4]))/(np.max(t_probability[0:end0-iter0,0:end1-iter1,0:4])-np.min(t_probability[0:end0-iter0,0:end1-iter1,0:4]))*65535


                iter1=iter1+1024

            iter0=iter0+1024


        self.save_dict.update({'structure_label_fineTune': self.label_image,
                               'probability0':np.array(self.probability[:,:,0]),
                               'probability1':np.array(self.probability[:,:,1]),
                               'probability2':np.array(self.probability[:,:,2]),
                               'probability3':np.array(self.probability[:,:,3])})

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
        out = self.fixed_part(img)
        predict=out.float()
        predict=predict.cpu().detach().numpy()
        predict=np.transpose(predict[0],[1,2,0])
        return predict


    def patch_deploy(self,img):

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
            out,out2 = self.segmentation_model(img)
            predict = out.float()
            sample = ch_channel(predict)
            sample = np.array(sample[0])
            # skimage.io.imsave('./test.tif',sample[0])
            v_pre = decode_segmap(ch_channel(predict), name='full')
            v_pre = np.transpose(v_pre[0], [1, 2, 0])

            predict2 = out.float()
            predict2 = predict2.cpu().detach().numpy()
            predict2 = np.transpose(predict2[0], [1, 2, 0])
        else:
            out = self.fineTune_part(self.fixed_part(img))
            fn_tonumpy = lambda x: x.to('cpu').detach().numpy().transpose(0, 2, 3, 1)
            fn_classifier = lambda x: 1.0 * (x > 0.5)
            out_probability = fn_tonumpy(out)
            out = fn_tonumpy(fn_classifier(out))

            probability0=1.0-self.foreground_probability
            probability2=1.0-out_probability
            probability3=out_probability

            probability1=np.zeros(probability0.shape)
            probability1[self.pre_label[:, :, 1] == 1]=1
            probability1[self.user_label[:, :, 1] == 1]=1

            probability0=np.reshape(probability0,(probability0.shape[0],probability0.shape[1],1))
            probability1=np.reshape(probability1,(probability0.shape[0],probability0.shape[1],1))
            probability2=np.reshape(probability2,(probability0.shape[0],probability0.shape[1],1))
            probability3=np.reshape(probability3,(probability0.shape[0],probability0.shape[1],1))

            predict2=np.concatenate((probability0,probability1,probability2,probability3),axis=2)

            label2=(np.squeeze(out)==0)*fn_classifier(self.foreground_probability)
            label3=np.squeeze(out)*fn_classifier(self.foreground_probability)

            sample=np.zeros(self.img.shape,'uint8')
            sample[label2==1]=2
            sample[label3==1]=3
            sample[self.pre_label[:, :, 1] == 1]=1
            sample[self.user_label[:, :, 0] == 1]=0
            sample[self.user_label[:, :, 1] == 1]=1
            sample[self.user_label[:, :, 2] == 1]=2
            sample[self.user_label[:, :, 3] == 1]=3



        return sample,predict2


    def save_image(self):
        for num,img in enumerate(self.save_dict):
            skimage.io.imsave(self.newpath+str(img)+'.tif',self.save_dict[img])


def main(argv):
    if not os.path.exists(argv[2]):
        os.makedirs(argv[2])

    print('<Structure Segmentation>')
    print('---------- Initialization ------------')
    task1 = structure_segmentation(model_path='./neuron_model/',nd2file=argv[1],result_path=argv[2],normalize_flag=argv[3]) #self,model_path='../neuron_model/',nd2file='#12_2.nd2'
    task1.do_fineTune_for_background()
    task1.do_fineTune()
    print('---------- Segmentation ------------')
    task1.segmentation()
    print('---------- Finish ------------')
    task1.save_image()

    print("-------------------------------")
    print("Done!")
    time.sleep(2)

    
if __name__ =='__main__':
    main(sys.argv)
