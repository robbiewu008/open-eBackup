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
#ifndef CORE_LVM_H_
#define CORE_LVM_H_
#include <cerrno>
#include "afs/AfsObject.h"
#include "afs/PartitionHandler.h"
#include "afs/Segment.h"
#include "afs/VolumeGroup.h"

#define MAX_PART 127

// lvm签名
#define LVM_SIGNATURE "LABELONE"
#define LVM_2_VERSION_MAGIC "LVM2 001"
// 判断lvm的扇区数
#define LVM_LABEL_SECTOR_RANGE 4
#define NOT_LVM_FORMAT (-65536)
#define LVM_SPLITE_CHAR 0x0A

/**
 * @brief lv信息
 */
typedef struct pv_metadata_info {
    // 每个lv的偏移
    uint64_t offset[MAX_PART];
    // 每个lv元数据的长度
    uint32_t meta_data_len[MAX_PART];
} pv_metadata_info_t;

/**
 * @brief LVM: LVM功能类
 */
class afsLVM : public afsObject {
public:
    afsLVM(partitionHandler *handler, vector<struct partition> &part_vect, int32_t part_index);
    virtual ~afsLVM();

    int32_t parseSegPVIndex();
    uint32_t parseSegStartExtent();
    uint32_t parseSegStripeCnt();
    uint32_t parseSegStripeSize();
    string parseSegThinPool();
    int32_t parseSegDevId();
    string parseSegPool();
    string parseSegMetaData();
    uint32_t parseThinPoolSegChunkSize();

    // 根据镜像句柄，设置分区信息
    int32_t parseLVMFormat(int32_t real_part_num);

    // 卷组信息
    volumeGroup *m_vg;

#ifdef CPPUNIT_MAIN
protected:
#else
private:
#endif
    afsLVM(const afsLVM &obj);
    afsLVM &operator = (const afsLVM &obj);

    // 解析LVM信息头
    int32_t parseLvmInfo();
    // 解析元数据
    int32_t parseMetadata();
    // 设置分区信息
    int32_t setPartInfo();

    int32_t lvmLabelHeadHandle(PV_LABEL_HEADER *header);

    int32_t parseMetadataVgInfo();
    int32_t parseMetadataPvInfo();
    int32_t parseMetadataLvInfo();
    string::size_type parseMetadataLvInfoHead();

    int32_t parseLvSegment(logicalVolume *lv);
    int32_t parseLvSegmentSingle(logicalVolume *lv, char *sseg_num);
    void *segmentTypeHandle(SEG_TYPE_ENU type);

    int32_t forEachAllOfLV();
    int32_t saveNoMetadataPv();

    int32_t initSegmentResource();

    int32_t String2UUID(string origin_uuid, char *target_uuid, int32_t target_uuid_len);
    int32_t UpdateLVPartitionInfo(int32_t new_lvm_num, uint64_t disk_offset, int32_t disk_id);
    int32_t UpdatePhysicalPartition(string pv_uuid_string, uint64_t pv_offset);
    int32_t getPhysicalPartitionInfo();

    int32_t parseMetadataVgInfoSeqno();
    uint32_t parseMetadataVgInfoExtentSize();

    uint64_t parseMetadataPvInfoDevSize();
    uint32_t parseMetadataPvInfoPeStart();
    uint32_t parseMetadataPvInfoPeCount();
    int32_t parseMetadataLvInfoSegCnt();
    int32_t pvolume(int32_t c_num);
    logicalVolume *parseMetadataLvInfoLv(string svname);
    int32_t forEachAllOfLVDoSeg(logicalVolume *plv, struct partition &temp_part, uint64_t &lv_length);

    // 解析特定字符串长度
    string lvmParseMetadataString(string str);

    uint32_t parseLvSegmentStartExtent();
    uint32_t parseLvSegmentExtentCount();

    void setPartPVType();

    void initErrorNo()
    {
        errno = 0;
    }

    char uuid[UUID_LEN + 1];
    int32_t m_length;

    // lvm所在物理分区偏移
    uint64_t m_pv_offset;

    // 类组合
    imgReader *m_ptr_reader;
    partitionHandler *m_ptr_handler;

    int32_t m_real_part_index;
    int32_t m_part_index;
    vector<struct partition> &m_part_vect;
    struct partition m_physical_part;

    string::size_type m_str_index;
    string m_str_metadata;

    int m_flag;
};

#endif /* CORE_LVM_H_ */
