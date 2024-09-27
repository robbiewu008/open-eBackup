/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 *
 * @file TaskStepOracleNativeMediaData.h
 * @brief  Contains function declarations for TaskStepOracleNativeMediaData
 * @version 1.0.0
 * @date 2020-08-01
 * @author wangguitao 00510599
 */
#ifndef AGENT_BACKUP_STEP_PREPARE_MEDIA_DATA_H
#define AGENT_BACKUP_STEP_PREPARE_MEDIA_DATA_H

#include "apps/oraclenative/TaskStepOracleNativeMedia.h"
#include "common/Types.h"

static const mp_string STEPNAME_PREPAREMEDIA_DATA = "TaskStepOracleNativeMediaData";

/*
 * 生成备份介质
 * 文件系统：
 *  首次创建：使用下发所有的磁盘，使用每个磁盘全部空间创建一个分区，并创建LVM，可以用于以后扩容
 *  扩容场景：使用磁盘的空闲空间创建新的分区，并加入到LVM中，再扩展逻辑卷
 * ASM：
 *  首次创建：使用下发所有的磁盘，根据下发参数，确定哪个磁盘创建备份元数据空间，如果创建备份元数据空间，则创建1GB的文件系统（oracleprepareasmmedia.sh）
 *          剩下的空间创建1个分区；不创建备份元数据的磁盘使用全部空间创建分区，最后使用所有的分区创建一个ASM磁盘组
 *  扩容场景：使用每个磁盘的空闲空间创建新的分区，新建的分区增加到ASM磁盘组中
 *
 * 恢复场景
 * 1、createmode=0， 只需要mount，不创建新的备份介质
 */
class TaskStepOracleNativeMediaData : public TaskStepOracleNativeMedia {
public:
    TaskStepOracleNativeMediaData(
        const mp_string& id, const mp_string& taskId, const mp_string& name, mp_int32 ratio, mp_int32 order);
    ~TaskStepOracleNativeMediaData();
    mp_int32 Init(const Json::Value& param);

protected:
    mp_int32 InitialVolumes(mp_string& diskList);
    mp_int32 InitMountPath();
    
private:
    mp_int32 PrepareMediumInfo();
};

#endif
