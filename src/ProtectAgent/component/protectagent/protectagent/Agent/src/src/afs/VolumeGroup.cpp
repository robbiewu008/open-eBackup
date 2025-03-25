/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 *
 * @file volumeGroup.cpp
 *
 * @brief AFS - LVM volume group class
 *
 */

#include <cstdlib>
#include <climits>
#include "afs/LogicalVolume.h"
#include "afs/VolumeGroup.h"

/**
 * @brief 构造函数
 *
 * @param &id    uuid
 * @param &name  卷组名
 * @param seq   序列号
 * @param size   extent的大小
 *
 */
volumeGroup::volumeGroup(string &id, string &name, int32_t seq, int32_t size)
{
    m_uuid = id;
    m_volname = name;
    m_seqno = seq;
    m_extent_size = size;
    m_max_lv = 0;
    m_max_pv = 0;
    m_span_part_flag = false;
    m_pvolumes.clear();
    m_lvolumes.clear();
    m_dev_name.clear();
    m_pv_part_id.clear();
}

/* * @brief 构造函数
 *
 */
volumeGroup::volumeGroup()
{
    m_seqno = 0;
    m_extent_size = 0;
    m_max_lv = 0;
    m_max_pv = 0;
    m_span_part_flag = false;
    m_pvolumes.clear();
    m_lvolumes.clear();
    m_dev_name.clear();
    m_pv_part_id.clear();
}

/**
 * @brief 析构函数
 */
volumeGroup::~volumeGroup()
{
    vector<physicalVolume *>::iterator i;
    vector<logicalVolume *>::iterator j;
    for (i = m_pvolumes.begin(); i != m_pvolumes.end(); ++i) {
        if (NULL != (*i)) {
            delete (*i);
            (*i) = NULL;
        }
    }

    for (j = m_lvolumes.begin(); j != m_lvolumes.end(); ++j) {
        if (NULL != (*j)) {
            delete (*j);
            (*j) = NULL;
        }
    }

    m_span_part_flag = false;

    m_pvolumes.clear();
    m_lvolumes.clear();
    m_dev_name.clear();
    m_pv_part_id.clear();
}

/**
 * @brief 根据卷组名找卷组
 *
 * @param &id uuid
 *
 * @return  返回找到的卷组
 *
 */
physicalVolume *volumeGroup::findPhysicalVolume(string &id)
{
    physicalVolume *pvol = NULL;
    vector<physicalVolume *>::iterator i;

    for (i = m_pvolumes.begin(); i != m_pvolumes.end(); ++i) {
        pvol = (*i);
        if (pvol->uuid.compare(id) == 0) {
            return pvol;
        }
    }

    return NULL;
}

/**
 * @brief 提取 字符和数字中的的字符和数字
 * @param src      源字符串
 * @param &dest    目的字符串
 * @param &num     长度
 * @return void
 */

void volumeGroup::splitStrAndNum(string src, string &dest, int32_t &num)
{
    string::size_type ilen = 0;
    int str_index = 0;
    int num_index = 0;
    char str_arr[512] = {0};
    char num_arr[512] = {0};

    while (ilen < src.length()) {
        if ('0' <= src.at(ilen) && '9' >= src.at(ilen)) {
            num_arr[num_index] = src.at(ilen);
            num_index++;
        } else {
            str_arr[str_index] = src.at(ilen);
            str_index++;
        }

        ilen++;
    }

    dest = str_arr;

    errno = 0;
    num = strtol(num_arr, NULL, 10);
    /* Check for various possible errors */
    if ((errno == ERANGE && (num == INT_MAX)) || (errno != 0 && num == 0)) {
        // 有可能是整个盘是一个分区
        return;
    }
}

/**
 * @brief 更新物理卷信息
 *
 * @param part_index 分区索引
 * @param m_pv_offset pv偏移
 *
 */
void volumeGroup::updatePVAddr(int32_t part_index, uint64_t m_pv_offset)
{
    vector<int32_t>::iterator it;
    int index = 0;

    m_span_part_flag = false;

    for (it = m_pv_part_id.begin(); it != m_pv_part_id.end(); ++it) {
        if ((*it) == part_index) {
            // 更新分区一览表信息
            m_pvolumes[index]->offset = m_pv_offset;
            break;
        }
        index++;
    }
}

/* * @brief 添加物理卷
 *
 * @param sdev 设备名称
 * @param id uuid
 * @param devsize 设备大小
 * @param start pe_start起始偏移
 * @param count pe_count数量
 * @param dsk_offset 镜像偏移
 * @param part_index 分区索引号
 * @param uuid2  第二种格式的PV UUID
 *
 * @return  返回找到的卷
 *
 */
