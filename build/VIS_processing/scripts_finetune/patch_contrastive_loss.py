import torch
from torch.nn.functional import normalize


def contrastive_loss_50to50(feature, user, label, temperature):
    other_feature, user_feature = feature[user==False, :], feature[user==True, :]
    other_label, user_label = label[user==False], label[user==True]
    
    feature = {
        'axon': normalize(other_feature[other_label==0, :], p=2 , dim=1),
        'dend': normalize(other_feature[other_label==1, :], p=2 , dim=1),
        'axon_user': normalize(user_feature[user_label==0, :], p=2 , dim=1),
        'dend_user': normalize(user_feature[user_label==1, :], p=2 , dim=1)
    }

    axon_all = torch.cat((feature['axon'], feature['axon_user']), dim=0)
    dend_all = torch.cat((feature['dend'], feature['dend_user']), dim=0)
    
    num_axon  = axon_all.shape[0]
    mask_axon = torch.eye(num_axon, dtype=torch.bool)
    axon_axon = torch.matmul(axon_all, axon_all.T)[~mask_axon].reshape(num_axon, -1) / temperature
    axon_dend = torch.matmul(axon_all, dend_all.T) / temperature
    axon_axon_exp = torch.exp(axon_axon)
    axon_dend_exp = torch.sum(
        torch.exp(axon_dend), dim=1, keepdim=True
    )
    loss_axon_axon = -torch.mean(axon_axon - torch.log(axon_axon_exp + axon_dend_exp))
    
    num_dend  = dend_all.shape[0]
    mask_dend = torch.eye(num_dend, dtype=torch.bool)
    dend_dend = torch.matmul(dend_all, dend_all.T)[~mask_dend].reshape(num_dend, -1) / temperature
    dend_axon = torch.matmul(dend_all, axon_all.T) / temperature
    dend_dend_exp = torch.exp(dend_dend)
    dend_axon_exp = torch.sum(
        torch.exp(dend_axon), dim=1, keepdim=True
    )
    loss_dend_dend = -torch.mean(dend_dend - torch.log(dend_dend_exp + dend_axon_exp))
    
    return loss_axon_axon + loss_dend_dend

# focus on user scribbled patches
def contrastive_loss_50to50_2(feature, user, label, temperature):
    other_feature, user_feature = feature[user==False, :], feature[user==True, :]
    other_label, user_label = label[user==False], label[user==True]
    
    feature = {
        'axon': normalize(other_feature[other_label==0, :], p=2 , dim=1),
        'dend': normalize(other_feature[other_label==1, :], p=2 , dim=1),
        'axon_user': normalize(user_feature[user_label==0, :], p=2 , dim=1),
        'dend_user': normalize(user_feature[user_label==1, :], p=2 , dim=1)
    }

    axon_all = torch.cat((feature['axon'], feature['axon_user']), dim=0)
    dend_all = torch.cat((feature['dend'], feature['dend_user']), dim=0)
    
    num_axon  = axon_all.shape[0]
    mask_axon = torch.eye(num_axon, dtype=torch.bool)
    axon_axon = torch.matmul(feature['axon'], axon_all.T)[~mask_axon].reshape(num_axon, -1) / temperature
    axon_dend = torch.matmul(feature['axon'], dend_all.T) / temperature
    axon_axon_exp = torch.exp(axon_axon)
    axon_dend_exp = torch.sum(
        torch.exp(axon_dend), dim=1, keepdim=True
    )
    loss_axon_axon = -torch.mean(axon_axon - torch.log(axon_axon_exp + axon_dend_exp))
    
    num_dend  = dend_all.shape[0]
    mask_dend = torch.eye(num_dend, dtype=torch.bool)
    dend_dend = torch.matmul(feature['dend'], dend_all.T)[~mask_dend].reshape(num_dend, -1) / temperature
    dend_axon = torch.matmul(feature['dend'], axon_all.T) / temperature
    dend_dend_exp = torch.exp(dend_dend)
    dend_axon_exp = torch.sum(
        torch.exp(dend_axon), dim=1, keepdim=True
    )
    loss_dend_dend = -torch.mean(dend_dend - torch.log(dend_dend_exp + dend_axon_exp))
    
    return loss_axon_axon + loss_dend_dend


