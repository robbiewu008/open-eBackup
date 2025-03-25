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
#include "afs/PartitionHandler.h"
#include "afs/RawReader.h"
#include "afs/LogMsg.h"
#include "afs/AfsLVM.h"
#include "afs/MBRHandler.h"
#include "afs/GPTHandler.h"
#include "afs/ThinlvSegment.h"

/**
 * @brief 构造函数
 *
 */
partitionHandler::partitionHandler()
{
    setObjType(OBJ_TYPE_PARTITION);
    m_type = (int32_t)PARTITION_UNKNOWN;
    setMagic("unknown");

    m_reader = NULL;
    m_partnum = 0;
    m_partBase = 0;
    m_disknum = 0;
    m_realhandler = NULL;
    m_real_part_reader = NULL;

    m_map_chunk_size.clear();
}

/**
 * @brief 析构函数
 *
 */
partitionHandler::~partitionHandler()
{
    m_reader = NULL;
    m_real_part_reader = NULL;
    m_map_chunk_size.clear();

    for (uint32_t i = 0; i < m_vect_imgreader.size(); i++) {
        if (m_vect_imgreader.at(i) != NULL) {
            delete m_vect_imgreader.at(i);
        }
    }
    m_vect_imgreader.clear();

    for (uint32_t i = 0; i < m_vector_realHandler.size(); i++) {
        if (m_vector_realHandler.at(i) != NULL) {
            delete m_vector_realHandler.at(i);
        }
    }
    m_vector_realHandler.clear();
}

/**
 * @brief 获取分区类型
 *
 * @return -1 失败 否则返回分区类型（MBR、GPT和无分区表）
 *
 */
int32_t partitionHandler::getPartType()
{
    int64_t read_size = 0;

    // 处理分区表
    unsigned char buf[SECTOR_SIZE] = {0};

    if (NULL == m_reader) {
        AFS_TRACE_OUT_ERROR("Cann't Read Image File");
        return AFS_ERR_PARAMETER;
    }

    read_size = m_reader->read(buf, 0, static_cast<int64_t>(SECTOR_SIZE), 1);
    if (static_cast<int64_t>(SECTOR_SIZE) != read_size) {
        AFS_TRACE_OUT_ERROR("Cann't Read Image File");
        return AFS_ERR_IMAGE_READ;
    }

    MBRHandler mbrhandler;
    // 已将无分区当做一个MBR特殊分区处理
    if (mbrhandler.mbr_no_part_magic(buf)) {
        m_type = (int32_t)PARTITION_NO;
        AFS_TRACE_OUT_DBG("PARTITION_NO.");
    } else if (0xee == mbrhandler.mbr_get_partition(buf, 0)->sys_ind) {
        AFS_TRACE_OUT_DBG("PARTITION_GPT.");
        m_type = (int32_t)PARTITION_GPT;
    } else if (mbrhandler.mbr_is_valid_magic(buf)) {
        AFS_TRACE_OUT_DBG("PARTITION_MBR.");
        m_type = (int32_t)PARTITION_MBR;
    } else {
        AFS_TRACE_OUT_ERROR("Cannot to analyze image partition.");
        m_type = (int32_t)PARTITION_UNKNOWN;
        return AFS_ERR_PARTITION;
    }

    return m_type;
}

/**
 * @brief 获取分区信息
 * @param index   分区号
 * @return 返回分区空间
 *
 */
void *partitionHandler::getPartitionPointer(int32_t index)
{
    if (index > m_partnum) {
        AFS_TRACE_OUT_ERROR("Partition count is little than index. part_num=%d, index=%d", m_partnum, index);
        return NULL;
    }

    if (m_parts_vect.size() < (size_t)(index + 1)) {
        AFS_TRACE_OUT_ERROR("Partition vector size [%u] is little than index. index=%d", m_parts_vect.size(), index);
        return NULL;
    }

    return &m_parts_vect[index];
}

/**
 * @brief 获取特定真实partitionHandler之前的分区总数，得到分区编号偏移
 *
 * @param real_handler  真实partitionHandler
 *
 * @return -1失败, >0分区总数
 *
 */
