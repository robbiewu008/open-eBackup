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
#ifndef __VMFS6_LVM_H__
#define __VMFS6_LVM_H__


#define VMFS6_LVM_MAX_EXTENTS 32

#define VMFS6_LVM_SEGMENT_SIZE (256 * 1024 * 1024)


namespace Vmfs6IO {
struct VmfsLvmInfoS {
    uuid_t uuid;
    uint32_t numExtents;
    uint64_t size;
    uint64_t blocks;
};
using VmfsLvmInfoT = struct VmfsLvmInfoS;

/* === LVM === */
struct VmfsLvmS {
    VmfsDeviceT dev;

    VmfsFlagsT flags;

    /* LVM information */
    VmfsLvmInfoT lvm_info;

    /* number of extents currently loaded in the lvm */
    int loadedExtents;

    /* extents */
    VmfsVolumeT *extents[VMFS6_LVM_MAX_EXTENTS];
};
using VmfsLvmT = struct VmfsLvmS;

class VmfsLvm {
public:
    static VmfsLvm *Instance();
    virtual ~VmfsLvm() = default;

    /* Create a volume structure */
    VmfsLvmT *Create(VmfsFlagsT flags);

    /* Add an extent to the LVM */
    int AddExtent(VmfsLvmT *lvm, VmfsVolumeT *vol);

    /* Open an LVM */
    int Open(VmfsLvmT *lvm);

private:
    VmfsVolumeT *GetExtentFromOffset(const VmfsLvmT *lvm, off_t pos);
    /* Get extent size */
    uint64_t ExtentSize(const VmfsVolumeT *extent);
    typedef ssize_t (*vmfs_vol_io_func)(const VmfsDeviceT *, off_t, u_char *, size_t);

    /* Read a raw block of data on logical volume */
    ssize_t VmfsLvmIO(const VmfsLvmT *lvm, off_t pos, u_char *buf, size_t len, vmfs_vol_io_func func);
    /* Read a raw block of data on logical volume */
    ssize_t VmfsLvmRead(const VmfsDeviceT *dev, off_t pos, u_char *buf, size_t len);
    /* Reserve the underlying volume given a LVM position */
    int Reserve(const VmfsDeviceT *dev, off_t pos);
    /* Release the underlying volume given a LVM position */
    int Release(const VmfsDeviceT *dev, off_t pos);
    /* Close an LVM */
    void Close(VmfsDeviceT *dev); // TODO - who calls this?

private:
    VmfsLvm() = default;
    static VmfsLvm *m_instance;
    static std::mutex m_mutex;
};
}

#endif
