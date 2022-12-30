clear -x
echo Running Docker...

# argument1: data directory
if [ -z "$1" ]
then
    dir_data=$1
fi

# argument2: iteration
if [ -z "$2" ]
then
    iter=$2
fi



# Docker run
docker run \
       -u jychoi \
       --rm \
       --shm-size=16G \
       --gpus 0 \
       -v /mnt/d/MitoVis/build:/workspace \
       --name test1 \
       --hostname jychoi \
       -ti jychoi/torch:1.7.1 \
       python /workspace/VIS_processing/finetune.py $1 $2