int32_t partitionHandler::getPartsNumBeforeHandler(partitionHandler *real_handler)
{
    int32_t ret = 0;
    partitionHandler *tempHandler;

    for (uint32_t i = 0; i < m_vector_realHandler.size(); i++) {
        tempHandler = m_vector_realHandler[i];
        if (tempHandler != real_handler) {
            ret = tempHandler->m_partnum;
        } else {
            break;
        }
    }

    AFS_TRACE_OUT_DBG("parts num before the real partition handler is %d", ret);
    return ret;
}

/**
 * @brief 获取分区信息
 *
 * @param index  分区号
 * @param *ppart 分区信息
 *
 * @return -1失败，0成功
 *
 */
int32_t partitionHandler::getPartition(int32_t index, struct partition *ppart)
{
    void *p = getPartitionPointer(index);
    if (NULL == p) {
        return AFS_ERR_PARTITION;
    }
    CHECK_MEMCPY_S_OK(ppart, sizeof(struct partition), p, sizeof(struct partition));

    return AFS_SUCCESS;
}

/**
 * @brief 给指定的分区创建reader
 *
 * @param *ppart 分区信息
 *
 * @return <0 失败，=0 成功
 *
 */
int32_t partitionHandler::createPartitionReader(struct partition *ppart)
{
    imgReader *img = NULL;
    // 根据分区属性，创建分区自己的reader
    img = m_createPartReader.createPartReaderOBJ(m_reader, ppart, m_vgManager.findPartIdMapLv(ppart->part_id));
    if (ppart->is_lvm && NULL == img) {
        // lv未找到
        AFS_TRACE_OUT_ERROR("Failed to create image reader by partition:ppart->is_lvm && NULL == img");
        return AFS_ERR_INNER;
    } else if (NULL == img) {
        AFS_TRACE_OUT_ERROR("Failed to create image reader by partition. img = NULL");
        return AFS_ERR_API;
    }

    // 添加镜像对象
    m_real_part_reader = img;

    return AFS_SUCCESS;
}

/**
 * @brief 设置分区号
 * @param partnum 分区个数
 * @return void
 */
int32_t partitionHandler::getDiskNum()
{
    return m_disknum;
}

/**
 * @brief 设置磁盘号
 * @param diskNum 磁盘个数
 * @return void
 */
void partitionHandler::setDiskNumValue(int32_t diskNum)
{
    m_disknum = diskNum;
}

/**
 * @brief 设置分区号
 * @param partnum 分区个数
 * @return void
 */
void partitionHandler::setPartnumValue(int32_t partnum)
{
    m_partnum = partnum;
}

/**
 * @brief 设置分区号
 *
 * @param partnum 分区个数
 *
 * @return AFS_SUCCESS
 *
 */
int32_t partitionHandler::setPartnum(int32_t partnum)
{
    struct partition part_info;
    CHECK_MEMSET_S_OK(&part_info, sizeof(struct partition), 0, sizeof(struct partition));
    m_parts_vect.push_back(part_info);
    m_partnum = partnum;
    return AFS_SUCCESS;
}

/**
 * @brief 获得分区总数
 * @return 返回分区个数
 *
 */
int32_t partitionHandler::getPartnum()
{
    return m_partnum;
}

/**
 * @brief 设置分区信息
 *
 * @param index   分区号
 * @param *ppart  分区信息
 *
 * @return   0：成功
 * 负数：错误ID
 *
 */
int32_t partitionHandler::setPartition(int32_t index, struct partition *ppart)
{
    void *part = getPartitionPointer(index);
    if (part != NULL) {
        CHECK_MEMCPY_S_OK(part, sizeof(struct partition), ppart, sizeof(struct partition));
        ((struct partition *)part)->disk_id = m_disknum;
        AFS_TRACE_OUT_DBG("partition belong disk is %d", m_disknum);
        return AFS_SUCCESS;
    }
    return AFS_ERR_PARTITION;
}

/**
 * @brief 创建分区处理对象
 * @returnpartitionHandler 返回创建的对象
 *
 */
