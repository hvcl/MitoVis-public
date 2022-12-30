import torch
from torch.nn.functional import normalize


def contrastive_loss(feature, label, temperature):
    feature = {
        'axon': normalize(feature[label==0, :], p=2 , dim=1),
        'dend': normalize(feature[label==1, :], p=2 , dim=1)
    }

    num_axon  = feature['axon'].shape[0]
    mask_axon = torch.eye(num_axon, dtype=torch.bool)
    axon_axon = torch.matmul(feature['axon'], feature['axon'].T)[~mask_axon].reshape(num_axon, -1) / temperature
    axon_dend = torch.matmul(feature['axon'], feature['dend'].T) / temperature
    axon_axon_exp = torch.exp(axon_axon)
    axon_dend_exp = torch.sum(
        torch.exp(axon_dend), dim=1, keepdim=True
    )
    loss_axon_axon = -torch.mean(axon_axon - torch.log(axon_axon_exp + axon_dend_exp))
    
    num_dend  = feature['dend'].shape[0]
    mask_dend = torch.eye(num_dend, dtype=torch.bool)
    dend_dend = torch.matmul(feature['dend'], feature['dend'].T)[~mask_dend].reshape(num_dend, -1) / temperature
    dend_axon = torch.matmul(feature['dend'], feature['axon'].T) / temperature
    dend_dend_exp = torch.exp(dend_dend)
    dend_axon_exp = torch.sum(
        torch.exp(dend_axon), dim=1, keepdim=True
    )
    loss_dend_dend = -torch.mean(dend_dend - torch.log(dend_dend_exp + dend_axon_exp))
    
    return loss_axon_axon + loss_dend_dend
    