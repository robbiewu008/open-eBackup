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
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "dataprocess/ioscheduler/libvmfs/Vmfs.h"


namespace Vmfs5IO {
VmfsVolume *VmfsVolume::m_instance = nullptr;
std::mutex VmfsVolume::m_mutex;

VmfsVolume *VmfsVolume::Instance()
{
    if (m_instance == nullptr) {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_instance == nullptr) {
            m_instance = new (std::nothrow) VmfsVolume();
        }
    }
    return m_instance;
}

/* Open a VMFS volume */
VmfsVolumeT *VmfsVolume::VolOpen(const char *filename, VmfsFlagsT flags)
{
    VmfsVolumeT *vol;
    struct stat st;
    int file_flags;

    if (!(vol = (VmfsVolumeT *)calloc(1, sizeof(*vol)))) {
        return NULL;
    }

    if (!(vol->device = strdup(filename))) {
        goto ERRFNAME;
    }

    file_flags = (flags.readWrite) ? O_RDWR : O_RDONLY;
    if ((vol->fd = open(vol->device, file_flags)) < 0) {
        perror("open");
        goto ERROPEN;
    }

    vol->flags = flags;
    fstat(vol->fd, &st);
    vol->isBlkdev = S_ISBLK(st.st_mode);

#if defined(O_DIRECT) || defined(DIRECTIO_ON)
    if (vol->isBlkdev) {
#ifdef O_DIRECT
        fcntl(vol->fd, F_SETFL, O_DIRECT);
#else
#ifdef DIRECTIO_ON
        directio(vol->fd, DIRECTIO_ON);
#endif
#endif
    }
#endif

    vol->vmfsBase = VMFS5_VOLINFO_BASE;

    /* Read volume information */
    do {
        DECL_ALIGNED_BUFFER(buf, NUM_512);
        uint16_t magic;
        /* Look for the MBR magic number */
        PReadFromFD(vol->fd, buf, buf_len, 0);
        magic = ReadLE16(buf, NUM_510);
        if (magic == 0xaa55) {
            /* Scan partition table */
            int off;
            for (off = NUM_446; off < NUM_510; off += NUM_16) {
                if (buf[off + NUM_FOUR] == 0xfb) {
                    vol->vmfsBase += (off_t)ReadLE32(buf, off + NUM_EIGHT) * NUM_512;
                    break;
                }
            }
        }
    } while (0);

    if (VolinfoRead(vol) == -1) {
        goto ERROPEN;
    }

    COMMLOG(OS_LOG_INFO, "vmfsio - version %d", vol->volInfo.version);
    /* We support only VMFS3 and VMFS5 */
    if ((vol->volInfo.version != NUM_THREE) && (vol->volInfo.version != NUM_FIVE)) {
        COMMLOG(OS_LOG_WARN, "vmfsio - unsupported version %u", vol->volInfo.version);
        goto ERROPEN;
    }

    if ((vol->volInfo.version == NUM_FIVE) && flags.readWrite) {
        COMMLOG(OS_LOG_WARN, "vmfsio - can't open VMFS for write");
        goto ERROPEN;
    }

    if (vol->isBlkdev && (SCSI::Instance()->GetLUN(vol->fd) != vol->volInfo.lun)) {
        COMMLOG(OS_LOG_WARN, "vmfsio - lun id mismatch on %s, lun id %ld", vol->device, vol->volInfo.lun);
    }

    CheckReservation(vol);

    COMMLOG(OS_LOG_DEBUG, "vmfsio - volume opened successfully");
    vol->dev.read = std::bind(&VmfsVolume::VolRead, Instance(), std::placeholders::_1, std::placeholders::_2,
        std::placeholders::_3, std::placeholders::_4);
    vol->dev.close = std::bind(&VmfsVolume::VolClose, Instance(), std::placeholders::_1);
    vol->dev.uuid = &vol->volInfo.lvmUuid;

    return vol;

ERROPEN:
    free(vol->device);
ERRFNAME:
    free(vol);
    return NULL;
}

