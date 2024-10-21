#!bin/bash
########################################
#  This file is part of the open-eBackup project.
# Copyright (c) 2024 Huawei Technologies Co.,Ltd.
#
# open-eBackup is licensed under MPL v2.
# You can use this software according to the terms and conditions of the MPL v2.
# You may obtain a copy of MPL v2 at:
#
#          https://www.mozilla.org/en-US/MPL/2.0
#
# THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
# EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
# MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
# See the MPL v2 for more details.
########################################
CUR_PATH=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)
BASE_PATH=${CUR_PATH}/../..
INF_PATH=${BASE_PATH}/component/INF_CI
DEE_PATH=${BASE_PATH}/component/DEE_CI
DME_PATH=${BASE_PATH}/component/DME_CI
PM_PATH=${BASE_PATH}/component/PM_CI
MS_PKG_OUTPATH=${BASE_PATH}/pkg/mspkg

mkdir -p ${MS_PKG_OUTPATH}

#set bep
#echo "start set Bep_Time!"
#source bep_env.sh -s ${BASE_PATH}/CI/conf/bep_env.conf

function Copy_PKG()
{
	S_DIR=$1
	echo "Copy pkgs from ${S_DIR}/ to ${MS_PKG_OUTPATH}/, pkg:"
    ls -l "${S_DIR}/"
	if [ "$(ls -A $S_DIR)" = "" ]; then
		echo "${S_DIR} id empty!"
		echo "copy files from ${S_DIR} to ${MS_PKG_OUTPATH} failed!"
		exit 1
	fi

	cp -rf ${S_DIR}/*  ${MS_PKG_OUTPATH}/

}

function INF_PACK()
{
	cd ${INF_PATH}/OM/build/
	sh build.sh
	if [ $? -ne 0 ];then
		echo "om build failed!"
		exit 1
	fi
	cd ${INF_PATH}/infrastructure/script
	sh build.sh
	if [ $? -ne 0 ];then
		echo "base images build failed!"
		exit 1
	fi

	Copy_PKG "${INF_PATH}/OM/pkg"
	Copy_PKG "${INF_PATH}/package/compileLib"
}

function PM_PACK()
{
	PM_MS_LIST="PM_System_Base_Common_Service PM_DataMover_Access_Point PM_GUI PM_App_Common_Lib PM_Resource_Lock_Manager PM_Copies_Catalog PM_Resource_Manager PM_Data_Protection_Service PM_API_Gateway PM_Live_Mount_Manager"
	for pmservice in ${PM_MS_LIST}; do
      echo "start compile ${pmservice}!"
      cd ${PM_PATH}/component/${pmservice}/CI
      sh build.sh
      if [ $? -ne 0 ]; then
        echo "${pmservice} compile failed!"
        exit 1
      fi
    done

	for DIR_NAME in $(ls ${pmservice}); do
        if [ -d ${pmservice}/${DIR_NAME}/pkg ]; then
        	Copy_PKG "${pmservice}/${DIR_NAME}/pkg"
        fi
    done

}

function DME_PACK()
{
	cd ${DME_PATH}/build
	sh download_frame_third_platform.sh dorado master
	if [ $? -ne 0 ];then
		echo "download frame third platform error!"
		exit 1
	fi

	sh dme_make-cmake.sh
	if [ $? -ne 0 ];then
		echo "dme compile failed."
		exit 1
	fi
		
	sh dme_pack.sh
	if [ $? -ne 0 ];then
		echo "dme package failed."
		exit 1
	fi

	Copy_PKG "${DME_PATH}/pkg/mspkg"

}

function DEE_PACK()
{
	cd ${DEE_PATH}/build
	sh dee_pack.sh all
	if [ $? -ne 0 ];then
		echo "dee package failed."
		exit 1
	fi

	Copy_PKG "${DEE_PATH}/pkg"
}


function IMAGE_PACK()
{	
	cd ${BASE_PATH}/build
	sh docker_helm_build.sh
	if [ $? -ne 0 ];then
		echo "docker compile failed."
		exit 1
	fi
	
	sh package_final.sh
	if [ $? -ne 0 ];then
		echo "package failed."
		exit 1
	fi

	echo "The final package path is: ${BASE_PATH}/pkg/final"

}


function main()
{
	echo 
	echo "#########################################################"
	echo "  start compile base image  "
	echo "#########################################################"
	INF_PACK
	if [ $? -ne 0 ]; then
		echo "infrastructure compile failed!"
		exit 1
	fi

	echo 
	echo "#########################################################"
	echo "  start compile protectmanager   "
	echo "#########################################################"
	PM_PACK
	if [ $? -ne 0 ]; then
		echo "protectmanager compile failed!"
		exit 1
	fi

	echo 
	echo "#########################################################"
	echo "  start compile datamoverengine   "
	echo "#########################################################"
	DME_PACK
	if [ $? -ne 0 ]; then
		echo "datamoverengine compile failed!"
		exit 1
	fi

	echo 
	echo "#########################################################"
	echo "  start compile dataenableengine   "
	echo "#########################################################"
	DEE_PACK
	if [ $? -ne 0 ]; then
		echo "dataenableengine compile failed!"
		exit 1
	fi

	echo 
	echo "#########################################################"
	echo "  start compile final images   "
	echo "#########################################################"
	IMAGE_PACK
	if [ $? -ne 0 ]; then
		echo "final images compile failed!"
		exit 1
	fi

}

echo "#########################################################"
echo "   Begin package 100P  "
echo "#########################################################"

main

echo "#########################################################"
echo "   100P package Success  "
echo "#########################################################"
