#!/bin/bash

# This file was used for set package version with build info.
# package.json will in 2 places
# 1. Top zip file DataProtect_<version>_client.zip
# 2. ProtectClient-e\protectclient-Linux-x86_64\conf
# 
# Example Usage:  sh set_package_version.sh ./package.json eisoo_20210730 1.0.0.066 20210804130739 1.1.0

PACKAGE_FILE_NAME=$1  # Package file  e.g. package.json
MS_IMAGE_TAG=$2       # Build number. e.g.  1.0.0.066
VERSION=$3 # Package Major Version  e.g. 1.1.0
TIMESTAMP_STARTTIME=`date +%s`000 # Time Stamp when build start  e.g. 1629084463308

sys=`uname -s`
if [ "$sys" = "Linux" ]; then
    sed -i "s/<MS_IMAGE_TAG>/${MS_IMAGE_TAG}/g" ${PACKAGE_FILE_NAME}
    sed -i "s/<TIMESTAMP_STARTTIME>/${TIMESTAMP_STARTTIME}/g" ${PACKAGE_FILE_NAME}  
    sed -i "s/<VERSION>/${VERSION}/g" ${PACKAGE_FILE_NAME} 
else
    sed "s/<MS_IMAGE_TAG>/${MS_IMAGE_TAG}/g" ${PACKAGE_FILE_NAME} > ${PACKAGE_FILE_NAME}.bak
    mv -f ${PACKAGE_FILE_NAME}.bak ${PACKAGE_FILE_NAME}
    sed "s/<TIMESTAMP_STARTTIME>/${TIMESTAMP_STARTTIME}/g" ${PACKAGE_FILE_NAME} > ${PACKAGE_FILE_NAME}.bak
    mv -f ${PACKAGE_FILE_NAME}-bak ${PACKAGE_FILE_NAME}
    sed "s/<VERSION>/${VERSION}/g" ${PACKAGE_FILE_NAME} > ${PACKAGE_FILE_NAME}.bak
    mv -f ${PACKAGE_FILE_NAME}.bak ${PACKAGE_FILE_NAME}
fi
