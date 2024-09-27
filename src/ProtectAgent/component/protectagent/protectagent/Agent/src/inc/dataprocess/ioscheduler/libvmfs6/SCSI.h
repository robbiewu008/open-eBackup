/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 *
 * @file
 * @brief
 * @version 1.0.0.0
 * @date 2023-11-22
 * @author
 */
#ifndef __VMFS6_SCSI_H__
#define __VMFS6_SCSI_H__

#include <scsi/scsi.h>
#include <sys/ioctl.h>
#include <mutex>

namespace Vmfs6IO {
struct ScsiIDLun {
    int fourInOne;
    int hostUniqueId;
};

class SCSI {
public:
    static SCSI *Instance();
    virtual ~SCSI() = default;

    /* Send a SCSI "reserve" command */
    int Reserve(int fd);

    /* Send a SCSI "release" command */
    int Release(int fd);

    /* get lun */
    int GetLUN(int fd);

private:
    SCSI() = default;
    static SCSI *m_instance;
    static std::mutex m_mutex;
};
}


#endif
