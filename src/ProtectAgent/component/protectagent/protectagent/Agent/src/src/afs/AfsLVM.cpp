/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 *
 * @file afsLVM.cpp
 * @brief Afs - A C/C++ Library for analyze format of a disk image.
 *
 */

#include <string.h>
#include <climits>
#include <algorithm>
#include <cerrno>
#include "afs/AfsError.h"
#include "afs/ImgReader.h"
#include "afs/LVMReader.h"
#include "afs/AfsLVM.h"
#include "afs/ThinlvSegment.h"
#include "afs/StripeSegment.h"
#include "afs/ThinPoolSegment.h"
#include "securec.h"

/* * @brief 构造体
 *
 * @param handler 分区句柄
 * @param part_vect 分区数组
 * @param part_index 分区索引号
 *
 */
afsLVM::afsLVM(partitionHandler *handler, vector<struct partition> &part_vect, int32_t part_index)
    : afsObject(), m_part_vect(part_vect)
{
    setObjType(OBJ_TYPE_LVM);
    setMagic("LVM");

    m_ptr_reader = handler->getImgReader();
    m_ptr_handler = handler;
    m_part_index = part_index;
    m_pv_offset = 0;
    m_real_part_index = 0;
    m_str_index = 0;
    m_length = 0;
    m_vg = NULL;
    m_flag = 0;
    uuid[UUID_LEN] = 0;
}

/**
 * @brief 析构函数
 */
afsLVM::~afsLVM()
{
    m_vg = NULL;
    m_ptr_reader = NULL;
    m_ptr_handler = NULL;
}

/**
 * @brief 处理segment
 * @param plv 逻辑卷lv
 * @param temp_part 分区信息
 * @param lv_length 长度
 */
int32_t afsLVM::forEachAllOfLVDoSeg(logicalVolume *plv, struct partition &temp_part, uint64_t &lv_length)
{
    int32_t offset_calc = 0;
    SEG_TYPE_ENU type = SEG_NULL;

    if (NULL == m_vg) {
        AFS_TRACE_OUT_ERROR("The space is NULL.");
        return AFS_ERR_LVM_PART;
    }

    list<segment *>::iterator seg_list;
    for (seg_list = plv->m_segments.begin(); seg_list != plv->m_segments.end(); ++seg_list) {
        if (NULL == (*seg_list)) {
            AFS_TRACE_OUT_ERROR("The logicalVolume space is NULL.");
            return AFS_ERR_LVM_PART;
        }

        type = (*seg_list)->getType();
        if (type != SEG_LINEAR && type != SEG_STRIPED && type != SEG_THIN_POOL && type != SEG_THIN) {
            AFS_TRACE_OUT_INFO("The Segment type is not support.");
            return NOT_SUPPORT_FORMAT;
        }

        // 每个segment关联卷组
        (*seg_list)->m_this_vg = plv->m_this_group;
        lv_length += (*seg_list)->m_extent_count * m_vg->m_extent_size;

        if ((0 == offset_calc) && plv->m_type != SEG_THIN) {
            // 给用户展示第一个pv的地址, THIN LV 的首地址在后续会处理
            temp_part.lvm_info.lv_offset = (*seg_list)->getPartitionFirstAddr();
            offset_calc++;
        }

        if ((0 == offset_calc) && plv->m_type == SEG_THIN) { // /thin lv, 需要根据xxx_tdata进行设置stripe_size
            logicalVolume *pPool_lv = (dynamic_cast<thinlvSegment *>(*seg_list))->pool_lv;
            thinpoolSegment *pThin_pool_seg = dynamic_cast<thinpoolSegment *>(pPool_lv->m_segments.back());
            logicalVolume *pPool_data_lv = pThin_pool_seg->findDataLv();
            plv->m_stripe_size = pPool_data_lv->m_stripe_size;
        }
    }

    return AFS_SUCCESS;
}

/**
 * @brief 遍历分析所有的lv
 * @return 分区id
 */
int32_t afsLVM::forEachAllOfLV()
{
    int32_t part_id = 0;
    int32_t ret = AFS_SUCCESS;
    size_t name_len = 0;
    vector<logicalVolume *>::iterator lv_iter;

    if (static_cast<size_t>(m_part_index) > m_part_vect.size() || NULL == m_vg) {
        AFS_TRACE_OUT_ERROR("Cann't parse lvm");
        return AFS_ERR_LVM_PART;
    }

    int32_t start_part_id = (m_part_vect[m_part_index]).part_id;
    uint64_t part_offset = (m_part_vect[m_part_index]).offset;
    uint64_t part_length = (m_part_vect[m_part_index]).length;
    unsigned char part_type = (m_part_vect[m_part_index]).part_type;

    AFS_TRACE_OUT_DBG("afsLVM UUID = %s, m_part_index = %d", uuid, m_part_index);

    // 获得整个lv信息
    for (lv_iter = m_vg->m_lvolumes.begin(); lv_iter != m_vg->m_lvolumes.end(); ++lv_iter) {
        if (NULL == (*lv_iter)) {
            AFS_TRACE_OUT_ERROR("The logicalVolume space is NULL.");
            return AFS_ERR_LVM_PART;
        }

        // 只给用户展示，lv属性为可见，并且是非POOL卷
        if ((*lv_iter)->m_is_visible && ((*lv_iter)->m_type != SEG_THIN_POOL)) {
            if ((*lv_iter)->getFirstPVuuid() == NULL) {
                AFS_TRACE_OUT_DBG("lv[%s] LVM type id is SEG_THIN, not SEG_LINEAR or SEG_STRIPED",
                    (*lv_iter)->m_volname.c_str());
            }

            // 设置lv的信息
            struct partition temp_part;
            CHECK_MEMSET_S_OK(&temp_part, sizeof(temp_part), 0, sizeof(temp_part));
            uint64_t lv_length = 0; // LV分区长度
            temp_part.is_lvm = true;
            temp_part.fstype = AFS_FILESYSTEM_NULL;
            temp_part.is_pv_part = true; // 分区为LVM PV 格式

            // 所在的pv地址，需要根据segment进行打印
            ret = forEachAllOfLVDoSeg((*lv_iter), temp_part, lv_length);
            if (ret == NOT_SUPPORT_FORMAT) {
                continue;
            } else if (ret != AFS_SUCCESS) {
                AFS_TRACE_OUT_ERROR("Cann't parse segment");
                return AFS_ERR_LVM_PART;
            }

            // 设置分区号:
            // 注：只有对用户展示的分区信息才有part_id(用户使用)，pool不给用户展示
            temp_part.part_id = part_id + start_part_id;
            // 生成分区reader需要绑定对应得lv，需要part_id
            (*lv_iter)->set_part_id(temp_part.part_id);
            // 设置分区相关信息
            temp_part.offset = part_offset;
            temp_part.length = part_length;
            temp_part.part_type = part_type;

            // 设置LV大小:单位扇区
            temp_part.lvm_info.lv_length = lv_length;
            name_len = (*lv_iter)->m_volname.length();
            if (name_len >= (size_t)AFS_MAX_LV_NAME) {
                return AFS_ERR_LVM_PART;
            }

            if ((*lv_iter)->m_type == SEG_THIN) { // thin lv的lv_pvUUID特殊处理，用于后续唯一的更新thin LV的lv_offset
                ret = (*lv_iter)->String2UUID((*lv_iter)->m_uuid);
            }

            if (-1 == ret) {
                AFS_TRACE_OUT_ERROR("String2UUID() for thin lv failed");
                return AFS_ERR_LVM_PART;
            }

            CHECK_MEMCPY_S_OK(temp_part.lvm_info.lv_pvUUID, AFS_PV_UUID_LEN + 1, (*lv_iter)->m_first_pv_uuid,
                AFS_PV_UUID_LEN + 1);

            temp_part.lvm_info.lv_update = true;
            CHECK_STRNCPY_S_OK(temp_part.lvm_info.lv_name, AFS_MAX_LV_NAME, (*lv_iter)->m_volname.c_str(),
                (*lv_iter)->m_volname.length());
            temp_part.lvm_info.lv_name[name_len] = '\0';

            if (0 == part_id) {
                // 之前mbr，gpt压入得分区信息进行出组
                ret = getPhysicalPartitionInfo();
                if (AFS_SUCCESS != ret) {
                    AFS_TRACE_OUT_ERROR("afs get physical partition info failed");
                    return ret;
                }
                m_part_vect.pop_back();
            }

            m_part_vect.push_back(temp_part);
            AFS_TRACE_OUT_DBG("m_part_vect add a partition[part_id = %d, lv_name =%s, is_lvm = %d, disk_id =%d]",
                temp_part.part_id,
                temp_part.lvm_info.lv_name,
                temp_part.is_lvm,
                temp_part.disk_id);

            // 连续块大小
            m_ptr_handler->m_map_chunk_size.insert(
                pair<int32_t, uint32_t>(temp_part.part_id, (*lv_iter)->m_chunk_size));
            AFS_TRACE_OUT_INFO("part_id's [%d] m_chunk_size is %u", temp_part.part_id, (*lv_iter)->m_chunk_size);

            part_id++;
        }
    }

    return (part_id - 1);
}

