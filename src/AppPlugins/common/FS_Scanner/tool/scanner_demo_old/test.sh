#!/bin/bash
SCRIPT_PATH=$(cd $(dirname $0); pwd)
echo "${SCRIPT_PATH}"
ROOT_PATH=${SCRIPT_PATH}/../../
BUILD_PATH=${SCRIPT_PATH}/../../build/
chmod +x ${BUILD_PATH}/build_scanner.sh
sh ${BUILD_PATH}/build_scanner.sh
if [ $? -ne 0 ];then
    echo "make scanner error"
    exit 1
fi

cp ${ROOT_PATH}/build-cmake/libScanner.so ${SCRIPT_PATH}/bin
cp ${ROOT_PATH}/data_move_engine/build-cmake/DME_Framework/libebkcommon.so ${SCRIPT_PATH}/bin
cp ${ROOT_PATH}/data_move_engine/build-cmake/DME_Framework/libFrameCommon.so ${SCRIPT_PATH}/bin
cp ${ROOT_PATH}/data_move_engine/build-cmake/DME_Framework/libGaussdb.so ${SCRIPT_PATH}/bin
cp ${ROOT_PATH}/data_move_engine/build-cmake/DME_Framework/libkafkaclient.so ${SCRIPT_PATH}/bin
cp ${ROOT_PATH}/data_move_engine/build-cmake/DME_Framework/libjob_pool.so ${SCRIPT_PATH}/bin

cp ${ROOT_PATH}/data_move_engine/build-cmake/Data_Transmission_Frame/libbackup_storage_plugin.so ${SCRIPT_PATH}/bin
cp ${ROOT_PATH}/data_move_engine/build-cmake/Data_Transmission_Frame/libscheduler.so ${SCRIPT_PATH}/bin

cp ${ROOT_PATH}/data_move_engine/DME_Framework/third_party_groupware/jsoncpp_rel/libs/libjsoncpp.so.23 ${SCRIPT_PATH}/bin
cp ${ROOT_PATH}/data_move_engine/DME_Framework/third_party_groupware/lnfs_rel/lib/libnfs.so.13 ${SCRIPT_PATH}/bin
cp ${ROOT_PATH}/data_move_engine/DME_Framework/third_party_groupware/lsmb2_rel/lib/libsmb2.so.3 ${SCRIPT_PATH}/bin
cp ${ROOT_PATH}/data_move_engine/DME_Framework/third_party_groupware/boost_rel/lib/libboost_chrono.so.1.74.0 ${SCRIPT_PATH}/bin
cp ${ROOT_PATH}/data_move_engine/DME_Framework/third_party_groupware/boost_rel/lib/libboost_atomic.so.1.74.0 ${SCRIPT_PATH}/bin
cp ${ROOT_PATH}/data_move_engine/DME_Framework/third_party_groupware/boost_rel/lib/libboost_thread.so.1.74.0 ${SCRIPT_PATH}/bin
cp ${ROOT_PATH}/data_move_engine/DME_Framework/third_party_groupware/boost_rel/lib/libboost_filesystem.so.1.74.0 ${SCRIPT_PATH}/bin
cp ${ROOT_PATH}/data_move_engine/DME_Framework/third_party_groupware/boost_rel/lib/libboost_system.so.1.74.0 ${SCRIPT_PATH}/bin
cp ${ROOT_PATH}/data_move_engine/DME_Framework/third_party_groupware/boost_rel/lib/libboost_regex.so.1.74.0 ${SCRIPT_PATH}/bin
cp ${ROOT_PATH}/data_move_engine/DME_Framework/third_party_groupware/boost_rel/lib/libboost_log.so.1.74.0 ${SCRIPT_PATH}/bin
cp ${ROOT_PATH}/data_move_engine/DME_Framework/third_party_groupware/boost_rel/lib/libboost_date_time.so.1.74.0 ${SCRIPT_PATH}/bin
cp ${ROOT_PATH}/data_move_engine/DME_Framework/third_party_groupware/esdk_rel/lib/libeSDKOBS.so ${SCRIPT_PATH}/bin
cp ${ROOT_PATH}/data_move_engine/DME_Framework/third_party_groupware/esdk_rel/lib/libeSDKLogAPI.so ${SCRIPT_PATH}/bin
cp ${ROOT_PATH}/data_move_engine/DME_Framework/third_party_groupware/esdk_rel/lib/liblog4cpp.so.5 ${SCRIPT_PATH}/bin
cp ${ROOT_PATH}/data_move_engine/DME_Framework/third_party_groupware/leveldb_rel/out-shared/libleveldb.so.1 ${SCRIPT_PATH}/bin
cp ${ROOT_PATH}/data_move_engine/DME_Framework/third_party_groupware/protobuf_rel/lib/libprotobuf.so.24 ${SCRIPT_PATH}/bin
cp ${ROOT_PATH}/data_move_engine/DME_Framework/third_party_groupware/sqlite_rel/unixODBC/lib/libodbc.so.2 ${SCRIPT_PATH}/bin
cp ${ROOT_PATH}/data_move_engine/DME_Framework/third_party_groupware/LibRdKafka_rel/libs/librdkafka.so.1 ${SCRIPT_PATH}/bin
cp ${ROOT_PATH}/data_move_engine/DME_Framework/third_party_groupware/LibRdKafka_rel/libs/librdkafka++.so.1 ${SCRIPT_PATH}/bin
cp ${ROOT_PATH}/data_move_engine/DME_Framework/third_party_groupware/soci_rel/lib64/libsoci_core.so.4.0 ${SCRIPT_PATH}/bin
cp ${ROOT_PATH}/data_move_engine/DME_Framework/third_party_groupware/soci_rel/lib64/libsoci_sqlite3.so.4.0 ${SCRIPT_PATH}/bin
cp ${ROOT_PATH}/data_move_engine/DME_Framework/third_party_groupware/libiconv_rel/lib/libiconv.so.2 ${SCRIPT_PATH}/bin
cp ${ROOT_PATH}/data_move_engine/DME_Framework/third_party_groupware/icu_rel/libs/lib/libicudata.so.67 ${SCRIPT_PATH}/bin
cp ${ROOT_PATH}/data_move_engine/DME_Framework/third_party_groupware/icu_rel/libs/lib/libicui18n.so.67 ${SCRIPT_PATH}/bin
cp ${ROOT_PATH}/data_move_engine/DME_Framework/third_party_groupware/icu_rel/libs/lib/libicuuc.so.67 ${SCRIPT_PATH}/bin

cp ${ROOT_PATH}/data_move_engine/DME_Framework/platform/KMCv3_infra_rel/lib/libkmcv3.so ${SCRIPT_PATH}/bin
cp ${ROOT_PATH}/data_move_engine/DME_Framework/platform/libssh2_rel/lib/libssh2.so.1 ${SCRIPT_PATH}/bin

mkdir -p ${SCRIPT_PATH}/bin
mkdir -p ${SCRIPT_PATH}/build-cmake
cd ${SCRIPT_PATH}/build-cmake
cmake .. && make -j16
if [ $? -ne 0 ];then
    echo "make scanner demo error"
    exit 1
fi

export LD_LIBRARY_PATH=${SCRIPT_PATH}/bin

if [ "X$1" == "Xrun" ];then
# start
${SCRIPT_PATH}/bin/scanner_demo_exe
fi

exit 0