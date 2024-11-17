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
#include <cstdlib>
#include <cstring>
#include <cerrno>
#include <sys/stat.h>
#include "dataprocess/ioscheduler/libvmfs6/Vmfs.h"


namespace Vmfs6IO {
VmfsDirent *VmfsDirent::m_instance = nullptr;
std::mutex VmfsDirent::m_mutex;

const int NUM_8191 = 8191;
const int NUMB_1024 = 1024;

VmfsDirent *VmfsDirent::Instance()
{
    if (m_instance == nullptr) {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_instance == nullptr) {
            m_instance = new (std::nothrow) VmfsDirent();
        }
    }
    return m_instance;
}

/* Read a directory entry */
int VmfsDirent::VmfsDirentRead(VmfsDirentT *entry, const u_char *buf)
{
    entry->type = ReadLE32(buf, VMFS6_DIRENT_OFFSET_TYPE);
    entry->blockId = ReadLE32(buf, VMFS6_DIRENT_OFFSET_BLK_ID);
    entry->recordId = ReadLE32(buf, VMFS6_DIRENT_OFFSET_REC_ID);
    int ret = memcpy_s(entry->name, VMFS6_DIRENT_OFFSET_NAME_SIZE,
        buf + VMFS6_DIRENT_OFFSET_NAME, VMFS6_DIRENT_OFFSET_NAME_SIZE);
    if (ret != 0) {
        return (-1);
    }
    entry->name[VMFS6_DIRENT_OFFSET_NAME_SIZE] = 0;
    return (0);
}

/* Write a directory entry */
int VmfsDirent::VmfsDirentWrite(const VmfsDirentT *entry, u_char *buf)
{
    WriteLE32(buf, VMFS6_DIRENT_OFFSET_TYPE, entry->type);
    WriteLE32(buf, VMFS6_DIRENT_OFFSET_BLK_ID, entry->blockId);
    WriteLE32(buf, VMFS6_DIRENT_OFFSET_REC_ID, entry->recordId);
    int ret = memcpy_s(buf + VMFS6_DIRENT_OFFSET_NAME,
        VMFS6_DIRENT_OFFSET_NAME_SIZE, entry->name, VMFS6_DIRENT_OFFSET_NAME_SIZE);
    if (ret != 0) {
        return (-1);
    }
    return (0);
}

/* Search for an entry into a directory ; affects position of the next
entry VmfsDirRead will return */
const VmfsDirentT *VmfsDirent::VmfsDirLookup(VmfsDirT *d, const char *name)
{
    const VmfsDirentT *rec;
    VmfsDirSeek(d, 0);
    while ((rec = VmfsDirRead(d))) {
        if (!strcmp(rec->name, name)) {
            return (rec);
        }
    }
    return (NULL);
}

/* Read a symlink */
char *VmfsDirent::VmfsDirentReadSymlink(const VmfsFsT *fs, const VmfsDirentT *entry)
{
    VmfsFileT *f;
    size_t str_len;
    char *str = NULL;

    if (!(f = VmfsFile::Instance()->OpenFromBlkid(fs, entry->blockId))) {
        return NULL;
    }

    str_len = VmfsFile::Instance()->VmfsFileGetSize(f);
    if (!(str = (char *)malloc(str_len + 1))) {
        goto done;
    }

    if ((str_len = VmfsFile::Instance()->Read(f, (u_char *)str, str_len, 0)) == -1) {
        free(str);
        goto done;
    }

    str[str_len] = 0;

done:
    VmfsFile::Instance()->Close(f);
    return str;
}

/* Resolve a path name to a block id */
uint32_t VmfsDirent::VmfsDirResolvePath(VmfsDirT *base_dir, const char *path, int follow_symlink)
{
    VmfsDirT *cur_dir;
    VmfsDirT *sub_dir;
    const VmfsDirentT *rec;
    char *nam;
    char *str;
    char *sl;
    char *symlink;
    int close_dir = 0;
    const VmfsFsT *fs = VmfsDirGetFs(base_dir);
    uint32_t ret = 0;

    cur_dir = base_dir;

    if (*path == '/') {
        if (!(cur_dir = VmfsDirOpenFromBlkid(fs, VMFS6_BLK_FD_BUILD(0, 0, 0)))) {
            return (0);
        }
        path++;
        close_dir = 1;
    }

    if (!(rec = VmfsDirLookup(cur_dir, "."))) {
        return (0);
    }

    ret = rec->blockId;
    nam = str = strdup(path);
    while (*str != 0) {
        sl = strchr(str, '/');
        if (sl != NULL) {
            *sl = 0;
        }

        if (*str == 0) {
            str = sl + 1;
            continue;
        }

        if (!(rec = VmfsDirLookup(cur_dir, str))) {
            ret = 0;
            break;
        }

        ret = rec->blockId;
        if ((sl == NULL) && !follow_symlink) {
            break;
        }

        /* follow the symlink if we have an entry of this type */
        if (rec->type == VMFS6_FTYPE_SYMLINK) {
            if (!(symlink = VmfsDirentReadSymlink(fs, rec))) {
                ret = 0;
                break;
            }

            ret = VmfsDirResolvePath(cur_dir, symlink, 1);
            free(symlink);

            if (!ret) {
                break;
            }
        }

        if (sl == NULL)
            break;

        if (!(sub_dir = VmfsDirOpenFromBlkid(fs, ret))) {
            ret = 0;
            break;
        }

        if (close_dir)
            VmfsDirClose(cur_dir);

        cur_dir = sub_dir;
        close_dir = 1;
        str = sl + 1;
    }
    free(nam);

    if (close_dir)
        VmfsDirClose(cur_dir);

    return (ret);
}

