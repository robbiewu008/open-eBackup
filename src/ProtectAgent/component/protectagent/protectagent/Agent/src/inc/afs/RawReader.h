#ifndef RAWREADER_H
#define RAWREADER_H

#include "afs/FSCommon.h"
#include "afs/Afslibrary.h"
#include "afs/ImgReader.h"

/**
 * @brief raw格式的image文件的reader类
 */
class rawReader : public imgReader {
public:
    rawReader()
    {
        setObjType(OBJ_TYPE_IMGREADER);
        setMagic("raw");
        memset_s(&m_imageinfo, sizeof(m_imageinfo), 0, sizeof(m_imageinfo));
        m_imageinfo.formattype = IMG_FORMAT_TYPE_RAW;
    }

    ~rawReader() {}

    virtual void setImgInfoDiskID(int32_t diskId)
    {
        m_imageinfo.disk_id = diskId;
    }

    virtual int32_t getImgInfoDiskID()
    {
        return m_imageinfo.disk_id;
    }

    virtual void initImgReader(struct imgInfo *pImageInfo, int64_t offset, int64_t length);

    // 裸读（例如：lvm虚拟读，这里是真正的读）
    virtual int64_t readDisk(void *buf, int64_t offset, int64_t count);
    // 文件系统读，raw格式和裸读一样
    virtual int64_t read(void *buf, int64_t offset, int64_t count, int32_t is_annotated);

    static afsObject *CreateObject()
    {
        return new rawReader();
    }

    virtual int64_t getVaddrToPaddr(int64_t vaddr, int32_t &disk_id)
    {
        // 目前需要装换的只有lvm模式
        disk_id = getImgInfoDiskID();
        return (vaddr / SECTOR_SIZE);
    }
};

#endif // RAWREADER_H
