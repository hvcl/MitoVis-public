import numpy as np
from nd2reader import ND2Reader
import sys
import os
import skimage.io

os.environ["KMP_DUPLICATE_LIB_OK"] = "TRUE"

def main(argv):
    nd2Dir=argv[1]
    result_path=argv[2]
    if not os.path.exists(result_path):
        os.makedirs(result_path)

    if os.path.exists(result_path+'/ch0.tif'):
        os.remove(result_path+'/ch0.tif')

    if os.path.exists(result_path+'/ch1.tif'):
        os.remove(result_path+'/ch1.tif')

    if os.path.exists(result_path+'/ch2.tif'):
        os.remove(result_path+'/ch2.tif')

    if os.path.exists(result_path+'/ch3.tif'):
        os.remove(result_path+'/ch3.tif')


    with ND2Reader(nd2Dir) as images:
        images.iter_axes = 'zc'
        channelNum=images.sizes['c']

        stacks = [[] for i in range(channelNum)]
        i = 0
        for fov in images:
            stacks[i%channelNum].append(fov)
            i+=1

        stacks=np.array(stacks)

        images = [[] for i in range(channelNum)]
        if stacks[0].ndim>2:
            for i in range(channelNum):
                images[i]=np.max(stacks[i],axis=0)
        else:
            images[i]=stacks[i]

        for i in range(channelNum):
            max_v = np.max(images[i])
            print(max_v)
            print('mean: ',np.mean(images[i]))
            print('std: ',np.std(images[i]))

            res = (images[i] - np.min(images[i])) / (max_v - np.min(images[i]))
            np.clip(res, 0, 1.0, out=res)
#            print('mean: ',np.mean(res[:]))
#            print('std: ',np.std(res[:]))
            res = res * 65535
            res = res.astype('uint16')
            skimage.io.imsave(result_path + '/ch' + str(i) + '.tif', res)


if __name__ == '__main__':
    main(sys.argv)