/**
 * @brief 单独保存本次物理分区的信息
 *
 */
int32_t afsLVM::getPhysicalPartitionInfo()
{
    vector<struct partition>::iterator iter = m_part_vect.end() - 1;
    CHECK_MEMCPY_S_OK(&m_physical_part, sizeof(struct partition), &*iter, sizeof(struct partition));
    AFS_TRACE_OUT_DBG("m_physical_part.offset = %llu, .part_id =%d, disk_id=%d", m_physical_part.offset,
        m_physical_part.part_id, m_physical_part.disk_id);
    return AFS_SUCCESS;
}

/**
 * @brief 保存没有VG metadata的PV
 *
 */
int32_t afsLVM::saveNoMetadataPv()
{
    vector<struct partition>::iterator iter = m_part_vect.end() - 1;
    struct pv_no_vg_metadata temp_pv;
    list<struct pv_no_vg_metadata> &pv_no_metadata = m_ptr_handler->getPvNoMetadataList();

    CHECK_MEMSET_S_OK(&temp_pv, sizeof(struct pv_no_vg_metadata), 0, sizeof(struct pv_no_vg_metadata));
    temp_pv.pv_disk_id = m_ptr_handler->getDiskNum();
    temp_pv.pv_offset = iter->offset;
    CHECK_MEMCPY_S_OK(temp_pv.pv_uuid, UUID_LEN + 1, uuid, UUID_LEN + 1);
    AFS_TRACE_OUT_DBG("the no metadata pv [%s] disk id is %d, offset to disk is %llu",
        temp_pv.pv_uuid,
        temp_pv.pv_offset,
        temp_pv.pv_disk_id);

    pv_no_metadata.push_back(temp_pv);
    AFS_TRACE_OUT_DBG("pv_no_metadata size is %u", pv_no_metadata.size());
    return AFS_SUCCESS;
}

/**
 * @brief PV UUID string 转换成 UUID标准格式
 * string格式："\"o3OAi1-IUIz-0Svv-vum1-kknj-kHp3-sVT4qq\""
 * UUID格式  ：  o3OAi1IUIz0Svvvum1kknjkHp3sVT4qq
 *
 * @return 返回转换结果，0表示成功，-1 表示失败
 */
int32_t afsLVM::String2UUID(string origin_uuid, char *target_uuid, int32_t target_uuid_len)
{
    const char *pOriginUUID = origin_uuid.c_str();
    if (origin_uuid.size() != 40) {
        AFS_TRACE_OUT_ERROR("The size of String[%d] is not right", origin_uuid.size());
        return -1;
    }

    CHECK_MEMCPY_S_OK(target_uuid, target_uuid_len, pOriginUUID + 1, 6);
    CHECK_MEMCPY_S_OK(target_uuid + 6, target_uuid_len - 6, pOriginUUID + 8, 4);
    CHECK_MEMCPY_S_OK(target_uuid + 10, target_uuid_len - 10, pOriginUUID + 13, 4);
    CHECK_MEMCPY_S_OK(target_uuid + 14, target_uuid_len - 14, pOriginUUID + 18, 4);
    CHECK_MEMCPY_S_OK(target_uuid + 18, target_uuid_len - 18, pOriginUUID + 23, 4);
    CHECK_MEMCPY_S_OK(target_uuid + 22, target_uuid_len - 22, pOriginUUID + 28, 4);
    CHECK_MEMCPY_S_OK(target_uuid + 26, target_uuid_len - 26, pOriginUUID + 33, 6);
    target_uuid[UUID_LEN] = '\0';

    AFS_TRACE_OUT_DBG("string uuid convert to chars uuid is %s", target_uuid);

    return 0;
}

/**
 * @brief   更新物理卷的偏移（ physicalVolume 类的 offset属性）
 *
 * @return 0：成功， AFS_ERR_INNER：程序内部错误
 */
int32_t afsLVM::UpdatePhysicalPartition(string pv_uuid_string, uint64_t pv_offset)
{
    char target_uuid[UUID_LEN + 1];
    physicalVolume *targetPV = NULL;
    CHECK_MEMSET_S_OK(target_uuid, UUID_LEN + 1, 0, UUID_LEN + 1);

    for (uint32_t i = 0; i < m_vg->m_pvolumes.size(); i++) {
        string tempString;
        targetPV = m_vg->m_pvolumes.at(i);
        if (0 != String2UUID(targetPV->uuid, target_uuid, UUID_LEN + 1)) { // uuid格式转换错误，后续处理也会错误
            AFS_TRACE_OUT_ERROR("String2UUID from %s to uuid is error", targetPV->uuid.c_str());
            return AFS_ERR_INNER;
        }

        tempString = target_uuid;
        if (0 == tempString.compare(pv_uuid_string)) {
            targetPV->offset = pv_offset;
            targetPV->disk_id = m_ptr_handler->getDiskNum();
            AFS_TRACE_OUT_INFO("targetPV that need to be updated is %s, new pv_offset is %llu, disk id is %d",
                target_uuid, pv_offset, targetPV->disk_id);
            break;
        }
    }

    return 0;
}

/**
 * @brief 更新LV的分区信息（主要是偏移和首地址磁盘ID）
 *
 * @return 0：成功， -1：失败
 *
 */
int32_t afsLVM::UpdateLVPartitionInfo(int32_t new_lvm_num, uint64_t pv_disk_offset, int32_t pv_diskId)
{
    struct partition *temp_part;
    string pv_uuid_string;
    string lv_string;
    int32_t ret;
    uuid[UUID_LEN] = '\0';
    pv_uuid_string = uuid;

    for (int32_t i = 0; i < m_part_index + new_lvm_num + 1; i++) {
        temp_part = &m_part_vect[i];
        if (temp_part->is_lvm == 0) { // 不是LVM，不需要更新
            continue;
        }

        if (temp_part->lvm_info.lv_update) { // 标志位，表明该LV需要更新
            temp_part->lvm_info.lv_pvUUID[UUID_LEN] = '\0';
            lv_string = temp_part->lvm_info.lv_pvUUID;
            if (pv_uuid_string.compare(lv_string) == 0) { // 找到需要更新的一个LV
                AFS_TRACE_OUT_DBG("find a lv [part id: %d] that need update[pv_offset=%llu]", i, pv_disk_offset);
                temp_part->lvm_info.lv_offset += pv_disk_offset; // 更新LV相对磁盘的偏移
                temp_part->disk_id = pv_diskId;                  // disk_id应该不变。。。。
                temp_part->lvm_info.lv_update = false;
            }
        }
    }

    ret = UpdatePhysicalPartition(pv_uuid_string, pv_disk_offset);
    return ret;
}

