/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 *
 * @file VMwareNativePrepareAfsBitmapTask.h
 * @brief  Contains function declarations VMwareNativePrepareAfsBitmapTask
 * @version 1.0.0
 * @date
 * @author
 */
#ifndef AGENT_VMWARENATIVE_BACKUP_AFSBITMAP_TASK
#define AGENT_VMWARENATIVE_BACKUP_AFSBITMAP_TASK

#include "apps/vmwarenative/VMwareNativeTask.h"

class VMwareNativePrepareAfsBitmapTask : public VMwareNativeTask {
public:
    VMwareNativePrepareAfsBitmapTask(const mp_string &taskID);
    virtual ~VMwareNativePrepareAfsBitmapTask();

    mp_int32 InitTaskStep(const Json::Value &param);

private:
    mp_void CreateTaskStep();
    mp_int32 PrepareIscsiMedia(const Json::Value &param);

    mp_string m_diskType;
};

#endif
