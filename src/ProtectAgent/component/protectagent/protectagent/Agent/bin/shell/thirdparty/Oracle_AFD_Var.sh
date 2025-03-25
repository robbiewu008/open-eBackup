#!/bin/bash
#=================================define variable begin=================================
ASM_USER=grid
AFD_DISK_GROUP=oinstall
PRE_SNAPSHOT=DRdata+RDdata
UDEV_FILE=/etc/udev/rules.d/99-oracle-asmdevices.rules
AFD_PATH=/dev/asmdisk
# source lun information, eg lunid1-lunwwn1;lunid2-lunwwn2
SOURCE_LUN_LIST="4166-6e09796100558199014fd16f00001046;4167-6e09796100558199014fd1d400001047;4168-6e09796100558199014fd23500001048;4169-6e09796100558199014fd26900001049"

# AFD and Disk relationship
LABEL_LIST="/dev/udisks/data2&&UDEVD1;/dev/udisks/arc2&&UDEVD2;/dev/udisks/arc1&&UDEVD3;/dev/udisks/data1&&UDEVD4"

AppName=dbUDEV
InstanceName=dbUDEV1
UserName=
Password=
ACTION_START=0
ACTION_STOP=1
IsASM=1
# ASM disk group, split by +, eg DG1+DG2+DG3
ASMDiskGroups=DGUDEV
ASMUserName=
ASMPassword=
ASMInstanceName=+ASM

AFD_MODE=1 # 0:nativeAFD, 1:udevAFD
AFD_NATIVE_MODE=664
#=================================define variable end===================================
