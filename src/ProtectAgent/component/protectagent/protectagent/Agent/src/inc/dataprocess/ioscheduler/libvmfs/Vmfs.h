#ifndef __VMFS5_H__
#define __VMFS5_H__

#include <cstdlib>
#include <cerrno>
#include <cstdint>
#include <memory>
#include <uuid/uuid.h>
#include <functional>
#include <securec.h>

namespace Vmfs5IO {
union VmfsFlagsS {
    int packed;
    struct {
        unsigned int debugLevel : 4;
        unsigned int readWrite : 1;
        unsigned int allowMissingExtents : 1;
    };
} __attribute__((transparent_union));
using VmfsFlagsT = union VmfsFlagsS;

/* VMFS types - forward declarations */
using VmfsVolinfoT = struct VmfsVolinfoS;
using VmfsFsInfoT = struct VmfsFsInfoS;
using VmfsLvmInfoT = struct VmfsLvmInfoS;
using VmfsHeartbeatT = struct VmfsHeartbeatS;
using VmfsMetadataHdrT = struct VmfsMetadataHdrS;
using VmfsBitmapHeaderT = struct VmfsBitmapHeaderS;
using VmfsBitmapEntryT = struct VmfsBitmapEntryS;
using VmfsBitmapT = struct VmfsBitmapS;
using VmfsInodeT = struct VmfsInodeS;
using VmfsDirentT = struct VmfsDirentS;
using VmfsDirT = struct VmfsDirS;
using VmfsFileT = struct VmfsFileS;
using VmfsDeviceT = struct VmfsDeviceS;
using VmfsVolumeT = struct VmfsVolumeS;
using VmfsLvmT = struct VmfsLvmS;
using VmfsFsT = struct VmfsFsS;
}

#include "common/Log.h"
#include "Utils.h"
#include "SCSI.h"
#include "VmfsHeartbeat.h"
#include "VmfsMetadata.h"
#include "VmfsBlock.h"
#include "VmfsBitmap.h"
#include "VmfsInode.h"
#include "VmfsFile.h"
#include "VmfsDirent.h"
#include "VmfsDevice.h"
#include "VmfsVolume.h"
#include "VmfsLvm.h"
#include "VmfsFs.h"

namespace Vmfs5IO {
#define VMFS5_ROOT_INO (1)
using VmfsInoT = uint32_t;
inline uint32_t ino2blkid(VmfsInoT ino)
{
    if (ino == VMFS5_ROOT_INO) {
        return (VMFS5_BLK_FD_BUILD(0, 0, 0));
    }
    return ((uint32_t)ino);
}

inline VmfsInoT blkid2ino(uint32_t blkId)
{
    if (blkId == VMFS5_BLK_FD_BUILD(0, 0, 0))
        return (VMFS5_ROOT_INO);
    return ((VmfsInoT)blkId);
}
}

#endif