partitionHandler *partitionHandler::createPartHandler()
{
    switch (m_type) {
        case PARTITION_NO:
            // PARTITION_NO,当做MBR的一个大分区
            AFS_TRACE_OUT_INFO("The partition no partition");
            m_realhandler = static_cast<partitionHandler *>(new MBRHandler((int32_t)PARTITION_NO));
            break;
        case PARTITION_MBR:
            AFS_TRACE_OUT_INFO("The partition is MBR partition");
            m_realhandler = static_cast<partitionHandler *>(new MBRHandler());
            break;
        case PARTITION_GPT:
            AFS_TRACE_OUT_INFO("The partition is GPT partition");
            m_realhandler = static_cast<partitionHandler *>(new GPTHandler());
            break;
        default:
            AFS_TRACE_OUT_ERROR("Invalid partition. Type=%d", m_type);
            return NULL;
    }

    if (NULL == m_realhandler) {
        AFS_TRACE_OUT_ERROR("The RealHandler is NULL");
        return NULL;
    }

    copyFatherHandlerInfo();

    m_vector_realHandler.push_back(m_realhandler); // 保存该磁盘的真实分区类型

    return m_realhandler;
}

/**
 * @brief   拷贝父类的信息到真实的分区 hander
 * @return  无
 *
 */
void partitionHandler::copyFatherHandlerInfo()
{
    list<volumeGroup *> &childVgList = m_realhandler->m_vgManager.getGroupListAddr();
    list<volumeGroup *> &fatherVgList = m_vgManager.getGroupListAddr();
    list<struct pv_no_vg_metadata> &childPvList = m_realhandler->getPvNoMetadataList();
    list<struct pv_no_vg_metadata> &fatherPvList = m_pv_no_vg_metadata;

    uint32_t childVgSize = childVgList.size();
    uint32_t childPvSize = childPvList.size();

    childVgList.swap(fatherVgList);
    AFS_TRACE_OUT_DBG("childVgSize before swap is %u, Now is %u", childVgSize, childVgList.size());

    m_realhandler->setImgReader(getImgReader());
    m_realhandler->setDiskNumValue(m_disknum);
    m_realhandler->setPartnumValue(m_partnum);
    m_realhandler->setPartBaseNumValue(m_partnum);
    AFS_TRACE_OUT_DBG("father disk num is %d, part num is %d", m_disknum, m_partnum);
    AFS_TRACE_OUT_DBG("child  disk num is %d, part num is %d", m_realhandler->m_disknum, m_realhandler->m_partnum);

    m_parts_vect.swap(m_realhandler->m_parts_vect); // /问题在此处
    AFS_TRACE_OUT_DBG("m_realhandler->m_parts_vect[%p] after swap is %u", &m_realhandler->m_parts_vect,
        m_realhandler->m_parts_vect.size());

    childPvList.swap(fatherPvList);
    AFS_TRACE_OUT_DBG("childPvSize before swap is %u, Now is %u", childPvSize, childPvList.size());
}

/**
 * @brief 更新thin类型LV的(partition[x].lvm_info.lv_offset)
 *
 * @return 负数： 错误
 *
 *    */
int32_t partitionHandler::updateThinLVPartInfo()
{
    vector<struct partition>::iterator part_iter;
    struct partition *pTempPart = NULL;
    logicalVolume *pThinLV;
    thinlvSegment *pThinlvSeg;
    list<volumeGroup *>::iterator vg_iter;

    for (part_iter = m_parts_vect.begin(); part_iter != m_parts_vect.end(); ++part_iter) {
        pTempPart = &(*part_iter);
        if (pTempPart->is_lvm && pTempPart->lvm_info.lv_update) { // 找到待更新的LV
            string lv_uuid = pTempPart->lvm_info.lv_pvUUID;
            // 寻找 thin LV
            for (vg_iter = m_vgManager.getGroupListAddr().begin(); vg_iter != m_vgManager.getGroupListAddr().end();
                ++vg_iter) {
                pThinLV = (*vg_iter)->findThinLogicalVolume(lv_uuid);
                if (pThinLV != NULL) {
                    pThinlvSeg = dynamic_cast<thinlvSegment *>(pThinLV->m_segments.back());
                    pTempPart->lvm_info.lv_offset = pThinlvSeg->getPartitionFirstAddr();
                    pTempPart->lvm_info.lv_update = 0;
                    pTempPart->disk_id = pThinlvSeg->m_thin_lv_disk_id;

                    AFS_TRACE_OUT_DBG("thin lv [name : %s, uuid : %s, disk_id = %d] update success",
                        pTempPart->lvm_info.lv_name, pTempPart->lvm_info.lv_pvUUID, pTempPart->disk_id);
                    break;
                }
            }
        }
    }

    return 0;
}

