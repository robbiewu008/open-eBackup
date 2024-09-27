#!/bin/sh

CUR_PATH=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)
cd "${CUR_PATH}/../../../"
pwd
ls
ROOT_PATH=`dirname $0`

CMC_Binary=$ROOT_PATH/CMC_Binary
CMC_KMC_Src=$CMC_Binary/src
CMC_KMC_Include=$CMC_Binary/include
Platform=$ROOT_PATH/platform

func_exec=$1

##Huawei Secure C
function get_huawei_secure_c()
{
   echo "start get_huawei_secure_c..."
   local huawei_secure_c_path=$Platform/securec
   local huawei_secure_c_name="Huawei Secure C V100R001C01SPC004B002"
   local huawei_secure_c_Pakcage="$huawei_secure_c_name.zip"
   
   mkdir -p $huawei_secure_c_path
   
   unzip $CMC_Binary/"$huawei_secure_c_Pakcage" -d $CMC_Binary
   cp -rf $CMC_Binary/"$huawei_secure_c_name"/include  $huawei_secure_c_path
   cp -rf $CMC_Binary/"$huawei_secure_c_name"/src  $huawei_secure_c_path
}


ALL_KMC_INCLUDE_FILES="                       \
kmc_itf.h                                     \
wsec_cbb.h                                    \
wsec_config.h                                 \
wsec_errorcode.h                              \
wsec_itf.h                                    \
wsec_type.h                                   \
"

ALL_KMC_SRC_CAC_FILES="                       \
cac_ipsi.c                                    \
cac_openssl.c                                 \
cac_pri.h                                     \
"

ALL_KMC_SRC_COMMON_FILES="                    \
wsec_array.c                                  \
wsec_array.h                                  \
wsec_datetime.c                               \
wsec_pri.h                                    \
wsec_securec.h                                \
wsec_share.h                                  \
wsec_sspwin.c                                 \
wsec_util.c                                   \
"

ALL_KMC_SRC_KMC_FILES="                       \
kmc_func.c                                    \
kmc_pri.h                                     \
"

ALL_KMC_SRC_SDP_FILES="                       \
sdp_func.c                                    \
sdp_itf.h                                     \
sdp_pri.h                                     \
"

##KMC
function get_kmc()
{
   echo "start get_kmc..."
   local huawei_kmc_c_path=$Platform/kmc
   local huawei_kmc_include_c_path=$huawei_kmc_c_path/include
   local huawei_kmc_lib_c_path=$huawei_kmc_c_path/lib
   local huawei_kmc_src_c_path=$huawei_kmc_c_path/src
   local huawei_kmc_src_cac_c_path=$huawei_kmc_src_c_path/cac
   local huawei_kmc_src_common_c_path=$huawei_kmc_src_c_path/common
   local huawei_kmc_src_kmc_c_path=$huawei_kmc_src_c_path/kmc
   local huawei_kmc_src_sdp_c_path=$huawei_kmc_src_c_path/sdp
   
   mkdir -p $huawei_kmc_include_c_path
   mkdir -p $huawei_kmc_lib_c_path  
   for file in ${ALL_KMC_INCLUDE_FILES}
   do
       cp $CMC_KMC_Include/${file} $huawei_kmc_include_c_path
   done
   
   mkdir -p $huawei_kmc_src_cac_c_path
   mkdir -p $huawei_kmc_src_common_c_path
   mkdir -p $huawei_kmc_src_kmc_c_path
   mkdir -p $huawei_kmc_src_sdp_c_path
   for file in ${ALL_KMC_SRC_CAC_FILES}
   do
       cp $CMC_KMC_Src/cac/${file} $huawei_kmc_src_cac_c_path
   done
   for file in ${ALL_KMC_SRC_COMMON_FILES}
   do
       cp $CMC_KMC_Src/common/${file} $huawei_kmc_src_common_c_path
   done
   for file in ${ALL_KMC_SRC_KMC_FILES}
   do
       cp $CMC_KMC_Src/kmc/${file} $huawei_kmc_src_kmc_c_path
   done
   for file in ${ALL_KMC_SRC_SDP_FILES}
   do
       cp $CMC_KMC_Src/sdp/${file} $huawei_kmc_src_sdp_c_path
   done
}

##other 
function other_handle()
{  
   rm -rf $CMC_Binary
}


##prepare
function prepare() 
{
   echo "start prepare... "
   if [ -d "$CMC_Binary" ]; then
       rm -rf $CMC_Binary
   fi 
   
   if [ -d "$Platform" ]; then
       rm -rf $Platform
   fi
}

##post_
function post() 
{
   echo "start post..."
   get_huawei_secure_c
   get_kmc
   other_handle
}

function main()
{
   echo "start main..."
   if [ "$func_exec" = "prepare" ]; then
       prepare  
   fi 
   if [ "$func_exec" = "post" ]; then
       post  
   fi    
}
#excute function
main