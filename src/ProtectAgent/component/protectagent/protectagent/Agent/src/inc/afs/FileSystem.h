/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 *
 * @file FileSystem.h
 * @brief Afs - Analyze file system interface.
 *
 */

#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include "afs/Afslibmain.h"
#include "afs/AfsObject.h"
#include "afs/ImgReader.h"
#include "afs/Bitmap.h"

#define SWAP_MAGIC "SWAPSPACE2"

typedef union _AFS_LARGEINTEGER {
    struct {
        uint32_t LowPart;
        int32_t HighPart;
    } u;
    int64_t QuadPart;
} AFS_LARGEINTEGER;

/**
 * @brief 处理文件系统的类
 */
class filesystemHandler : public afsObject {
public:
    filesystemHandler();
    ~filesystemHandler();

    // 文件系统操作句柄
    imgReader *m_reader;

    int32_t initRealHandler();
    void setImageReader(imgReader *reader);

    void setType(int32_t type);
    int32_t getType();
    int32_t getFSTypeUUID(int32_t *fs_type, char *fs_uuid, size_t fs_uuid_len);

    // 读取文件系统块信息到缓存
    uint64_t readFSBlock(char *buff, uint64_t blkNum, uint64_t blk_unit);
    uint64_t readFSData(char *buff, uint64_t start_byte, uint64_t size_byte, uint64_t blk_unit);

    uint64_t setFSBitMapData(char *buff, uint64_t start_byte, uint64_t size_byte);
    uint64_t setFSBitMapBlock(char *buff, uint64_t sect_no);

    // 获取BitMap
    virtual int32_t getBitmap(vector<BitMap *> &bitmap_vect);
    virtual int32_t getFile(const char *file_path, vector<BitMap *> &bitmap_vect);

    // 通过vddk要按照扇区的整数倍读取数据，解决读取数据不正确的问题
    bool AllocBySectors(std::unique_ptr<char[]> &buffer, int64_t length, int64_t &allocSize);
    bool ReadBySectorsBuff(imgReader *reader, void *buff, int64_t offset, int64_t length, int32_t annotated);

protected:
    imgReader *getImgReader()
    {
        return m_reader;
    }

private:
    // 获得文件系统类型
    int32_t m_fs_type;

    uint32_t convert_bswap_32(uint32_t x);
    int32_t getFSTypeUUID_1(unsigned char *buf, char *fs_uuid, size_t fs_uuid_len);
};

#endif
