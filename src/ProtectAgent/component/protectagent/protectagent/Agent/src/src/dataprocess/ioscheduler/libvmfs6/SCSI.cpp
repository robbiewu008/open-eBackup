/*
* This file is a part of the open-eBackup project.
* This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
* If a copy of the MPL was not distributed with this file, You can obtain one at
* http://mozilla.org/MPL/2.0/.
*
* Copyright (c) [2024] Huawei Technologies Co.,Ltd.
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*/
#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <cstring>
#include <cerrno>
#include <sys/types.h>
#include <scsi/scsi.h>
#include <scsi/sg.h>
#include <sys/ioctl.h>
#include <securec.h>
#include "dataprocess/ioscheduler/libvmfs6/Vmfs.h"


namespace Vmfs6IO {
/* SCSI "reserve" command */
const int SCSI_CMD_RESERVE = 0x16;
const int SCSI_CMD_LEN_RESERVE = 6;

/* SCSI "release command */
const int SCSI_CMD_RELEASE = 0x17;
const int SCSI_CMD_LEN_RELEASE = 6;

SCSI *SCSI::m_instance = nullptr;
std::mutex SCSI::m_mutex;

SCSI *SCSI::Instance()
{
    if (m_instance == nullptr) {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_instance == nullptr) {
            m_instance = new (std::nothrow) SCSI();
        }
    }
    return m_instance;
}

/* Send a SCSI "reserve" command */
int SCSI::Reserve(int fd)
{
    sg_io_hdr_t io_hdr;
    u_char sense_buffer[32];
    u_char cmd[SCSI_CMD_LEN_RESERVE] = { SCSI_CMD_RESERVE, 0, 0, 0, 0, 0 };

    int ret = memset_s(&io_hdr, sizeof(struct sg_io_hdr), 0, sizeof(struct sg_io_hdr));
    if (ret != 0) {
        return (-1);
    }
    io_hdr.interface_id = 'S';
    io_hdr.cmd_len = sizeof(cmd);
    io_hdr.mx_sb_len = sizeof(sense_buffer);
    io_hdr.dxfer_direction = SG_DXFER_NONE;
    io_hdr.cmdp = cmd;
    io_hdr.sbp = sense_buffer;
    io_hdr.timeout = NUM_TIME_OUT;

    if (ioctl(fd, SG_IO, &io_hdr) < 0) {
        perror("ioctl");
        return (-1);
    }

    return (0);
}

/* Send a SCSI "release" command */
int SCSI::Release(int fd)
{
    sg_io_hdr_t io_hdr;
    u_char sense_buffer[32];
    u_char cmd[SCSI_CMD_LEN_RELEASE] = { SCSI_CMD_RELEASE, 0, 0, 0, 0, 0 };

    int ret = memset_s(&io_hdr, sizeof(struct sg_io_hdr), 0, sizeof(struct sg_io_hdr));
    if (ret != 0) {
        return (-1);
    }
    io_hdr.interface_id = 'S';
    io_hdr.cmd_len = sizeof(cmd);
    io_hdr.mx_sb_len = sizeof(sense_buffer);
    io_hdr.dxfer_direction = SG_DXFER_NONE;
    io_hdr.cmdp = cmd;
    io_hdr.sbp = sense_buffer;
    io_hdr.timeout = NUM_TIME_OUT;

    if (ioctl(fd, SG_IO, &io_hdr) < 0) {
        perror("ioctl");
        return (-1);
    }

    return (0);
}

int SCSI::GetLUN(int fd)
{
    struct ScsiIDLun idlun;
    if (ioctl(fd, SCSI_IOCTL_GET_IDLUN, &idlun)) {
        return (-1);
    }
    return ((idlun.fourInOne >> NUM_EIGHT) & 0xff);
}
}