/**
 * @brief 设置一个分区中关于lv的信息
 *
 * @return lv的个数, 负数：错误信息
 *
 */
int32_t afsLVM::setPartInfo()
{
    int32_t ret = AFS_SUCCESS;

    ret = forEachAllOfLV();
    if (ret >= -1) {
        AFS_TRACE_OUT_INFO("afsLVM parse %d logical volumes", ret + 1);
        UpdateLVPartitionInfo(ret, m_physical_part.offset, m_physical_part.disk_id);
    }

    return ret;
}

/**
 * @brief 分析lvm的格式
 *
 * @param real_part_num 真正分区数目
 *
 * @return NOT_LVM_FORMAT：非lvm格式
 * AFS_ERR_LVM_VERSION：版本不支持
 * AFS_ERR_LVM_PART：数据出错无法分析
 * AFS_ERR_API：系统调用出错
 */
int32_t afsLVM::parseLVMFormat(int32_t real_part_num)
{
    int32_t ret = 0;

    //  检测输入参数设置分区偏移
    if (static_cast<size_t>(m_part_index) > m_part_vect.size() || NULL == m_ptr_reader) {
        AFS_TRACE_OUT_ERROR("parseLVMFormat's param is error");
        return AFS_ERR_LVM_PART;
    }

    m_pv_offset = (m_part_vect[m_part_index]).offset;
    AFS_TRACE_OUT_DBG("m_pv_offset is %llu, length is %llu", (unsigned long long)m_pv_offset,
        (unsigned long long)(m_part_vect[m_part_index]).length);

    m_real_part_index = real_part_num;
    AFS_TRACE_OUT_DBG("m_real_part_index is %d, m_part_index = %d", m_real_part_index, m_part_index);

    // 分析分区
    // 函数名字
    ret = parseLvmInfo();
    if (NOT_LVM_FORMAT == ret) {
        AFS_TRACE_OUT_INFO("Not a lvm parttion");
        return NOT_LVM_FORMAT;
    } else if (AFS_VOLUME_GROUP_EXIST == ret) {
        AFS_TRACE_OUT_INFO("enter AFS_VOLUME_GROUP_EXIST macro");
        ret = getPhysicalPartitionInfo();
        if (AFS_SUCCESS != ret) {
            AFS_TRACE_OUT_ERROR("afs get physical partition info failed");
            return ret;
        }
        ret = UpdateLVPartitionInfo(0, m_physical_part.offset, m_physical_part.disk_id);
        if (AFS_SUCCESS != ret) {
            AFS_TRACE_OUT_ERROR("afs update LV's partition info failed");
            return ret;
        }
        return AFS_VOLUME_GROUP_EXIST;
    } else if (AFS_PV_NO_VG_METADAT == ret) {
        AFS_TRACE_OUT_INFO("enter AFS_PV_NO_VG_METADAT macro");
        ret = saveNoMetadataPv();
        if (AFS_SUCCESS != ret) {
            AFS_TRACE_OUT_ERROR("afs save PV which not contains VG metadat failed");
            return ret;
        }
        return AFS_PV_NO_VG_METADAT;
    } else if (0 > ret) {
        // 错误值
        AFS_TRACE_OUT_ERROR("Cann't parse lvm");
        return ret;
    }

    // 设置lvm的分区
    ret = setPartInfo();
    if (AFS_ERR_LVM_PART == ret) {
        AFS_TRACE_OUT_ERROR("Cann't set part info");
        return AFS_ERR_LVM_PART;
    }

    return ret;
}

/* * @brief 解析lvm的label头
 *
 * @param header 头信息
 *
 * @return NOT_LVM_FORMAT：非lvm格式
 * AFS_ERR_LVM_VERSION：版本不支持
 * AFS_ERR_LVM_PART：数据出错无法分析
 * AFS_ERR_API：系统调用出错
 * AFS_SUCCESS：成功
 */
int32_t afsLVM::lvmLabelHeadHandle(PV_LABEL_HEADER *header)
{
    int32_t ret = 0;
    int32_t read_size = 0;
    uint64_t offset = 0;
    char buffer[SECTOR_SIZE] = {0};
    PV_LABEL *label = NULL;
    int32_t length = 0;

    // 1.计算label扇区
    offset = (header->pv_labeloffset / SECTOR_SIZE) + m_pv_offset;
    read_size = m_ptr_reader->readSector(buffer, offset, 1);
    if (1 != read_size) {
        AFS_TRACE_OUT_ERROR("Cannt read img");
        return AFS_ERR_IMAGE_READ;
    }

    label = (PV_LABEL *)&buffer[0];

    // 2.计算metadata
    offset = m_pv_offset + ((label->pv_offset_low + label->pv_offset_high) / SECTOR_SIZE);

    AFS_TRACE_OUT_DBG("metadata area offset is %llu", offset);

    // len大小(元数据区域：2048扇区，但是元数据：8扇区左右（目前是，3个扇区）)
    length = (label->pv_length + SECTOR_SIZE - 1) / SECTOR_SIZE;
    AFS_TRACE_OUT_DBG("pv metadata size is %d (512)", length);
    if (length <= 0) {
        AFS_TRACE_OUT_ERROR("The length of metadata is zero.");
        return AFS_ERR_LVM_PART;
    }

    char *metadata = new char[(uint32_t)(length * SECTOR_SIZE)]();
    if (NULL == metadata) {
        AFS_TRACE_OUT_ERROR("Cannt new metadata");
        return AFS_ERR_API;
    }

    ret = m_ptr_reader->readSector(metadata, offset, length);
    if (length != ret) {
        AFS_TRACE_OUT_ERROR("Cannt read img");
        delete[] metadata;
        return AFS_ERR_IMAGE_READ;
    }

    // 字符串最后一位添加结尾符
    metadata[label->pv_length] = 0;
    m_str_metadata = metadata;

    // 3.解析metadata
    ret = parseMetadata();
    delete[] metadata;

    return ret;
}

/**
 * @brief 解析lvm的label头，扫描磁盘镜像，判断磁盘分区是否是LVM镜像管理方式
 *
 * @return NOT_LVM_FORMAT：非lvm格式
 * AFS_ERR_LVM_VERSION：版本不支持
 * AFS_ERR_LVM_PART：数据出错无法分析
 * AFS_ERR_API：系统调用出错
 * AFS_SUCCESS：成功
 */