/* Read a raw block of data on logical volume */
ssize_t VmfsVolume::VolRead(const VmfsDeviceT *dev, off_t pos, u_char *buf, size_t len)
{
    VmfsVolumeT *vol = (VmfsVolumeT *)dev;
    pos += vol->vmfsBase + 0x1000000;

    return (PReadFromFD(vol->fd, buf, len, pos));
}

/* Volume reservation */
int VmfsVolume::VolReserve(const VmfsDeviceT *dev, off_t pos)
{
    VmfsVolumeT *vol = (VmfsVolumeT *)dev;
    return SCSI::Instance()->Reserve(vol->fd);
}

/* Volume release */
int VmfsVolume::VolRelease(const VmfsDeviceT *dev, off_t pos)
{
    VmfsVolumeT *vol = (VmfsVolumeT *)dev;
    return SCSI::Instance()->Release(vol->fd);
}

/*
 * Check if physical volume support reservation.
 */
int VmfsVolume::CheckReservation(VmfsVolumeT *vol)
{
    int res[2];

    /* The device must be a block device */
    if (!vol->isBlkdev) {
        return (0);
    }

    /* Try SCSI commands */
    res[0] = SCSI::Instance()->Reserve(vol->fd);
    res[1] = SCSI::Instance()->Release(vol->fd);

    /* Error with the commands */
    if ((res[0] < 0) || (res[1] < 0)) {
        return (0);
    }

    vol->dev.reserve = std::bind(&VmfsVolume::VolReserve, Instance(), std::placeholders::_1, std::placeholders::_2);
    vol->dev.release = std::bind(&VmfsVolume::VolRelease, Instance(), std::placeholders::_1, std::placeholders::_2);
    return (1);
}

/* Read volume information */
int VmfsVolume::VolinfoRead(VmfsVolumeT *volume)
{
    DECL_ALIGNED_BUFFER(buf, NUMBER_1024);
    VmfsVolinfoT *vol = &volume->volInfo;

    if (PReadFromFD(volume->fd, buf, buf_len, volume->vmfsBase) != buf_len) {
        return (-1);
    }

    vol->magic = ReadLE32(buf, VMFS5_VOLINFO_OFFSET_MAGIC);

    if (vol->magic != VMFS5_VOLINFO_MAGIC) {
        COMMLOG(OS_LOG_ERROR, "vmfsio - VolInfo: invalid magic number 0x%8.8x\n", vol->magic);
        return (-1);
    }

    vol->version = ReadLE32(buf, VMFS5_VOLINFO_OFFSET_VER);
    vol->size = ReadLE32(buf, VMFS5_VOLINFO_OFFSET_SIZE);
    vol->lun = buf[VMFS5_VOLINFO_OFFSET_LUN];

    vol->name = strndup((char *)buf + VMFS5_VOLINFO_OFFSET_NAME, VMFS5_VOLINFO_OFFSET_NAME_SIZE);

    ReadUuid(buf, VMFS5_VOLINFO_OFFSET_UUID, &vol->uuid);

    vol->lvmSize = ReadLE64(buf, VMFS5_LVMINFO_OFFSET_SIZE);
    vol->blocks = ReadLE64(buf, VMFS5_LVMINFO_OFFSET_BLKS);
    vol->numSegments = ReadLE32(buf, VMFS5_LVMINFO_OFFSET_NUM_SEGMENTS);
    vol->firstSegment = ReadLE32(buf, VMFS5_LVMINFO_OFFSET_FIRST_SEGMENT);
    vol->lastSegment = ReadLE32(buf, VMFS5_LVMINFO_OFFSET_LAST_SEGMENT);
    vol->numExtents = ReadLE32(buf, VMFS5_LVMINFO_OFFSET_NUM_EXTENTS);

    ReadUuid(buf, VMFS5_LVMINFO_OFFSET_UUID, &vol->lvmUuid);

    return (0);
}

/* Close a VMFS volume */
void VmfsVolume::VolClose(VmfsDeviceT *dev)
{
    VmfsVolumeT *vol = (VmfsVolumeT *)dev;
    if (!vol) {
        return;
    }
    close(vol->fd);
    free(vol->device);
    free(vol->volInfo.name);
    free(vol);
}
}