/**
 * @brief 根据no metadata pv 更新m_pvolumes中的PV
 *
 * @return 负数： 错误
 *
 *    */
int32_t partitionHandler::updateNoMetadataPV()
{
    int32_t ret = -1;
    list<struct pv_no_vg_metadata>::iterator pv_iter;
    vector<struct physicalVolume *>::iterator phy_iter;
    list<volumeGroup *>::iterator vg_iter;

    for (pv_iter = m_pv_no_vg_metadata.begin(); pv_iter != m_pv_no_vg_metadata.end(); pv_iter++) {
        string pv_uuid_string = pv_iter->pv_uuid;
        ret = -1;

        // 更新VG中m_pvolumes的PV信息
        for (vg_iter = m_vgManager.getGroupListAddr().begin(); vg_iter != m_vgManager.getGroupListAddr().end();
            ++vg_iter) {
            phy_iter = (*vg_iter)->m_pvolumes.begin();
            for (; phy_iter != (*vg_iter)->m_pvolumes.end(); phy_iter++) {
                string real_pv_uuid_string = (*phy_iter)->pv_uuid2;
                AFS_TRACE_OUT_DBG("the uuid of pv that no metadata is %s", pv_uuid_string.c_str());
                AFS_TRACE_OUT_DBG("now found pv uuid is %s", real_pv_uuid_string.c_str());
                if (real_pv_uuid_string.compare(pv_uuid_string) == 0) {
                    (*phy_iter)->disk_id = pv_iter->pv_disk_id;
                    (*phy_iter)->offset = pv_iter->pv_offset;
                    AFS_TRACE_OUT_DBG("PV that need to be updated is %s, new pv_offset is %llu, disk id is %d",
                        (*phy_iter)->uuid.c_str(), (*phy_iter)->offset, (*phy_iter)->disk_id);
                    ret = 0;
                    break;
                }
            }
            if (ret == 0) {
                break;
            }
        }

        if (ret == -1) {
            AFS_TRACE_OUT_ERROR("there is a error when update, result is %d", ret);
            return ret;
        }
    }

    return 0;
}

/**
 * @brief 根据no metadata pv 更新所有的LV
 *
 * @return 负数： 错误
 *
 *    */
int32_t partitionHandler::updateLVWithNoMetadataPV()
{
    int32_t ret = -1;
    vector<struct partition>::iterator part_iter;
    list<struct pv_no_vg_metadata>::iterator pv_iter;
    struct partition *pTempPart = NULL;

    for (part_iter = m_parts_vect.begin(); part_iter != m_parts_vect.end(); ++part_iter) {
        pTempPart = &(*part_iter);
        if (pTempPart->is_lvm && pTempPart->lvm_info.lv_update) { // 找到待更新的LV
            ret = -1;
            string lv_uuid_string = pTempPart->lvm_info.lv_pvUUID;
            pTempPart->lvm_info.lv_pvUUID[UUID_LEN] = '\0';

            for (pv_iter = m_pv_no_vg_metadata.begin(); pv_iter != m_pv_no_vg_metadata.end(); pv_iter++) {
                string pv_uuid_string = pv_iter->pv_uuid;
                if (pv_uuid_string.compare(lv_uuid_string) == 0) { // 找到需要更新的一个pv
                    AFS_TRACE_OUT_DBG("find a lv [part id: %d, name: %s] that need update", part_iter->part_id,
                        part_iter->lvm_info.lv_name);
                    pTempPart->lvm_info.lv_offset += pv_iter->pv_offset; // 更新LV相对磁盘的偏移
                    pTempPart->disk_id = pv_iter->pv_disk_id;
                    pTempPart->lvm_info.lv_update = false;
                    ret = 0;
                    break;
                }
            }

            if (ret == -1) {
                AFS_TRACE_OUT_ERROR("there is a error when update, result is %d", ret);
                return ret;
            }
        }
    }
    return 0;
}