int32_t afsLVM::parseLvmInfo()
{
    bool flag = false;
    int32_t read_size = 0;
    PV_LABEL_HEADER *header = NULL;
    uint64_t offset = 0;
    int32_t ret = NOT_LVM_FORMAT;
    char buffer[SECTOR_SIZE] = {0};

    // 1.Note,RedHat比较特殊，注意事项如下
    // According to [REDHAT] the physical volume label can be
    // stored in any of the first four sectors.
    for (int32_t i = 0; i < LVM_LABEL_SECTOR_RANGE; i++) {
        offset = m_pv_offset + i;

        read_size = m_ptr_reader->readSector(buffer, offset, 1);
        if (1 != read_size) {
            AFS_TRACE_OUT_ERROR("Cant read disk");
            return AFS_ERR_IMAGE_READ;
        }

        header = (PV_LABEL_HEADER *)&buffer[0];
        if (strncmp(header->pv_name, LVM_SIGNATURE, LVM_SIGLEN) != 0) {
            continue;
        }

        // 打印lvm2版本信息
        char sVersion[LVM_MAGIC_LEN + 1];
        CHECK_MEMCPY_S_OK(sVersion, sizeof(sVersion), header->pv_vermagic, LVM_MAGIC_LEN);
        sVersion[LVM_MAGIC_LEN] = '\0';

        AFS_TRACE_OUT_DBG("LVM's Version-Magic is %s", sVersion);

        // LVM版本判断（目前支持lvm2）
        if (strncmp(header->pv_vermagic, LVM_2_VERSION_MAGIC, LVM_MAGIC_LEN) != 0) {
            AFS_TRACE_OUT_ERROR("LVM Don't support %s", header->pv_vermagic);
            return AFS_ERR_LVM_VERSION;
        }

        // PV UUID
        CHECK_MEMCPY_S_OK(uuid, UUID_LEN + 1, header->pv_uuid, UUID_LEN);
        // 设置标志位
        flag = true;

        break;
    }

    if (flag) {
        setPartPVType();
        AFS_TRACE_OUT_DBG("pv's uuid is %s, and the header pv label offset is %llu", header->pv_uuid,
            header->pv_labeloffset);

        if (header->pv_labeloffset == 0) {
            return AFS_PV_NO_VG_METADAT;
        }
        ret = lvmLabelHeadHandle(header);
    }

    return ret;
}

/**
 * @brief 解析序列号
 *
 * @return
 * AFS_ERR_LVM_PART 错误的分区
 * 序列号
 */
int32_t afsLVM::parseMetadataVgInfoSeqno()
{
    int32_t base = 10;

    string sseqno = lvmParseMetadataString("seqno");
    if (sseqno.empty()) {
        AFS_TRACE_OUT_ERROR("Can't parse meatadata");
        return -1;
    }

    initErrorNo();

    int32_t seq = strtol(sseqno.c_str(), NULL, base);
    /* Check for various possible errors */
    if ((errno == ERANGE && (seq == static_cast<int32_t>(INT_MAX) || seq == static_cast<int32_t>(INT_MIN))) ||
        (errno != 0 && seq == 0)) {
        AFS_TRACE_OUT_ERROR("LVM Metadata is error, The API Error Number is %d", errno);
        return -1;
    }

    return seq;
}

/**
 * @brief 解析extent size
 *
 * @return
 * AFS_ERR_LVM_PART 错误的分区
 * extent size
 */
uint32_t afsLVM::parseMetadataVgInfoExtentSize()
{
    string::size_type i_start;
    string::size_type j_num;
    int32_t base = 10;

    i_start = m_str_metadata.find("extent_size", m_str_index);
    if (string::npos == i_start) {
        AFS_TRACE_OUT_ERROR("LVM format is error.");
        return static_cast<uint32_t>(-1);
    }

    j_num = m_str_metadata.find("\n", i_start);
    if (string::npos == j_num) {
        AFS_TRACE_OUT_ERROR("LVM format is error.");
        return static_cast<uint32_t>(-1);
    }

    m_str_index = j_num;
    string sextent_size = m_str_metadata.substr(i_start + LVM_FIX_SPACE_FORMAT + strlen("extent_size") - 1,
        j_num - (i_start + LVM_FIX_SPACE_FORMAT + strlen("extent_size") - 1));

    initErrorNo();

    uint32_t extent_size = strtoul(sextent_size.c_str(), NULL, base);
    /* Check for various possible errors */
    if ((errno == ERANGE && extent_size == static_cast<uint32_t>(ULONG_MAX)) || (errno != 0 && extent_size == 0)) {
        AFS_TRACE_OUT_ERROR("LVM Metadata is error, The API Error Number is %d", errno);
        return static_cast<uint32_t>(-1);
    }

    return extent_size;
}

/**
 * @brief 解析卷组
 *
 * @return
 * AFS_ERR_LVM_PART：数据出错无法分析
 * AFS_ERR_API：系统调用出错
 * AFS_SUCCESS：成功
 */
int32_t afsLVM::parseMetadataVgInfo()
{
    string::size_type length = 0;
    string volname;
    string suuid;

    m_length = m_str_metadata.length();
    length = m_length;

    m_str_index = m_str_metadata.find("{", m_str_index);
    if (m_str_index > static_cast<string::size_type>(length) || m_str_index == string::npos) {
        AFS_TRACE_OUT_ERROR("Cannt found VG info\n");

        return AFS_ERR_LVM_PART;
    }

    // 减去一个空格，得到卷组名
    volname = m_str_metadata.substr(0, m_str_index - 1);

    // 1.1 UUID
    suuid = lvmParseMetadataString("id");
    if (suuid.empty()) {
        AFS_TRACE_OUT_ERROR("Can't parse meatadata");
        return AFS_ERR_LVM_PART;
    }

    // 1.2 seqno
    int32_t seq = parseMetadataVgInfoSeqno();
    if (-1 == seq) {
        AFS_TRACE_OUT_ERROR("Can'nt parse sequence of lvm_vg");
        return AFS_ERR_LVM_PART;
    }

    // 1.3 LVM版本（"lvm2"）centos6.0 没有该关键字
    // 1.6 extent_size,比较重要,单元extent的大小。 ***
    uint32_t extent_size = parseMetadataVgInfoExtentSize();
    if (static_cast<uint32_t>(-1) == extent_size || 0 >= extent_size) {
        AFS_TRACE_OUT_ERROR("Can'nt parse extent_size of lvm_vg");
        return AFS_ERR_LVM_PART;
    }

    // 寻找卷组
    m_vg = m_ptr_handler->m_vgManager.findVolgroup(suuid);
    if (!m_vg) {
        m_vg = m_ptr_handler->m_vgManager.addVolgroup(suuid, volname, seq, extent_size);
        if (NULL == m_vg) {
            AFS_TRACE_OUT_ERROR("The api of addVolgroup failed");
            return AFS_ERR_API;
        }
    } else {
        // 如果发现卷组，则说明已经进行了处理不需要进行重复的处理
        AFS_TRACE_OUT_DBG("LVM: volume group exists");
        return AFS_VOLUME_GROUP_EXIST; // modify by zyj
    }

    return AFS_SUCCESS;
}

/**
 * @brief 解析device size
 *
 * @return
 * -1
 * dev_size
 */
uint64_t afsLVM::parseMetadataPvInfoDevSize()
{
    int32_t base = 10;

    string sdev_size = lvmParseMetadataString("dev_size");
    if (sdev_size.empty()) {
        AFS_TRACE_OUT_ERROR("Can't parse meatadata");
        return static_cast<uint64_t>(-1);
    }

    initErrorNo();
    uint64_t dev_size = strtoull(sdev_size.c_str(), NULL, base);
    /* Check for various possible errors */
    if ((errno == ERANGE && dev_size == static_cast<uint64_t>(ULLONG_MAX)) || (errno != 0 && dev_size == 0)) {
        AFS_TRACE_OUT_ERROR("LVM Metadata is error, The API Error Number is %d", errno);
        return static_cast<uint64_t>(-1);
    }

    return dev_size;
}

/**
 * @brief 解析pe_start
 *
 * @return
 * -1
 * dev_size
 */
uint32_t afsLVM::parseMetadataPvInfoPeStart()
{
    int32_t base = 10;

    string spe_start = lvmParseMetadataString("pe_start");
    if (spe_start.empty()) {
        AFS_TRACE_OUT_ERROR("Can't parse meatadata");
        return static_cast<uint32_t>(-1);
    }

    errno = 0;
    uint32_t pe_start = strtoul(spe_start.c_str(), NULL, base);
    /* Check for various possible errors */
    if ((errno == ERANGE && pe_start == static_cast<uint32_t>(UINT_MAX)) || (errno != 0 && pe_start == 0)) {
        AFS_TRACE_OUT_ERROR("LVM Metadata is error, The API Error Number is %d", errno);
        return static_cast<uint32_t>(-1);
    }

    return pe_start;
}

