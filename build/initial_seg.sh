clear -x
echo Running Docker...

# argument1: nd2file
if [ -z "$1" ]
then
    dir_data=$1
fi

# argument2: result_path
if [ -z "$2" ]
then
    iter=$2
fi

# argument3: image name
if [ -z "$3" ]
then
    in=$3
fi

# argument4: model for structure seg.
if [ -z "$4" ]
then
    model_s=$4
fi

# argument5: model for mito seg
if [ -z "$5" ]
then
    model_m=$5
fi


# Docker run
docker run \
       -u hjoh \
       --rm \
       --shm-size=16G \
       --gpus 0 \
       -v /mnt/d/MitoVis/build:/workspace \
       --name test1 \
       --hostname hjoh \
       -ti hjoh/torch:1.7.1 \
       python /workspace/processing/neuron_segmentation_new_model.py $1 $2 $3 $4 $5