# loss on user to user / other to other
def contrastive_loss_test2(feature, user, label, temperature):
    other_feature, user_feature = feature[user==False, :], feature[user==True, :]
    other_label, user_label = label[user==False], label[user==True]
    
    feature = {
        'axon': normalize(other_feature[other_label==0, :], p=2 , dim=1),
        'dend': normalize(other_feature[other_label==1, :], p=2 , dim=1),
        'axon_user': normalize(user_feature[user_label==0, :], p=2 , dim=1),
        'dend_user': normalize(user_feature[user_label==1, :], p=2 , dim=1)
    }

    axon_all = torch.cat((feature['axon'], feature['axon_user']), dim=0)
    dend_all = torch.cat((feature['dend'], feature['dend_user']), dim=0)
    
    # num_axon  = feature['axon'].shape[0]
    # mask_axon = torch.eye(num_axon, dtype=torch.bool)
    # axon_axon = torch.matmul(feature['axon'], feature['axon'].T)[~mask_axon].reshape(num_axon, -1) / temperature
    # axon_dend = torch.matmul(feature['axon'], feature['dend'].T) / temperature
    # axon_axon_exp = torch.exp(axon_axon)
    # axon_dend_exp = torch.sum(
    #     torch.exp(axon_dend), dim=1, keepdim=True
    # )
    # loss_axon = -torch.mean(axon_axon - torch.log(axon_axon_exp + axon_dend_exp))
    
    # num_dend  = feature['dend'].shape[0]
    # mask_dend = torch.eye(num_dend, dtype=torch.bool)
    # dend_dend = torch.matmul(feature['dend'], feature['dend'].T)[~mask_dend].reshape(num_dend, -1) / temperature
    # dend_axon = torch.matmul(feature['dend'], feature['axon'].T) / temperature
    # dend_dend_exp = torch.exp(dend_dend)
    # dend_axon_exp = torch.sum(
    #     torch.exp(dend_axon), dim=1, keepdim=True
    # )
    # loss_dend = -torch.mean(dend_dend - torch.log(dend_dend_exp + dend_axon_exp))

    num_axon  = feature['axon_user'].shape[0]
    mask_axon = torch.eye(num_axon, dtype=torch.bool)
    axon_axon = torch.matmul(feature['axon_user'], feature['axon_user'].T)[~mask_axon].reshape(num_axon, -1) / temperature
    axon_dend = torch.matmul(feature['axon_user'], dend_all.T) / temperature
    axon_axon_exp = torch.exp(axon_axon)
    axon_dend_exp = torch.sum(
        torch.exp(axon_dend), dim=1, keepdim=True
    )
    loss_axon_user = -torch.mean(axon_axon - torch.log(axon_axon_exp + axon_dend_exp))

    print(feature['dend_user'].shape)
    exit()

    num_dend  = feature['dend_user'].shape[0]
    mask_dend = torch.eye(num_dend, dtype=torch.bool)
    dend_dend = torch.matmul(feature['dend_user'], feature['dend_user'].T)[~mask_dend].reshape(num_dend, -1) / temperature
    dend_axon = torch.matmul(feature['dend_user'], axon_all.T) / temperature
    dend_dend_exp = torch.exp(dend_dend)
    dend_axon_exp = torch.sum(
        torch.exp(dend_axon), dim=1, keepdim=True
    )
    loss_dend_user = -torch.mean(dend_dend - torch.log(dend_dend_exp + dend_axon_exp))
    
    return loss_axon_user + loss_dend_user # + loss_axon + loss_dend