/**
 * @brief 解析pe_count
 *
 * @return
 * -1
 * dev_size
 */
uint32_t afsLVM::parseMetadataPvInfoPeCount()
{
    int32_t base = 10;

    string spe_count = lvmParseMetadataString("pe_count");
    if (spe_count.empty()) {
        AFS_TRACE_OUT_ERROR("Can't parse meatadata");
        return static_cast<uint32_t>(-1);
    }

    initErrorNo();

    uint32_t pe_count = strtoul(spe_count.c_str(), NULL, base);
    /* Check for various possible errors */
    if ((errno == ERANGE && pe_count == static_cast<uint32_t>(UINT_MAX)) || (errno != 0 && pe_count == 0)) {
        AFS_TRACE_OUT_ERROR("LVM Metadata is error, The API Error Number is %d", errno);
        return static_cast<uint32_t>(-1);
    }

    return pe_count;
}

/**
 * @brief 解析pv
 *
 * @param c_num pv索引号
 * @return
 * NULL
 * pv
 */
int32_t afsLVM::pvolume(int32_t c_num)
{
    int32_t ret = AFS_SUCCESS;
    int32_t disk_id = m_ptr_handler->getDiskNum();

    string sid = lvmParseMetadataString("id");
    if (sid.empty()) {
        AFS_TRACE_OUT_ERROR("Can't parse meatadata");
        return AFS_ERR_LVM_PART;
    }

    // 设备名
    string sdev = lvmParseMetadataString("device");
    if (sdev.empty()) {
        AFS_TRACE_OUT_ERROR("Can't parse device of meatadata");
        return AFS_ERR_LVM_PART;
    }

    uint64_t dev_size = parseMetadataPvInfoDevSize();
    if (static_cast<uint64_t>(-1) == dev_size) {
        AFS_TRACE_OUT_ERROR("Can't parse dev_size of meatadata");
        return AFS_ERR_LVM_PART;
    }

    // 找PV的pe_start字段
    uint32_t pe_start = parseMetadataPvInfoPeStart();
    if (static_cast<uint32_t>(-1) == pe_start) {
        AFS_TRACE_OUT_ERROR("Can't parse pe_start of meatadata");
        return AFS_ERR_LVM_PART;
    }

    uint32_t pe_count = parseMetadataPvInfoPeCount();
    if (static_cast<uint32_t>(-1) == pe_count) {
        AFS_TRACE_OUT_ERROR("Can't parse pe_count of meatadata");
        return AFS_ERR_LVM_PART;
    }

    physicalVolume *new_pv = m_vg->findPhysicalVolume(sid);
    AFS_TRACE_OUT_DBG("VG[%s] contains a physical volume which sid is %s", m_vg->m_volname.c_str(), sid.c_str());

    if (NULL == new_pv) {
        char uuid2[UUID_LEN + 1];
        if (0 != String2UUID(sid, uuid2, UUID_LEN + 1)) {
            return AFS_ERR_LVM_PART;
        }

        new_pv = m_vg->addPhysicalVolume(sdev, sid, dev_size, pe_start, pe_count, m_pv_offset, disk_id,
            m_real_part_index, uuid2);
        if (NULL == new_pv) {
            AFS_TRACE_OUT_ERROR("afs add physical volume failed");
            return AFS_ERR_LVM_PART;
        }

        AFS_TRACE_OUT_DBG("Physical volume[%s] has been added into volumes vector", uuid2);
        new_pv->pv_num = c_num;
    } else {
        // 跨分区需要更新
        m_vg->updatePVAddr(m_real_part_index, m_pv_offset);
        AFS_TRACE_OUT_DBG("m_vg->updatePVAddr");
    }

    return ret;
}

/* * @brief 解析物理卷
 *
 * @return
 * AFS_ERR_LVM_PART：数据出错无法分析
 * AFS_ERR_API：系统调用出错
 * AFS_SUCCESS：成功
 */
int32_t afsLVM::parseMetadataPvInfo()
{
    // 2.1解析物理卷字段
    string pv_header("pv");
    int32_t c_num = 0;
    char pv_num[11] = {0};
    string pv("physical_volumes");
    int32_t ret = AFS_SUCCESS;

    m_str_index = m_str_metadata.find(pv, 0);
    if (m_str_index == pv.npos) {
        AFS_TRACE_OUT_ERROR("Cannt read metadata\n");
        return AFS_ERR_LVM_PART;
    }

    CHECK_VSNPRINTF_S_OK(pv_num, sizeof(pv_num), sizeof(pv_num) - 1, "%d", c_num);
    pv_num[sizeof(pv_num) - 1] = '\0';

    string::size_type index = 0;

    // 2.2解析pv字段例如：pv0  pv1  pv2
    while ((index = m_str_metadata.find(pv_header + pv_num, m_str_index)) != m_str_metadata.npos) {
        m_str_index = index;
        // 创建空间
        // 找PV的pe_start字段
        ret = pvolume(c_num);
        if (AFS_SUCCESS != ret) {
            AFS_TRACE_OUT_ERROR("The API of the new is failed");
            return AFS_ERR_LVM_PART;
        }

        c_num++;
        CHECK_VSNPRINTF_S_OK(pv_num, sizeof(pv_num), sizeof(pv_num) - 1, "%d", c_num);
        pv_num[sizeof(pv_num) - 1] = '\0';
    }

    return ret;
}

/* * @brief 按照类型解析
 *
 * @return
 * AFS_ERR_LVM_PART：数据出错无法分析
 * AFS_ERR_API：系统调用出错
 * AFS_SUCCESS：成功
 */
void *afsLVM::segmentTypeHandle(SEG_TYPE_ENU type)
{
    segment *psegment = NULL;
    int32_t ret = 0;

    // 根据不同的类型添加属性
    switch (type) {
        // linear创建出来的lv，也是stripe模式
        case SEG_LINEAR:
        // 性能优化，按照条状进行写
        case SEG_STRIPED:
            psegment = new stripeSegment();
            break;
        case SEG_THIN_POOL:
            psegment = new thinpoolSegment();
            break;
        case SEG_THIN:
            psegment = new thinlvSegment();
            break;
        default:
            AFS_TRACE_OUT_INFO("Cann't support format of segment:%d", type);
            psegment = new segment();
    }

    if (NULL == psegment) {
        AFS_TRACE_OUT_ERROR("Cann't new space");
        return NULL;
    }

    ret = psegment->setSegProp(this);
    if (AFS_SUCCESS != ret) {
        delete psegment;
        AFS_TRACE_OUT_ERROR("Cann't parse space");
        return NULL;
    }

    return psegment;
}

/**
 * @brief 获取extent count
 * @return
 * -1
 * AFS_SUCCESS
 */
uint32_t afsLVM::parseLvSegmentStartExtent()
{
    int32_t base = 10;

    // start_extent字段
    string str = lvmParseMetadataString("start_extent");
    if (str.empty()) {
        AFS_TRACE_OUT_ERROR("Can't parse meatadata");
        return static_cast<uint32_t>(-1);
    }

    errno = 0;
    uint32_t start_extent = strtoul(str.c_str(), NULL, base);
    /* Check for various possible errors */
    if ((errno == ERANGE && (start_extent == static_cast<uint32_t>(UINT_MAX))) || (errno != 0 && start_extent == 0)) {
        AFS_TRACE_OUT_ERROR("LVM Metadata is error, The API Error Number is %d", errno);
        return static_cast<uint32_t>(-1);
    }

    return start_extent;
}

/**
 * @brief 获取 stripe size
 * @return
 * -1
 * AFS_SUCCESS
 */
