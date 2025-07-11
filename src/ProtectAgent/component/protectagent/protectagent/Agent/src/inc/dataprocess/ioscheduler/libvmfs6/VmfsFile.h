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
#ifndef __VMFS6_FILE_H__
#define __VMFS6_FILE_H__

#include <sys/stat.h>

namespace Vmfs6IO {
#define VMFS6_FTYPE_DIR 0x02
#define VMFS6_FTYPE_FILE 0x03
#define VMFS6_FTYPE_SYMLINK 0x04
#define VMFS6_FTYPE_META 0x05
#define VMFS6_FTYPE_RDM 0x06

#define VMFS6_FFLAG_RW 0x01
#define VMFS6_FFLAG_FD 0x02

struct VmfsFileS {
    union {
        VmfsInodeT *inode;
        int fd;
    };
    u_int flags;
};
using VmfsFileT = struct VmfsFileS;

class VmfsFile {
public:
    static VmfsFile *Instance();
    virtual ~VmfsFile() = default;

    inline const VmfsFsT *VmfsFileGetFs(VmfsFileT *f)
    {
        if (f && !(f->flags & VMFS6_FFLAG_FD) && f->inode)
            return (f->inode->fs);

        return NULL;
    }

    inline mode_t VmfsFileType2Mode(uint32_t type)
    {
        switch (type) {
            case VMFS6_FTYPE_DIR:
                return S_IFDIR;
            case VMFS6_FTYPE_SYMLINK:
                return S_IFLNK;
            default:
                return S_IFREG;
        }
    }

    /* Get file size */
    inline uint64_t VmfsFileGetSize(const VmfsFileT *f)
    {
        if (f && !(f->flags & VMFS6_FFLAG_FD) && f->inode)
            return (f->inode->size);

        return 0;
    }

    /* Open a file based on an inode buffer */
    VmfsFileT *OpenFromInode(const VmfsInodeT *inode);

    /* Open a file based on a block id */
    VmfsFileT *OpenFromBlkid(const VmfsFsT *fs, uint64_t blkId);

    /* Open a file */
    VmfsFileT *OpenAt(VmfsDirT *dir, const char *path);

    /* Close a file */
    int Close(VmfsFileT *f);

    /* Read data from a file at the specified position */
    ssize_t Read(VmfsFileT *f, u_char *buf, size_t len, off_t pos);

private:
    VmfsFile() = default;
    static VmfsFile *m_instance;
    static std::mutex m_mutex;
};
}

#endif