/**
 * @brief 验证所有磁盘中包含的 thin-pool LV的Super block是否正确
 *
 *
 * @return 负数： 错误
 *
 *    */
int32_t partitionHandler::verifyThinPoolSuperBlock()
{
    int32_t ret = 0;
    volumeGroup *grp = NULL;
    logicalVolume *lvol = NULL;
    list<volumeGroup *>::iterator vg_iter;
    vector<logicalVolume *>::iterator lv_iter;

    AFS_TRACE_OUT_DBG("Begin to verify thin pool super block");
    for (vg_iter = m_vgManager.getGroupListAddr().begin(); vg_iter != m_vgManager.getGroupListAddr().end(); ++vg_iter) {
        if (NULL == (*vg_iter)) {
            AFS_TRACE_OUT_ERROR("The vg_iter is NULL");
            return -1;
        }

        grp = (*vg_iter);
        for (lv_iter = grp->m_lvolumes.begin(); lv_iter != grp->m_lvolumes.end(); ++lv_iter) {
            if (NULL == (*lv_iter)) {
                AFS_TRACE_OUT_ERROR("The lv[%s] of Volume group[%s] is NULL", (*lv_iter)->m_volname.c_str(),
                    (*vg_iter)->m_uuid.c_str());
                return -1;
            }

            lvol = (logicalVolume *)(*lv_iter);
            if (lvol->m_type == SEG_THIN_POOL) {
                ret = lvol->verifyThinPoolSB();
            }

            if (AFS_SUCCESS != ret) {
                AFS_TRACE_OUT_ERROR("partitionHandler verify thin pool super block failed");
                return -1;
            }
        }
    }

    ret = updateThinLVPartInfo();
    if (0 != ret) {
        AFS_TRACE_OUT_ERROR("updateThinLVPartInfo() failed");
        return -1;
    }

    return 0;
}

/**
 * @brief  解析多磁盘所有分区后，对特殊场景进行处理
 * @return 0 成功
 * 其他   失败
 */
int32_t partitionHandler::updateAllPartitions()
{
    int32_t ret;

    ret = copyImgReaderVectToLVs(); // /在verifyThinPoolSuperBlock之前调用
    if (ret != 0) {
        AFS_TRACE_OUT_ERROR("copyImgReaderVectToLVs failed");
        return ret;
    }

    ret = updateNoMetadataPV();
    if (ret != 0) {
        AFS_TRACE_OUT_ERROR("updateNoMetadataPV failed");
        return ret;
    }

    ret = verifyThinPoolSuperBlock();
    if (ret != 0) {
        AFS_TRACE_OUT_ERROR("verifyThinPoolSuperBlock failed");
        return ret;
    }

    ret = updateLVWithNoMetadataPV();
    if (ret != 0) {
        AFS_TRACE_OUT_ERROR("updateLVWithNoMetadataPV failed");
        return ret;
    }

    return AFS_SUCCESS;
}

/**
 * @brief  将所有磁盘的imgReader复制到每个LV
 * @return 0 成功
 * 其他   失败
 */
