import torch
import torch.nn as nn
import torch.nn.functional as F

Pool = nn.MaxPool2d(2)
Norm = nn.BatchNorm2d
Activation = nn.ReLU(inplace=True)
InsNorm = torch.nn.InstanceNorm2d

class TunerOrigModel(torch.nn.Module):
    def __init__(self):
        super().__init__()
        
        self.conv1 = nn.Conv2d(1, 16, kernel_size=11, stride=1, padding=0)
        self.norm1 = Norm(16)
        
        self.conv2 = nn.Conv2d(16, 32, kernel_size=5, stride=1, padding=0)
        self.norm2 = Norm(32)
        
        self.conv3 = nn.Conv2d(32, 64, kernel_size=5, stride=1, padding=0)
        self.norm3 = Norm(64)
        
        self.conv4 = nn.Conv2d(64, 128, kernel_size=3, stride=2, padding=0)
        self.norm4 = Norm(128)
        
        self.fc1 = nn.Linear(128, 2)
        # self.fc2 = nn.Linear(128, 2)

    def forward(self, x):
        x = Activation(self.norm1(self.conv1(x))) # [21,21] -> [11,11]
        x = Activation(self.norm2(self.conv2(x))) # [11,11] -> [7,7]
        x = Activation(self.norm3(self.conv3(x))) # [7,7]   -> [3,3]
        x = Activation(self.norm4(self.conv4(x))) # [3,3]   -> [1,1]
        
        features = x[:,:,0,0]
        outputs = self.fc1(x[:,:,0,0])
        # outputs = self.fc2(x)
        
        return outputs, features
        

class ProjectionHead(torch.nn.Module):
    def __init__(self):
        super().__init__()
        self.conv1 = nn.Conv2d(in_channels=512, out_channels=256, kernel_size=1, padding=0, bias=False)
        self.activation = nn.ReLU(inplace=True)
        self.conv2 = nn.Conv2d(in_channels=256, out_channels=128, kernel_size=1, padding=0, bias=False)
    
    def forward(self, x):
        x = self.conv1(x)
        x = self.activation(x)
        x = self.conv2(x)
        return x
    
    
class Encoder(torch.nn.Module):
    def __init__(self):
        super().__init__()
        
        self.conv1 = nn.Conv2d(1, 16, kernel_size=11, stride=1, padding=0)
        self.norm1 = Norm(16)
        
        self.conv2 = nn.Conv2d(16, 32, kernel_size=5, stride=1, padding=0)
        self.norm2 = Norm(32)
        
        self.conv3 = nn.Conv2d(32, 64, kernel_size=5, stride=1, padding=0)
        self.norm3 = Norm(64)
        
        self.conv4 = nn.Conv2d(64, 128, kernel_size=3, stride=2, padding=0)
        self.norm4 = Norm(128)
        
    def forward(self, x):
        x = Activation(self.norm1(self.conv1(x))) # [21,21] -> [11,11]
        x = Activation(self.norm2(self.conv2(x))) # [11,11] -> [7,7]
        x = Activation(self.norm3(self.conv3(x))) # [7,7]   -> [3,3]
        x = Activation(self.norm4(self.conv4(x))) # [3,3]   -> [1,1]
        
        features = x[:,:,0,0]
        
        return features

class LightEncoder(torch.nn.Module):
    def __init__(self):
        super().__init__()
        
        self.conv1 = nn.Conv2d(1, 8, kernel_size=11, stride=1, padding=0)
        self.norm1 = Norm(8)
        
        self.conv2 = nn.Conv2d(8, 16, kernel_size=5, stride=1, padding=0)
        self.norm2 = Norm(16)
        
        self.conv3 = nn.Conv2d(16, 32, kernel_size=5, stride=1, padding=0)
        self.norm3 = Norm(32)
        
        self.conv4 = nn.Conv2d(32, 64, kernel_size=3, stride=2, padding=0)
        self.norm4 = Norm(64)
        
    def forward(self, x):
        x = Activation(self.norm1(self.conv1(x))) # [21,21] -> [11,11]
        x = Activation(self.norm2(self.conv2(x))) # [11,11] -> [7,7]
        x = Activation(self.norm3(self.conv3(x))) # [7,7]   -> [3,3]
        x = Activation(self.norm4(self.conv4(x))) # [3,3]   -> [1,1]
        
        features = x[:,:,0,0]
        
        return features

class Classifier(torch.nn.Module):
    def __init__(self):
        super().__init__()
        self.fc = nn.Linear(128, 2)
    
    def forward(self, x):
        x = self.fc(x)
        return x
    

class Classifier_2layer(torch.nn.Module):
    def __init__(self):
        super().__init__()
        self.fc1 = nn.Linear(128, 32)
        self.fc2 = nn.Linear(32, 2)
    
    def forward(self, x):
        x = self.fc1(x)
        x = self.fc2(x)
        return x
    

class Classifier_4classes(torch.nn.Module):
    def __init__(self):
        super().__init__()
        self.fc1 = nn.Linear(128, 32)
        self.fc2 = nn.Linear(32, 4)
    
    def forward(self, x):
        x = self.fc1(x)
        x = self.fc2(x)
        return x