#include <cstring>
#include <sys/stat.h>
#include "dataprocess/ioscheduler/libvmfs6/Vmfs.h"


namespace Vmfs6IO {
VmfsMetadata *VmfsMetadata::m_instance = nullptr;
std::mutex VmfsMetadata::m_mutex;

VmfsMetadata *VmfsMetadata::Instance()
{
    if (m_instance == nullptr) {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_instance == nullptr) {
            m_instance = new (std::nothrow) VmfsMetadata();
        }
    }
    return m_instance;
}

/* Read a metadata header */
int VmfsMetadata::HdrRead(VmfsMetadataHdrT *mdh, const u_char *buf)
{
    mdh->magic = ReadLE32(buf, VMFS6_MDH_OFFSET_MAGIC);
    mdh->pos = ReadLE64(buf, VMFS6_MDH_OFFSET_POS);
    mdh->hbPos = ReadLE64(buf, VMFS6_MDH_OFFSET_HB_POS);
    mdh->hbSeq = ReadLE64(buf, VMFS6_MDH_OFFSET_HB_SEQ);
    mdh->objSeq = ReadLE64(buf, VMFS6_MDH_OFFSET_OBJ_SEQ);
    mdh->hbLock = ReadLE32(buf, VMFS6_MDH_OFFSET_HB_LOCK);
    mdh->mtime = ReadLE64(buf, VMFS6_MDH_OFFSET_MTIME);
    ReadUuid(buf, VMFS6_MDH_OFFSET_HB_UUID, &mdh->hbUuid);
    return (0);
}

/* Write a metadata header */
int VmfsMetadata::HdrWrite(const VmfsMetadataHdrT *mdh, u_char *buf)
{
    int ret = memset_s(buf, VMFS6_METADATA_HDR_SIZE, 0, VMFS6_METADATA_HDR_SIZE);
    if (ret != 0) {
        return (-1);
    }
    WriteLE32(buf, VMFS6_MDH_OFFSET_MAGIC, mdh->magic);
    WriteLE64(buf, VMFS6_MDH_OFFSET_POS, mdh->pos);
    WriteLE64(buf, VMFS6_MDH_OFFSET_HB_POS, mdh->hbPos);
    WriteLE64(buf, VMFS6_MDH_OFFSET_HB_SEQ, mdh->hbSeq);
    WriteLE64(buf, VMFS6_MDH_OFFSET_OBJ_SEQ, mdh->objSeq);
    WriteLE32(buf, VMFS6_MDH_OFFSET_HB_LOCK, mdh->hbLock);
    WriteLE64(buf, VMFS6_MDH_OFFSET_MTIME, mdh->mtime);
    WriteUuid(buf, VMFS6_MDH_OFFSET_HB_UUID, &mdh->hbUuid);
    return (0);
}
}