uint32_t afsLVM::parseSegStripeSize()
{
    int32_t base = 10;

    string str = lvmParseMetadataString("stripe_size");
    if (str.empty()) {
        AFS_TRACE_OUT_ERROR("Can't parse meatadata");
        return 0;
    }

    errno = 0;
    int32_t stripe_size = strtol(str.c_str(), NULL, base);
    /* Check for various possible errors */
    if ((errno == ERANGE && (stripe_size == INT_MAX)) || (errno != 0 && stripe_size == 0)) {
        AFS_TRACE_OUT_ERROR("LVM Metadata is error, The API Error Number is %d", errno);
        return 0;
    }

    return static_cast<uint32_t>(stripe_size);
}

/**
 * @brief 获取extent count
 * @return
 * -1
 * AFS_SUCCESS
 */
uint32_t afsLVM::parseLvSegmentExtentCount()
{
    int32_t base = 10;

    string str = lvmParseMetadataString("extent_count");
    if (str.empty()) {
        AFS_TRACE_OUT_ERROR("Can't parse meatadata");
        return static_cast<uint32_t>(-1);
    }

    errno = 0;

    int32_t extent_count = strtol(str.c_str(), NULL, base);
    /* Check for various possible errors */
    if ((errno == ERANGE && (extent_count == INT_MAX)) || (errno != 0 && extent_count == 0)) {
        AFS_TRACE_OUT_ERROR("LVM Metadata is error, The API Error Number is %d", errno);
        return static_cast<uint32_t>(-1);
    }

    return static_cast<uint32_t>(extent_count);
}

/**
 * @brief 解析单个segment
 * @param *lv        LV对象
 * @param *sseg_num  segment数
 * @return 0：分析成功
 * 负数：错误ID
 */
int32_t afsLVM::parseLvSegmentSingle(logicalVolume *lv, char *sseg_num)
{
    string segment_header("segment");
    // 创建空间
    segment lv_seg;

    // segment绑定vg
    lv_seg.m_this_vg = m_vg;

    m_str_index = m_str_metadata.find(segment_header + sseg_num, m_str_index);
    if (string::npos == m_str_index) {
        AFS_TRACE_OUT_ERROR("LVM format is error.");
        return AFS_ERR_LVM_PART;
    }

    // start_extent字段
    lv_seg.m_start_extent = parseLvSegmentStartExtent();
    if (static_cast<uint32_t>(-1) == lv_seg.m_start_extent) {
        AFS_TRACE_OUT_ERROR("Cannt parse extent count");
        return AFS_ERR_LVM_PART;
    }

    // extent_count字段
    lv_seg.m_extent_count = parseLvSegmentExtentCount();
    if (static_cast<uint32_t>(-1) == lv_seg.m_extent_count || 0 == lv_seg.m_extent_count) {
        AFS_TRACE_OUT_ERROR("Cannt parse extent count");
        return AFS_ERR_LVM_PART;
    }

    // 解析类型（type）
    string segtype_str = lvmParseMetadataString("type");
    if (segtype_str.empty()) {
        AFS_TRACE_OUT_ERROR("Segment Type not exist");
        return AFS_ERR_LVM_PART;
    }

    // 关联物理卷和逻辑卷
    lv_seg.setType(segtype_str);
    SEG_TYPE_ENU type = lv_seg.getType();
    if (SEG_NULL == type) {
        AFS_TRACE_OUT_ERROR("Can't support lv-segment type");
        return AFS_ERR_LVM_PART;
    }

    segment *p_seg = static_cast<segment *>(segmentTypeHandle(type));
    if (NULL == p_seg) {
        AFS_TRACE_OUT_ERROR("Can't support lv-segment type");
        return AFS_ERR_LVM_PART;
    }

    // 3个公共成员
    p_seg->m_start_extent = lv_seg.m_start_extent;
    p_seg->m_extent_count = lv_seg.m_extent_count;
    p_seg->m_segtype = lv_seg.m_segtype;
    p_seg->m_this_lv = lv;

    // 设置类型
    lv->m_type = lv_seg.getType();

    uint32_t temp_stripe_size = 0;
    if (SEG_STRIPED == type) {
        temp_stripe_size = ((stripeSegment *)p_seg)->m_stripe_size;
        if (lv->m_stripe_size == 0) {
            lv->m_stripe_size = temp_stripe_size;
        } else if (lv->m_stripe_size > temp_stripe_size && temp_stripe_size != 0) {
            lv->m_stripe_size = temp_stripe_size;
        }
    }

    AFS_TRACE_OUT_DBG("the %sth segment type is %s, segment stripe size is %u sectors", sseg_num, segtype_str.c_str(),
        temp_stripe_size);

    // 关联卷组
    p_seg->m_this_vg = m_vg;
    // 添加
    lv->m_segments.push_back(p_seg);
    return AFS_SUCCESS;
}

/* * @brief 解析segment
 *
 * @return
 * AFS_ERR_LVM_PART：数据出错无法分析
 * AFS_ERR_API：系统调用出错
 * AFS_SUCCESS：成功
 */
int32_t afsLVM::parseLvSegment(logicalVolume *lv)
{
    int32_t c_num = 1;
    char sseg_num[11] = {0};

    CHECK_VSNPRINTF_S_OK(sseg_num, sizeof(sseg_num), sizeof(sseg_num) - 1, "%d", c_num);
    sseg_num[sizeof(sseg_num) - 1] = '\0';
    AFS_TRACE_OUT_DBG("afs parse Lv[%s] Segment", lv->m_volname.c_str());

    // 遍历segment
    for (int32_t i = 0; i < lv->m_segment_count; i++) {
        int32_t ret = parseLvSegmentSingle(lv, sseg_num);
        if (ret != AFS_SUCCESS) {
            AFS_TRACE_OUT_ERROR("Cannt parse segment");
            return AFS_ERR_LVM_PART;
        }

        c_num++;
        CHECK_VSNPRINTF_S_OK(sseg_num, sizeof(sseg_num), sizeof(sseg_num) - 1, "%d", c_num);
        sseg_num[sizeof(sseg_num) - 1] = '\0';
    }

    AFS_TRACE_OUT_DBG("lv: %s segment stripe size is %u", lv->m_volname.c_str(), lv->m_stripe_size);

    return AFS_SUCCESS;
}

/* * @brief 初始化资源
 *
 * @return
 * AFS_ERR_LVM_PART：数据出错无法分析
 * AFS_ERR_API：系统调用出错
 * AFS_SUCCESS：成功
 */
int32_t afsLVM::initSegmentResource()
{
    if (NULL == m_vg) {
        AFS_TRACE_OUT_ERROR("The space is NULL.");
        return AFS_ERR_LVM_PART;
    }

    segment_init_info info;
    info.reader = m_ptr_reader;
    vector<logicalVolume *>::iterator lv_iter;
    list<segment *>::iterator seg_iter;
    int32_t ret = AFS_SUCCESS;

    // 遍历lv
    for (lv_iter = m_vg->m_lvolumes.begin(); lv_iter != m_vg->m_lvolumes.end(); ++lv_iter) {
        if (NULL == (*lv_iter)) {
            AFS_TRACE_OUT_ERROR("The logicalVolume space is NULL.");
            return AFS_ERR_LVM_PART;
        }
        // 遍历segment
        for (seg_iter = (*lv_iter)->m_segments.begin(); seg_iter != (*lv_iter)->m_segments.end(); ++seg_iter) {
            ret = (*seg_iter)->initSegment(&info);
            if (AFS_SUCCESS != ret) {
                AFS_TRACE_OUT_ERROR("Segment Type not exist");
                return ret;
            }
        }
    }

    return ret;
}

/**
 * @brief 解析segment count 字段
 * @return
 * -1
 * seg_cnt
 */
