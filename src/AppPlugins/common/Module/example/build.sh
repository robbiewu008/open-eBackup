CURRENT_DIR=$(cd $(dirname $0); pwd)

mkdir -p ${CURRENT_DIR}/build-cmake
cd ${CURRENT_DIR}/build-cmake
cmake ..
make -j8 install