/* Cache content of a directory */
int VmfsDirent::VmfsDirCacheEntries(VmfsDirT *d)
{
    off_t dir_size;
    int cn_page;
    if (d->buf != NULL) {
        free(d->buf);
    }

    dir_size = VmfsFile::Instance()->VmfsFileGetSize(d->dir);
    cn_page = (dir_size + NUM_8191) / (NUM_1024 * NUM_TWO); // get ceil number of pages;

    if (!(d->buf = (u_char *)calloc(1, dir_size))) {
        return (-1);
    }
    if (!(d->ar_hb_exist = (u_char *)calloc(1, cn_page))) {
        return (-1);
    }

    if (VmfsFile::Instance()->Read(d->dir, d->buf, dir_size, 0) != dir_size) {
        free(d->buf);
        return (-1);
    }
    memcpy_s(d->ar_hb_exist, cn_page, d->buf + 0x10040, cn_page);

    return (0);
}

/* Open a directory file */
VmfsDirT *VmfsDirent::VmfsDirOpenFromFile(VmfsFileT *file)
{
    VmfsDirT *d;
    bool isDir;
    if (file == NULL) {
        return NULL;
    }

    isDir = (((file->inode->cmode) & S_IFMT) == S_IFDIR) ? 1 : 0;
    if (!(d = (VmfsDirT *)calloc(1, sizeof(*d))) || ((file->inode->type != VMFS6_FTYPE_DIR) && !isDir)) {
        VmfsFile::Instance()->Close(file);
        return NULL;
    }

    d->dir = file;
    VmfsDirCacheEntries(d);
    return d;
}

/* Open a directory based on a directory entry */
VmfsDirT *VmfsDirent::VmfsDirOpenFromBlkid(const VmfsFsT *fs, uint64_t blkId)
{
    return VmfsDirOpenFromFile(VmfsFile::Instance()->OpenFromBlkid(fs, blkId));
}

/* Return next entry in directory. Returned directory entry will be overwritten
by subsequent calls */
const VmfsDirentT *VmfsDirent::VmfsDirRead(VmfsDirT *d)
{
    u_char hb;
    u_char *buf;
    uint32_t off = 0;
    uint32_t cn_pages;
    uint32_t off_in_page;
    uint32_t cn_dirent_per_page = 4096 / VMFS6_DIRENT_SIZE; // 0x1000/0x120 = 0x0e

    if (d == NULL) {
        return (NULL);
    }

    do {
        if (d->pos < NUM_TWO) {
            off = 0x3b8 + (d->pos * VMFS6_DIRENT_SIZE);
            cn_pages = 0;
        } else {
            cn_pages = (d->pos - NUM_TWO) / cn_dirent_per_page;
            off_in_page = (d->pos - NUM_TWO) % cn_dirent_per_page;
            off = 0x11000 + cn_pages * 0x1000 + 0x40 + (off_in_page * VMFS6_DIRENT_SIZE);
        }
        if (d->ar_hb_exist) {
            hb = (((d->ar_hb_exist[(cn_pages + 1) / NUM_TWO] << (NUM_FOUR * ((cn_pages + 1) % NUM_TWO))) & 0xf0) >>
                  NUM_FOUR);
            if ((hb & 0x08) > 0) {
                return (NULL);
            }
            if ((hb & 0x01) == 0) {
                d->dirent.blockId = 0;
                goto l_next;
            }
        }

        if (d->buf) {
            if (off >= VmfsFile::Instance()->VmfsFileGetSize(d->dir)) {
                return (NULL);
            }
            buf = &d->buf[off];
        } else {
            u_char _buf[VMFS6_DIRENT_SIZE];
            if ((VmfsFile::Instance()->Read(d->dir, _buf, sizeof(_buf), off) != sizeof(_buf))) {
                return (NULL);
            }
            buf = _buf;
        }

        VmfsDirentRead(&d->dirent, buf);
    l_next:
        d->pos++;
    } while (d->dirent.blockId == 0);

    return &d->dirent;
}

/* Close a directory */
int VmfsDirent::VmfsDirClose(VmfsDirT *dir)
{
    if (dir == NULL) {
        return -1;
    }

    if (dir->buf) {
        free(dir->buf);
    }

    if (dir->ar_hb_exist) {
        free(dir->ar_hb_exist);
    }

    VmfsFile::Instance()->Close(dir->dir);
    free(dir);
    return 0;
}
}