/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 *
 * @file
 * @brief
 * @version 1.0.0.0
 * @date 2023-11-22
 * @author
 */
/*
 * VMFS LVM layer
 */

#include <cstdlib>
#include "dataprocess/ioscheduler/libvmfs/Vmfs.h"


namespace Vmfs5IO {
VmfsLvm *VmfsLvm::m_instance = nullptr;
std::mutex VmfsLvm::m_mutex;

VmfsLvm *VmfsLvm::Instance()
{
    if (m_instance == nullptr) {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_instance == nullptr) {
            m_instance = new (std::nothrow) VmfsLvm();
        }
    }
    return m_instance;
}

/*
 * Until we uncover the details of the segment descriptors format,
 * it is useless to try to do something more efficient.
 */


/* Create a volume structure */
VmfsLvmT *VmfsLvm::Create(VmfsFlagsT flags)
{
    VmfsLvmT *lvm = nullptr;
    if (!(lvm = (VmfsLvmT *)calloc(1, sizeof(*lvm)))) {
        return NULL;
    }

    lvm->flags = flags;
    if (flags.readWrite) {
        COMMLOG(OS_LOG_ERROR, "vmfsio - R/W support is experimental. Use at your own risk");
    }

    return lvm;
}

/* Add an extent to the LVM */
int VmfsLvm::AddExtent(VmfsLvmT *lvm, VmfsVolumeT *vol)
{
    uint32_t i;

    if (!vol) {
        return (-1);
    }

    if (lvm->loadedExtents == 0) {
        uuid_copy(lvm->lvm_info.uuid, vol->volInfo.lvmUuid);
        lvm->lvm_info.size = vol->volInfo.lvmSize;
        lvm->lvm_info.blocks = vol->volInfo.blocks;
        lvm->lvm_info.numExtents = vol->volInfo.numExtents;
    } else if (uuid_compare(lvm->lvm_info.uuid, vol->volInfo.lvmUuid)) {
        COMMLOG(OS_LOG_ERROR, "vmfsio - The %s file/device is not part of the LVM\n", vol->device);
        return (-1);
    } else if ((lvm->lvm_info.size != vol->volInfo.lvmSize) || (lvm->lvm_info.blocks != vol->volInfo.blocks) ||
        (lvm->lvm_info.numExtents != vol->volInfo.numExtents)) {
        fprintf(stderr,
            "vmfsio - LVM information mismatch for the %s"
            " file/device\n",
            vol->device);
        return (-1);
    }

    for (i = 0; (i < lvm->loadedExtents) && (vol->volInfo.firstSegment > lvm->extents[i]->volInfo.firstSegment); i++);
    if (lvm->loadedExtents) {
        size_t len = (lvm->loadedExtents - i) * sizeof(VmfsVolumeT *);
        int ret = memmove_s(&lvm->extents[i + 1], len, &lvm->extents[i], len);
        if (ret != 0) {
            return (-1);
        }
    }
    lvm->extents[i] = vol;
    lvm->loadedExtents++;

    return (0);
}

/* Open an LVM */
int VmfsLvm::Open(VmfsLvmT *lvm)
{
    if (!lvm->flags.allowMissingExtents && (lvm->loadedExtents != lvm->lvm_info.numExtents)) {
        COMMLOG(OS_LOG_ERROR, "vmfsio - Missing extents");
        return (-1);
    }

    lvm->dev.read = std::bind(&VmfsLvm::VmfsLvmRead, Instance(), std::placeholders::_1, std::placeholders::_2,
        std::placeholders::_3, std::placeholders::_4);
    lvm->dev.reserve = std::bind(&VmfsLvm::Reserve, Instance(), std::placeholders::_1, std::placeholders::_2);
    lvm->dev.release = std::bind(&VmfsLvm::Release, Instance(), std::placeholders::_1, std::placeholders::_2);
    lvm->dev.close = std::bind(&VmfsLvm::Close, Instance(), std::placeholders::_1);
    lvm->dev.uuid = &lvm->lvm_info.uuid;
    return (0);
}

VmfsVolumeT *VmfsLvm::GetExtentFromOffset(const VmfsLvmT *lvm, off_t pos)
{
    int extent;
    off_t segment = pos / VMFS5_LVM_SEGMENT_SIZE;

    for (extent = 0; extent < lvm->loadedExtents; extent++) {
        if ((segment >= lvm->extents[extent]->volInfo.firstSegment) &&
            (segment <= lvm->extents[extent]->volInfo.lastSegment)) {
            return (lvm->extents[extent]);
        }
    }

    return (NULL);
}

/* Get extent size */
inline uint64_t VmfsLvm::ExtentSize(const VmfsVolumeT *extent)
{
    return ((uint64_t)extent->volInfo.numSegments * VMFS5_LVM_SEGMENT_SIZE);
}

/* Read a raw block of data on logical volume */
inline ssize_t VmfsLvm::VmfsLvmIO(const VmfsLvmT *lvm, off_t pos, u_char *buf, size_t len, vmfs_vol_io_func func)
{
    VmfsVolumeT *extent = GetExtentFromOffset(lvm, pos);

    if (!extent) {
        return (-1);
    }

    pos -= (uint64_t)extent->volInfo.firstSegment * VMFS5_LVM_SEGMENT_SIZE;
    if ((pos + len) > ExtentSize(extent)) {
        COMMLOG(OS_LOG_ERROR, "vmfsio - i/o spanned over several extents is unsupported");
        return (-1);
    }

    return (func(&extent->dev, pos, buf, len));
}

/* Read a raw block of data on logical volume */
ssize_t VmfsLvm::VmfsLvmRead(const VmfsDeviceT *dev, off_t pos, u_char *buf, size_t len)
{
    VmfsLvmT *lvm = (VmfsLvmT *)dev;
    return (VmfsLvmIO(lvm, pos, buf, len, VmfsDeviceRead));
}

/* Reserve the underlying volume given a LVM position */
int VmfsLvm::Reserve(const VmfsDeviceT *dev, off_t pos)
{
    VmfsLvmT *lvm = (VmfsLvmT *)dev;
    VmfsVolumeT *extent = GetExtentFromOffset(lvm, pos);

    if (!extent) {
        return (-1);
    }

    return (VmfsDeviceReserve(&extent->dev, 0));
}

/* Release the underlying volume given a LVM position */
int VmfsLvm::Release(const VmfsDeviceT *dev, off_t pos)
{
    VmfsLvmT *lvm = (VmfsLvmT *)dev;
    VmfsVolumeT *extent = GetExtentFromOffset(lvm, pos);

    if (!extent) {
        return (-1);
    }

    return (VmfsDeviceRelease(&extent->dev, 0));
}

/* Close an LVM */
void VmfsLvm::Close(VmfsDeviceT *dev)
{
    VmfsLvmT *lvm = (VmfsLvmT *)dev;
    if (!lvm) {
        return;
    }
    while (lvm->loadedExtents--) {
        VmfsDeviceClose(&lvm->extents[lvm->loadedExtents]->dev);
    }

    free(lvm);
}
}