int32_t afsLVM::parseMetadataLvInfoSegCnt()
{
    int32_t base = 10;

    // 找PV的pe_start字段
    string sseg_cnt = lvmParseMetadataString("segment_count");
    if (sseg_cnt.empty()) {
        AFS_TRACE_OUT_ERROR("Can't parse meatadata");
        return -1;
    }

    errno = 0;
    int32_t seg_cnt = strtol(sseg_cnt.c_str(), NULL, base);
    /* Check for various possible errors */
    if ((errno == ERANGE && (seg_cnt == static_cast<int32_t>(INT_MAX) || seg_cnt == static_cast<int32_t>(INT_MIN))) ||
        (errno != 0 && seg_cnt == 0)) {
        AFS_TRACE_OUT_ERROR("LVM Metadata is error, The API Error Number is %d", errno);
        return -1;
    }

    return seg_cnt;
}

/**
 * @brief 解析lv
 * @param svname
 * @return
 * NULL
 * lv
 */
logicalVolume *afsLVM::parseMetadataLvInfoLv(string svname)
{
    // 找PV的pe_start字段
    string sid = lvmParseMetadataString("id");
    if (sid.empty()) {
        AFS_TRACE_OUT_ERROR("Can't parse meatadata");
        return NULL;
    }

    string sstatus = lvmParseMetadataString("status");
    if (sstatus.empty()) {
        AFS_TRACE_OUT_ERROR("Can't parse meatadata");
        return NULL;
    }

    // 找PV的pe_start字段
    int32_t seg_cnt = parseMetadataLvInfoSegCnt();
    if (-1 == seg_cnt) {
        AFS_TRACE_OUT_ERROR("Can't parse meatadata of segment count");
        return NULL;
    }

    // 创建空间
    logicalVolume *lv = m_vg->findLogicalVolume(sid);
    if (NULL == lv) {
        lv = m_vg->addLogicalVolume(sid, seg_cnt, svname);
        if (NULL == lv) {
            AFS_TRACE_OUT_ERROR("The Api of th new is failed");
            return NULL;
        }
    }

    // 是否是对外可见
    if (sstatus.npos != sstatus.find("VISIBLE")) {
        lv->m_is_visible = true;
    }

    return lv;
}

/**
 * @brief 解析lv
 * @return
 * NULL
 * lv_01_bracket
 */
string::size_type afsLVM::parseMetadataLvInfoHead()
{
    m_str_index = m_str_metadata.find("logical_volumes", 0);
    if (m_str_index == string::npos) {
        AFS_TRACE_OUT_INFO("Cannt read metadata\n");
        return string::npos;
    }

    // 3.2 先找第一个左大括号 ,logical_volumes {/0a/0a,有两个字节
    string::size_type lv_01_bracket = m_str_metadata.find("{", m_str_index);
    if (lv_01_bracket == string::npos) {
        AFS_TRACE_OUT_ERROR("LVM format is error\n");
        return string::npos;
    }

    return lv_01_bracket;
}

//  关于创建重复名字lv
//      [root@localhost dd]# dmsetup ls
//        vg_test-lvol1   (253:4)
//        vg_test-mythinpool-tpool        (253:2)
//        vg_test-mythinpool_tdata        (253:1)
//        vg_test-mythinpool_tmeta        (253:0)
//        vg_test-mythinpool      (253:3)
//        vg_test-lvol2   (253:5)
//        [root@localhost dd]# lvcreate -L 20M -n mythinpool_tmeta vg_test
//          WARNING: Not using lvmetad because duplicate PVs were found.
//          WARNING: Use multipath or vgimportclone to resolve duplicate PVs?
//          WARNING: After duplicates are resolved, run "pvscan --cache" to enable lvmetad.
//          Names including "_tmeta" are reserved. Please choose a different LV name.
//          Run `lvcreate --help' for more information.
//
//
//        [root@localhost dd]# lvcreate -L 20M -n mythinpool vg_test
//         WARNING: Not using lvmetad because duplicate PVs were found.
//         WARNING: Use multipath or vgimportclone to resolve duplicate PVs?
//         WARNING: After duplicates are resolved, run "pvscan --cache" to enable lvmetad.
//         Logical Volume "mythinpool" already exists in volume group "vg_test"
/**
 * @brief 解析逻辑卷
 * @return
 * AFS_ERR_LVM_PART：数据出错无法分析
 * AFS_ERR_API：系统调用出错
 * AFS_SUCCESS：成功
 */
int32_t afsLVM::parseMetadataLvInfo()
{
    int32_t ret = AFS_SUCCESS;

    // 3.2 先找第一个左大括号 ,logical_volumes {/0a/0a,有两个字节
    string::size_type lv_01_bracket = parseMetadataLvInfoHead();
    if (string::npos == lv_01_bracket) {
        AFS_TRACE_OUT_INFO("LVM format is error\n");
        return AFS_SUCCESS;
    }

    lv_01_bracket += 3;
    m_str_index = lv_01_bracket;

    string::size_type index = 0;
    string::size_type endPos = m_str_metadata.find("Generated by LVM");
    // 2.2解析lv所有的卷
    
    while ((index = m_str_metadata.find("{", m_str_index)) != m_str_metadata.npos) {
        if (endPos < index) {
            break;
        }
        m_str_index = index;
        // lv的名字
        // LV的名字,减一个空格
        string svname = m_str_metadata.substr(lv_01_bracket, (m_str_index - lv_01_bracket - 1));
        svname = "\"" + svname + "\"";

        // 创建空间
        logicalVolume *lv = parseMetadataLvInfoLv(svname);
        AFS_TRACE_OUT_DBG("afs find a lv[%s]", svname.c_str());
        if (NULL == lv) {
            AFS_TRACE_OUT_ERROR("The Api of th new is failed");
            return AFS_ERR_LVM_PART;
        }

        // lv关联卷组
        lv->m_this_group = m_vg;

        // 卷名
        lv->m_volname = svname;

        // 解析segment
        ret = parseLvSegment(lv);
        if (-1 == ret) {
            AFS_TRACE_OUT_ERROR("Cann't parse segment");
            return AFS_ERR_LVM_PART;
        }

        // 两个后括号
        m_str_index = m_str_metadata.find("}", m_str_index);
        if (m_str_index == string::npos) {
            AFS_TRACE_OUT_ERROR("LVM format is error");
            return AFS_ERR_LVM_PART;
        }

        // 有一个01
        m_str_index += 1;
        m_str_index = m_str_metadata.find("}", m_str_index);
        if (m_str_index == string::npos) {
            AFS_TRACE_OUT_ERROR("LVM format is error");
            return AFS_ERR_LVM_PART;
        }

        // 更新lv_01_bracket
        lv_01_bracket = m_str_index + 3;
    }

    // 初始化segment的资源
    return initSegmentResource();
}

/* * @brief 解析元数据
 *
 * @return
 * AFS_ERR_LVM_PART：数据出错无法分析
 * AFS_ERR_API：系统调用出错
 * AFS_SUCCESS：成功
 */
int32_t afsLVM::parseMetadata()
{
    int32_t ret = 0;
    m_str_index = 0;

    // 1.解析卷组  （卷组存在只分析pv更新分区地址偏移）
    ret = parseMetadataVgInfo();
    if (0 != ret) {
        if (ret == AFS_VOLUME_GROUP_EXIST) {
            AFS_TRACE_OUT_INFO("LVM: volume group exists");
        } else {
            AFS_TRACE_OUT_ERROR("Cann't Parse metadata of vg");
        }
        return ret;
    }

    AFS_TRACE_OUT_DBG("The extent size of Volme Group is %lld(sectors)", (long long)(m_vg->m_extent_size));

    // 2.解析pv
    ret = parseMetadataPvInfo();
    if (0 != ret) {
        AFS_TRACE_OUT_ERROR("Cann't Parse metadata of pv");
        return ret;
    }

    // 如果检测到跨分区这里不进行任何操作
    if (m_vg->m_span_part_flag) {
        AFS_TRACE_OUT_ERROR("m_vg->m_span_part_flag is true");
        return NOT_LVM_FORMAT;
    }

    ret = parseMetadataLvInfo();
    if (0 != ret) {
        AFS_TRACE_OUT_ERROR("Cann't Parse metadata of lv");
        return ret;
    }

    return ret;
}

