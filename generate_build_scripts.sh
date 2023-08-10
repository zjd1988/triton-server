# step1. clone ai_algo_framework
SHELL_FOLDER=$(cd "$(dirname "$0")";pwd)
BUILD_DIR=${SHELL_FOLDER}/build
if [[ -d "${BUILD_DIR}" ]]; then
  rm -fr ${BUILD_DIR}
fi
GIE_DIR=${SHELL_FOLDER}/build/gie
if [[ ! -d "${GIE_DIR}" ]]; then
  mkdir -p ${GIE_DIR}
fi
GIE_BRANCH_TAG=feature/v2-dev-debug
git clone --recursive --single-branch --depth=1 -b ${GIE_BRANCH_TAG} ssh://git@geekssh.glodon.com:7999/data-intelligence-service/ai_algo_framework.git ${GIE_DIR}

# step2. generate build script
python  ${SHELL_FOLDER}/build.py -v --dryrun --backend=gie --backend=ensemble --build-type=Release --build-parallel=16 \
        --endpoint=grpc --endpoint=http --enable-logging --enable-stats \
        --enable-metrics --enable-gpu-metrics --enable-tracing --enable-gpu \
        --enable-triton-gie-backend --enable-onnxruntime 

# step3. run docker_build
DOCKER_BUILD_FILE=${SHELL_FOLDER}/build/docker_build
if [[ -e "${DOCKER_BUILD_FILE}" ]]; then
  bash ${DOCKER_BUILD_FILE}
else
  echo "file: ${DOCKER_BUILD_FILE} not exist"
  exit 1
fi 