physicalVolume *volumeGroup::addPhysicalVolume(string &sdev, string &id, uint64_t devsize, uint32_t start,
    uint32_t count, uint64_t disk_offset, int32_t disk_id, int32_t part_index, char *uuid2)
{
    string sdev_n;
    int32_t part_num = 0;
    if (uuid2 == NULL) {
        return NULL;
    }

    splitStrAndNum(sdev, sdev_n, part_num);
    AFS_TRACE_OUT_INFO("physical volume device name is %s, part_num = %d", sdev_n.c_str(), part_num);

    m_pv_part_id.push_back(part_num);
    m_span_part_flag = false;
    disk_offset = 0;

    physicalVolume *pvol = new physicalVolume(sdev, part_num, id, devsize, start, count, disk_id, disk_offset);
    if (!pvol) {
        return NULL;
    }

    if (AFS_SUCCESS != pvol->setPVUUID(uuid2)) {
        delete pvol;
        return NULL;
    }

    string span_part_str = m_span_part_flag == true ? "true" : "false";
    m_pvolumes.push_back(pvol);

    return pvol;
}

/* * @brief 根据 uuid 找 thin lv
 *
 * @param   uuid_type lv的uuid
 * UUID格式  ：  o3OAi1IUIz0Svvvum1kknjkHp3sVT4qq
 * @return  返回找到的lv
 *
 */
logicalVolume *volumeGroup::findThinLogicalVolume(string &uuid_type)
{
    logicalVolume *lvol = NULL;
    vector<logicalVolume *>::iterator lv_iter;

    for (lv_iter = m_lvolumes.begin(); lv_iter != m_lvolumes.end(); ++lv_iter) {
        lvol = (*lv_iter);
        if (lvol->m_type == SEG_THIN) {
            string tempStr = lvol->m_first_pv_uuid; // 统一格式
            if (tempStr.compare(uuid_type) == 0) {
                return lvol;
            }
        }
    }

    return NULL;
}

/* * @brief 根据卷组名找lv
 *
 * @param id uuid
 * @return  返回找到的lv
 *
 */
logicalVolume *volumeGroup::findLogicalVolume(string &id)
{
    logicalVolume *lvol = NULL;
    vector<logicalVolume *>::iterator i;

    for (i = m_lvolumes.begin(); i != m_lvolumes.end(); ++i) {
        lvol = (*i);
        if (lvol->m_uuid.compare(id) == 0) {
            return lvol;
        }
    }

    return NULL;
}

/* * @brief 添加逻辑卷
 *
 * @param id uuid
 * @param count pe_count数量
 * @param vname 逻辑卷名字
 * @return  返回找到的卷
 *
 */
logicalVolume *volumeGroup::addLogicalVolume(string &id, int32_t count, string &vname)
{
    logicalVolume *lvol = new logicalVolume(id, count, vname, *this);
    if (!lvol) {
        return NULL;
    }

    m_lvolumes.push_back(lvol);

    return lvol;
}

/**
 * @brief 获取bitmap
 *
 * @param bitmap 需要的设置的bitmap
 */
int32_t volumeGroup::getBitmap(vector<BitMap *> &bitmap_vect)
{
    vector<physicalVolume *>::iterator pv_iter;
    vector<logicalVolume *>::iterator lv_iter;
    physicalVolume *ppv = NULL;
    int32_t ret = AFS_SUCCESS;
    int32_t diskId = -1;
    BitMap *pbitmap = NULL;

    // 遍历设置pv
    for (pv_iter = m_pvolumes.begin(); pv_iter != m_pvolumes.end(); ++pv_iter) {
        ppv = (*pv_iter);
        if (NULL == ppv) {
            AFS_TRACE_OUT_ERROR("physicalVolume space is NULL.");
            return ret;
        }

        diskId = ppv->disk_id;
        pbitmap = bitmap_vect[diskId];
        ret = pbitmap->bitmapSetRange(ppv->offset, ppv->pe_start, 1);
        AFS_TRACE_OUT_DBG("PV [%s] bitmap set 1 [start offset = %llu, length = %u], diskId = %d", ppv->uuid.c_str(),
            ppv->offset, ppv->pe_start, diskId);
        if (AFS_SUCCESS != ret) {
            AFS_TRACE_OUT_ERROR("Can'nt set pv bitmap");
            return ret;
        }
    }

    // 遍历设置lv
    for (lv_iter = m_lvolumes.begin(); lv_iter != m_lvolumes.end(); ++lv_iter) {
        if (NULL == (*lv_iter)) {
            AFS_TRACE_OUT_ERROR("logicalVolume space is NULL.");
            return ret;
        }

        ret = (*lv_iter)->getBitMap(bitmap_vect);
        if (ret < 0) {
            AFS_TRACE_OUT_ERROR("Can'nt support format");
            return ret;
        }
    }

    return ret;
}