int32_t partitionHandler::copyImgReaderVectToLVs()
{
    volumeGroup *grp = NULL;
    logicalVolume *lvol = NULL;
    list<volumeGroup *>::iterator vg_iter;
    vector<logicalVolume *>::iterator lv_iter;
    vector<imgReader *>::iterator img_iter;

    for (vg_iter = m_vgManager.getGroupListAddr().begin(); vg_iter != m_vgManager.getGroupListAddr().end(); ++vg_iter) {
        if (NULL == (*vg_iter)) {
            AFS_TRACE_OUT_ERROR("The vg_iter is NULL");
            return -1;
        }

        grp = (*vg_iter);
        for (lv_iter = grp->m_lvolumes.begin(); lv_iter != grp->m_lvolumes.end(); ++lv_iter) {
            if (NULL == (*lv_iter)) {
                AFS_TRACE_OUT_ERROR("The lv_iter is NULL");
                return -1;
            }

            lvol = (logicalVolume *)(*lv_iter);
            vector<imgReader *> &disk_Readers_vect = lvol->m_disk_readers_vect;
            for (img_iter = m_vect_imgreader.begin(); img_iter != m_vect_imgreader.end(); img_iter++) {
                disk_Readers_vect.push_back((imgReader *)(*img_iter));
            }

            AFS_TRACE_OUT_DBG("Now lv[%s] imgReader vector size after copy from father partition is %u",
                lvol->m_volname.c_str(), lvol->m_disk_readers_vect.size());
        }
    }
    return 0;
}

/**
 * @brief 分析分区
 * @return   0 成功
 * 其他   失败
 *
 */
int32_t partitionHandler::analyzePartitions()
{
    int32_t ret = 0;

    // 获得分区类型
    ret = getPartType();
    AFS_TRACE_OUT_DBG("the disk or image partition type is %d", ret);
    if (ret <= 0) {
        AFS_TRACE_OUT_ERROR("Failed to get the disk or image partition type.");
        return ret;
    }

    // 根据类型创建对象
    if (NULL == createPartHandler()) {
        AFS_TRACE_OUT_ERROR("Cannot analyze the disk or image partition.");
        return AFS_ERR_API;
    }

    // 解析所有的分区
    ret = m_realhandler->parseAllOfPart();
    if (AFS_SUCCESS != ret) {
        AFS_TRACE_OUT_ERROR("Cannot analyze the disk or image partition. PartType=%d", m_type);
        return ret;
    }

    copyRealHandlerInfo();
    return AFS_SUCCESS;
}

/**
 * @brief  拷贝真实分区对象的数据到father partition
 * @return 暂无
 *
 */
void partitionHandler::copyRealHandlerInfo()
{
    list<volumeGroup *> &childVgList = m_realhandler->m_vgManager.getGroupListAddr();
    list<volumeGroup *> &fatherVgList = m_vgManager.getGroupListAddr();
    list<struct pv_no_vg_metadata> &childPvList = m_realhandler->getPvNoMetadataList();
    list<struct pv_no_vg_metadata> &fatherPvList = m_pv_no_vg_metadata;

    vector<struct partition>::iterator father_parts_iter;
    list<volumeGroup *>::iterator father_volume_group_iter;
    list<volumeGroup *>::iterator child_volume_group_iter;
    m_parts_vect.clear();
    father_parts_iter = m_parts_vect.end();
    m_parts_vect.insert(father_parts_iter, m_realhandler->m_parts_vect.begin(), m_realhandler->m_parts_vect.end());

    AFS_TRACE_OUT_DBG("father m_parts_vect size is %u", m_parts_vect.size());
    AFS_TRACE_OUT_DBG("child m_parts_vect size is %u", m_realhandler->m_parts_vect.size());

    for (uint32_t i = 0; i < m_parts_vect.size(); i++) {
        struct partition temp_parts = m_parts_vect[i];
        AFS_TRACE_OUT_DBG("The %d partition's part_id =%d, lv_name =%s, disk_id is %d", i, temp_parts.part_id,
            temp_parts.lvm_info.lv_name, temp_parts.disk_id);
    }

    father_volume_group_iter = fatherVgList.end();
    child_volume_group_iter = childVgList.begin();

    fatherVgList.swap(childVgList);

    if (m_realhandler->m_map_chunk_size.size()) {
        m_map_chunk_size.insert(m_realhandler->m_map_chunk_size.begin(), m_realhandler->m_map_chunk_size.end());
    }

    setPartnumValue(m_realhandler->getPartnum());
    AFS_TRACE_OUT_DBG("father partition set partitions number is %d", getPartnum());

    fatherPvList.swap(childPvList);
    AFS_TRACE_OUT_DBG("father list of PVs that not contain VG metadata is %u", fatherPvList.size());
    AFS_TRACE_OUT_DBG("child list of PVs that not contain VG metadata is %u", childPvList.size());
}

