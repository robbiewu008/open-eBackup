/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 *
 * @file PartitionHandler.h
 * @brief AFS - Partition handler class.
 *
 */

#ifndef PARTITIONHANDLER_H
#define PARTITIONHANDLER_H

#include "afs/PartReaderFactory.h"
#include "afs/LVMVolumeGroupManager.h"
#include "afs/LogicalVolume.h"

/**
 * @brief 文件系统类型
 */
typedef enum AFS_PARTITION {
    PARTITION_UNKNOWN = 0, // 不能正常识别的分区类型
    PARTITION_MBR,         // MBR分区
    PARTITION_GPT,         // GPT分区
    PARTITION_NO           // 整个磁盘作为一个分区
} AFS_PARTITION_t;

/**
 * @brief PV没有VG METADATA的情况
 */
struct pv_no_vg_metadata {
    uint64_t pv_offset; // PV 相对于磁盘的偏移
    int32_t pv_disk_id;
    char pv_uuid[UUID_LEN + 1];
};


/**
 * @brief 处理分区的类
 */
class partitionHandler : public afsObject {
public:
    partitionHandler();
    ~partitionHandler();

    // raw等镜像的全局Reader
    imgReader *m_reader;
    vector<struct partition> m_parts_vect;
    vector<struct partition> &getPartSpaceVect()
    {
        return m_parts_vect;
    }
    int32_t getPartsNumBeforeHandler(partitionHandler *real_handler);

    // 底层连续块大小(单位：扇区)
    map<int32_t, uint32_t> m_map_chunk_size;

    list<struct pv_no_vg_metadata> m_pv_no_vg_metadata;
    list<struct pv_no_vg_metadata> &getPvNoMetadataList()
    {
        return m_pv_no_vg_metadata;
    }

    // use rawReader
    vector<imgReader *> m_vect_imgreader;
    vector<imgReader *> &getImgReaderVector()
    {
        return m_vect_imgreader;
    }

    int32_t copyImgReaderVectToLVs();
    int32_t verifyThinPoolSuperBlock();
    int32_t updateThinLVPartInfo();
    int32_t updateLVWithNoMetadataPV();
    int32_t updateNoMetadataPV();

    int32_t updateAllPartitions();

    vector<partitionHandler *> m_vector_realHandler;
    vector<partitionHandler *> &getRealHandlerVector()
    {
        return m_vector_realHandler;
    }

    void pushImgReader(imgReader *imgreader)
    {
        m_vect_imgreader.push_back(imgreader);
    }
    void setPartBaseNumValue(int32_t partsBase)
    {
        m_partBase = partsBase;
    }
    int32_t getPartBaseNumValue()
    {
        return m_partBase;
    }

    // 卷组管理器
    LVMVolumeGroupManager m_vgManager;

    void *getPartitionPointer(int32_t index);

    void setImgReader(imgReader *imgreader)
    {
        this->m_reader = imgreader;
    }
    int32_t getPartType();
    int32_t analyzePartitions();
    virtual int32_t parseAllOfPart()
    {
        return 0;
    }

    imgReader *getImgReader()
    {
        return this->m_reader;
    }
    imgReader *getRealPartReader()
    {
        return this->m_real_part_reader;
    }
    int32_t getType()
    {
        return m_type;
    }

    int32_t getPartition(int32_t index, struct partition *ppart);
    int32_t setPartition(int32_t index, struct partition *ppart);

    int32_t createPartitionReader(struct partition *ppart);

    int32_t setPartnum(int32_t partnum);
    int32_t getPartnum();

    void setPartnumValue(int32_t partnum);

    int32_t getDiskNum();
    void setDiskNumValue(int32_t diskNum);
    void copyRealHandlerInfo();
    void copyFatherHandlerInfo();

    partitionHandler *getrealHandler();

    int32_t getDisksBitmap(vector<BitMap *> &bitmap_vect);
    virtual int32_t getBitmap(BitMap &bitmap);

    // 通过vddk要按照扇区的整数倍读取数据，解决读取数据不正确的问题
    bool AllocBySectors(std::unique_ptr<char[]> &buffer, int64_t length, int64_t &allocSize);
    bool ReadBySectorsBuff(imgReader *reader, void *buff, int64_t offset, int64_t length, int32_t annotated);

protected:
    void settype(int32_t t)
    {
        m_type = t;
    };

private:
    partitionHandler(const partitionHandler &partHandler);
    partitionHandler &operator = (const partitionHandler &partHandler);

    int32_t m_type;

    int32_t m_partnum;  // 分区个数
    int32_t m_partBase; // /实例分区起始基数

    int32_t m_disknum; // /磁盘个数

    partitionHandler *m_realhandler;

    // 每个分区的reader(分区会设置自己分区偏移的reader)
    imgReader *m_real_part_reader;
    // 分区reader
    partReaderFactory m_createPartReader;

    partitionHandler *createPartHandler();
    int32_t getOtherManagerModeBitmap(vector<BitMap *> &bitmap_vect);
};

#endif