/* * @brief 解析字符串
 *
 * @return
 * AFS_ERR_LVM_PART：数据出错无法分析
 * AFS_ERR_API：系统调用出错
 * AFS_SUCCESS：成功
 */
string afsLVM::lvmParseMetadataString(string str)
{
    int32_t swap_pos = 0;

    // 找到上一次到目前字符串的位置
    m_str_index = m_str_metadata.find(str, m_str_index);
    if (m_str_index == string::npos) {
        AFS_TRACE_OUT_INFO("Cannt found parse metadata\n");
        string null_str;
        null_str.clear();
        return null_str;
    }

    // 保存关键字字符串的位置
    swap_pos = m_str_index;

    // 查找特殊字符“0x0A”
    m_str_index = m_str_metadata.find(LVM_SPLITE_CHAR, m_str_index);
    if (m_str_index == string::npos) {
        AFS_TRACE_OUT_INFO("Cannt found parse metadata\n");
        string null_str;
        null_str.clear();
        return null_str;
    }

    // 值的头 = 关键字长度 + " = "(长度为3)
    swap_pos += str.length() + LVM_FIX_SPACE_FORMAT;

    // 得值长度
    int32_t value_len = m_str_index - swap_pos;

    return (m_str_metadata.substr(swap_pos, value_len));
}

/**
 * @brief 解析pv index
 * @return
 * -1
 * pv_index
 */
int32_t afsLVM::parseSegPVIndex()
{
    int32_t base = 10;

    m_str_index = m_str_metadata.find("pv", m_str_index);
    if (m_str_index == string::npos) {
        AFS_TRACE_OUT_INFO("Cannt found parse metadata\n");
        return AFS_ERR_LVM_PART;
    }

    string::size_type strlength = m_str_metadata.find("\"", m_str_index);
    if (strlength == string::npos) {
        AFS_TRACE_OUT_ERROR("Cannt found parse metadata\n");
        return AFS_ERR_LVM_PART;
    }

    string str = m_str_metadata.substr(m_str_index + 2, strlength - m_str_index - 2);

    initErrorNo();
    int32_t pv_index = strtol(str.c_str(), NULL, base);
    /* Check for various possible errors */
    if ((errno == ERANGE && (pv_index == static_cast<int32_t>(INT_MAX) || pv_index == static_cast<int32_t>(INT_MIN))) ||
        (errno != 0 && pv_index == 0)) {
        AFS_TRACE_OUT_ERROR("LVM Metadata is error, The API Error Number is %d", errno);
        return AFS_ERR_LVM_PART;
    }

    m_str_index = strlength + 2;

    return pv_index;
}

/**
 * @brief 解析start extent
 * @return
 * -1
 * stripe_start_extent
 */
uint32_t afsLVM::parseSegStartExtent()
{
    int32_t base = 10;

    // PV开始的extent
    string::size_type strlength = m_str_metadata.find(LVM_SPLITE_CHAR, m_str_index);
    if (string::npos == strlength) {
        AFS_TRACE_OUT_ERROR("LVM Metadata can not found.");
        return -1;
    }

    string str = m_str_metadata.substr(m_str_index + 1, strlength - m_str_index - 1);

    initErrorNo();
    uint32_t stripe_start_extent = strtol(str.c_str(), NULL, base);
    /* Check for various possible errors */
    if ((errno == ERANGE && stripe_start_extent == static_cast<uint32_t>(UINT_MAX)) ||
        (errno != 0 && stripe_start_extent == 0)) {
        AFS_TRACE_OUT_ERROR("LVM Metadata is error, The API Error Number is %d", errno);
        return -1;
    }

    m_str_index = strlength;

    return stripe_start_extent;
}

/**
 * @brief 解析type = "thin-pool"类型
 * @return
 * 0 ： error
 * chunk_size
 */
uint32_t afsLVM::parseThinPoolSegChunkSize()
{
    int32_t base = 10;

    string sChunk_size = lvmParseMetadataString("chunk_size");
    if (sChunk_size.empty()) {
        AFS_TRACE_OUT_ERROR("Can't parse Thin Pool Seg Chunk Size");
        return 0;
    }

    initErrorNo();
    uint32_t chunk_size = strtoul(sChunk_size.c_str(), NULL, base);
    /* Check for various possible errors */
    if ((errno == ERANGE && chunk_size == static_cast<uint32_t>(UINT_MAX)) || (errno != 0 && chunk_size == 0)) {
        AFS_TRACE_OUT_ERROR("LVM Thin Pool Seg Chunk Size is error, The API Error Number is %d", errno);
        return 0;
    }

    return chunk_size;
}

/**
 * @brief 解析StripeCnt
 * @return
 * -1
 * stripe_count
 */
uint32_t afsLVM::parseSegStripeCnt()
{
    int32_t base = 10;

    string sstripe_count = lvmParseMetadataString("stripe_count");
    if (sstripe_count.empty()) {
        AFS_TRACE_OUT_ERROR("Can't parse meatadata");
        return 0;
    }

    initErrorNo();
    uint32_t stripe_count = strtoul(sstripe_count.c_str(), NULL, base);
    /* Check for various possible errors */
    if ((errno == ERANGE && stripe_count == static_cast<uint32_t>(UINT_MAX)) || (errno != 0 && stripe_count == 0)) {
        AFS_TRACE_OUT_ERROR("LVM Metadata is error, The API Error Number is %d", errno);
        return 0;
    }

    return stripe_count;
}

/**
 * @brief 解析thin_pool
 * @return
 */
string afsLVM::parseSegThinPool()
{
    return lvmParseMetadataString("thin_pool");
}

/* * @brief 解析thin
 *
 * @return
 * AFS_ERR_LVM_PART：数据出错无法分析
 * AFS_ERR_API：系统调用出错
 * AFS_SUCCESS：成功
 */
int32_t afsLVM::parseSegDevId()
{
    // device-id 处理
    string sdevice_id = lvmParseMetadataString("device_id");
    if (sdevice_id.empty()) {
        AFS_TRACE_OUT_ERROR("Can't parse meatadata");
        return -1;
    }

    initErrorNo();

    int32_t base = 10;
    int32_t device_id = strtol(sdevice_id.c_str(), NULL, base);
    /* Check for various possible errors */
    if ((errno == ERANGE &&
        (device_id == static_cast<int32_t>(INT_MAX) || device_id == static_cast<int32_t>(INT_MIN))) ||
        (errno != 0 && device_id == 0)) {
        AFS_TRACE_OUT_ERROR("LVM Metadata is error, The API Error Number is %d", errno);
        return -1;
    }

    return device_id;
}

/**
 * @brief 解析MetaData
 *
 * @return
 */
string afsLVM::parseSegMetaData()
{
    return lvmParseMetadataString("metadata");
}

/**
 * @brief 解析SegPool
 * @return
 */
string afsLVM::parseSegPool()
{
    return lvmParseMetadataString("pool");
}

/**
 * @brief   表示该分区为PV格式，非单独的物理分区
 * @return
 */
void afsLVM::setPartPVType()
{
    vector<struct partition>::iterator iter = m_part_vect.end() - 1;
    (*iter).is_pv_part = true; // 表示该分区为PV分区
    AFS_TRACE_OUT_DBG("the partition[%u] belongs to pv type", m_part_vect.size() - 1);
}