/**
 * @brief 获得分区对象
 * @return 返回分区对象
 *
 */
partitionHandler *partitionHandler::getrealHandler()
{
    return this->m_realhandler;
}

/**
 * @brief 设置分区所占bitmap
 *
 * @param &bitmap  镜像Bitmap
 *
 * @return int32_t 0设置成功 -1设置失败
 *
 */
int32_t partitionHandler::getBitmap(BitMap &bitmap)
{
    AFS_TRACE_OUT_DBG("this is father partitionhandler getBitmap()");
    return 0;
}

/**
 * @brief 获取分区Bitmap
 *
 * @param &bitmap  需要设置的Bitmap对象
 *
 * @return 0:成功 负数：错误ID
 *
 */
int32_t partitionHandler::getDisksBitmap(vector<BitMap *> &bitmap_vect)
{
    int32_t ret = 0;
    vector<BitMap *>::iterator bm_iter = bitmap_vect.begin();
    BitMap *pbitmap;
    uint32_t handler_count = 0;

    for (; bm_iter != bitmap_vect.end(); bm_iter++) {
        m_realhandler = m_vector_realHandler[handler_count++];
        if (NULL == m_realhandler) {
            AFS_TRACE_OUT_ERROR("Pointer m_realhandler is null.");
            return AFS_ERR_INNER;
        }
        pbitmap = (*bm_iter);
        ret = m_realhandler->getBitmap(*pbitmap);
        if (ret != 0) {
            AFS_TRACE_OUT_ERROR("Failed to get bitmap.");
            return ret;
        }
    }

    // 分区其它设置（lvm设置）
    return getOtherManagerModeBitmap(bitmap_vect);
}

/**
 * @brief 设置其它管理模式下的Bitmap
 *
 * @param &bitmap    需要设置的BitMap对象
 *
 * @return 0:成功  负值： 失败（错误ID）
 *
 */
int32_t partitionHandler::getOtherManagerModeBitmap(vector<BitMap *> &bitmap_vect)
{
    int32_t ret = 0;
    vector<BitMap *>::iterator bm_iter = bitmap_vect.begin();
    BitMap *pbitmap = NULL;
    int32_t count = 0;

    for (; bm_iter != bitmap_vect.end(); bm_iter++) {
        AFS_TRACE_OUT_INFO("disk[%d]: pbitmap->bitmapSetRange(1, 2048, 1)", count++);
        pbitmap = (*bm_iter);
        // 分区保留
        ret = pbitmap->bitmapSetRange(1, 2048, 1); // 适应多磁盘,每个磁盘的前2049（含MBR）个扇区为有效数据
        if (ret != 0) {
            return ret;
        }
    }

    // LVM元数据设置
    return m_vgManager.getBitmap(bitmap_vect);
}

bool partitionHandler::AllocBySectors(std::unique_ptr<char[]> &buffer, int64_t length, int64_t &allocSize)
{
    allocSize = length;
    if (length % VMDK_SECTOR_SIZE != 0) {
        allocSize = ((length / VMDK_SECTOR_SIZE) + 1) * VMDK_SECTOR_SIZE;
    }

    buffer = std::make_unique<char[]>(allocSize);
    if (buffer == nullptr) {
        return false;
    }
    CHECK_MEMSET_S_OK(buffer.get(), allocSize, 0, allocSize);
    return true;
}

bool partitionHandler::ReadBySectorsBuff(imgReader *reader, void *buff,
int64_t offset, int64_t length, int32_t annotated)
{
    int64_t needSize = length;
    std::unique_ptr<char[]> bufferPtr;
    if (!AllocBySectors(bufferPtr, length, needSize)) {
        AFS_TRACE_OUT_ERROR("Failed to calloc agf.");
        return false;
    }
    int64_t readLen = reader->read(bufferPtr.get(), offset, needSize, annotated);
    if (needSize != readLen) {
        AFS_TRACE_OUT_ERROR("Failed to read data.");
        return false;
    }

    CHECK_MEMCPY_S_OK(buff, length, bufferPtr.get(), length);
    return true;
}