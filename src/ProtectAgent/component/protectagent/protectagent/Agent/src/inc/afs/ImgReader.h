#ifndef IMGREADER_H
#define IMGREADER_H

#include <unistd.h>
#include "afs/AfsObject.h"
#include "afs/AfsType.h"
#include "afs/Afslibrary.h"

#define IMG_FORMAT_TYPE_UNKNOW 0
#define IMG_FORMAT_TYPE_RAW 1

/**
 * @brief 镜像信息
 */
struct imageInfo {
    int32_t formattype;
    int64_t offset;
    int64_t length;
    uint32_t chunkSize;
    int32_t disk_id; // 表示image所属的磁盘ID
};

/**
 * @brief image文件的reader类
 */
class imgReader : public afsObject {
public:
    imgReader() : afsObject()
    {
        m_lowreader = NULL;
        setObjType(OBJ_TYPE_IMGREADER);
        setMagic("unknown");
        m_handle = NULL;
        m_read_call_back_func = NULL;
        memset_s(&m_imageinfo, sizeof(m_imageinfo), 0, sizeof(m_imageinfo));
    }

    imgReader(imgReader *low_lovel_reader)
    {
        m_lowreader = static_cast<afsObject *>(low_lovel_reader);
        setObjType(OBJ_TYPE_IMGREADER);
        setMagic("unknown");
        m_handle = NULL;
        m_read_call_back_func = NULL;
        memset_s(&m_imageinfo, sizeof(m_imageinfo), 0, sizeof(m_imageinfo));
    }

    virtual ~imgReader()
    {
        m_lowreader = NULL;
        m_handle = NULL;
    }

    virtual void setInfo(void *info) {}

    // 调整API后的接口
    virtual void initImgReader(struct imgInfo *pImageInfo, int64_t offset, int64_t length) {}

    int32_t initImgReader(afsObject *reader, struct partition *ppart, uint32_t chunk_size);
    virtual void setImgInfoDiskID(int32_t diskId) {}
    virtual int32_t getImgInfoDiskID()
    {
        return -1;
    }

    // 有文件系统的读（有可能需要地址的转换）
    virtual int64_t read(void *buf, int64_t offset, int64_t count, int32_t is_annotated);
    // 读：单位为扇区
    virtual int64_t readSector(void *buf, int64_t sect_off, int64_t count);

    // 虚拟块号获取物理块号
    virtual int64_t getVaddrToPaddr(int64_t vaddr, int32_t &disk_id);

    uint32_t getChunkSize()
    {
        return m_imageinfo.chunkSize;
    }

    int32_t GetReaderInfo(struct imageInfo *imginfo);

    virtual imgReader *getlowReader();

    virtual uint32_t getStripeSize();

    uint32_t getMaxStorageZone();

    int32_t setImgOffset(int64_t offset, int64_t length);

    struct imageInfo m_imageinfo;
    afsObject *m_lowreader;
    AFS_HANDLE m_handle;
    AFS_READ_CALLBACK_FUNC_t m_read_call_back_func;
};

#endif
