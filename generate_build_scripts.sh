# step1. generate build script
SHELL_FOLDER=$(cd "$(dirname "$0")";pwd)
cd ${SHELL_FOLDER} 
python  ./build.py -v --dryrun --backend=gie --build-type=Release --build-parallel=16 \
        --endpoint=grpc --endpoint=http --enable-logging --enable-stats \
        --enable-metrics --enable-gpu-metrics --enable-tracing --enable-gpu \
        --enable-triton-gie-backend  --enable-cuda 

# step2. clone ai_algo_framework
# GIE_DIR=${SHELL_FOLDER}/build/gie
# git clone --recursive --single-branch --depth=1 -b feature/v2-dev-debug ssh://git@geekssh.glodon.com:7999/data-intelligence-service/ai_algo_framework.git ${GIE_DIR}

# step3. run docker_build
# DOCKER_BUILD_FILE=${SHELL_FOLDER}/build/docker_build
# bash ${DOCKER_BUILD_FILE}
