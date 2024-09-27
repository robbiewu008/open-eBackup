#include "dataprocess/vmwarenative/VMwareDiskApi.h"

#include <vector>
#include "dataprocess/vmwarenative/VddkDeadlockCheck.h"
#include "common/Log.h"
#include "common/Utils.h"
#include "common/JsonUtils.h"

namespace {
    const mp_int32 MAX_CHUNK_NUMBER_DEFAULT = 512 * 1024;
    const uint64 ALLOCATED_BLOCK_CHUNKSIZE_DEFAULT = 1024;
}

VMwareDiskApi::VMwareDiskApi(
    const VMwareDiskOperations &operations, const VixDiskLibConnection &connection, MessageLoop * const messageLoop)
    : m_vddkOperations(operations),
      m_connection(connection),
      m_diskHandle(NULL),
      m_diskSize(0),
      m_diskCapacity(0),
      m_chunkSize(0),
      m_diskOpenMode(1),
      m_messageLoop(messageLoop)
{}

VMwareDiskApi::~VMwareDiskApi()
{}

VMWARE_DISK_RET_CODE VMwareDiskApi::OpenDisk(const std::string &path, const std::string &snapshotRef,
    const uint32_t flags, const uint64_t chunkSize, const std::string &transportModes,
    std::string &selectedtransportMode, std::string &errDesc)
{
    VddkDeadlockCheck::DeadlockCheck check("OpenDisk");
    if (m_connection == NULL) {
        COMMLOG(OS_LOG_ERROR, "The connection with remote product env is not established, please check!");
        return VIX_E_CANNOT_CONNECT_TO_HOST;
    }

    m_chunkSize = 0;
    m_path = path;
    m_snapshotRef = snapshotRef;
    m_transportModes = transportModes;

    // Open target disk
    std::promise<VMWARE_DISK_RET_CODE> promise;
    std::future<VMWARE_DISK_RET_CODE> future = promise.get_future();
    this->Open(path, flags, std::ref(promise));
    future.wait();
    VMWARE_DISK_RET_CODE code = future.get();
    if (code != VIX_OK) {
        errDesc = GetErrString(code);
        COMMLOG(OS_LOG_ERROR, "Open Disk '%s' failed: '%s'.", path.c_str(), GetErrString(code).c_str());
        return code;
    }

    // Query the disk detail info
    VixDiskLibInfo *diskInfo = NULL;
    code = m_vddkOperations.vixDiskLibGetInfo(m_diskHandle, &diskInfo);
    if (code != VIX_OK || diskInfo == NULL) {
        errDesc = GetErrString(code);
        COMMLOG(OS_LOG_ERROR, "Get Disk '%s' infomation failed: '%s'.", path.c_str(), GetErrString(code).c_str());
        std::string str;
        (void)CloseDisk(str);
        return VIX_E_FAIL;
    }

    m_diskCapacity = diskInfo->capacity;
    m_diskSize = diskInfo->capacity * SECTOR_SIZE;
    m_vddkOperations.vixDiskLibFreeInfo(diskInfo);
    diskInfo = NULL;

    // Get the disk transport mode selected
    selectedtransportMode = GetTransportMode();
    if (std::string::npos != selectedtransportMode.find("nbd")) {
        m_chunkSize = chunkSize;
    }

    m_diskOpenMode = flags;

    COMMLOG(OS_LOG_INFO,
        "Open Disk '%s' successfully, disk size '%llu', chunk size '%llu', transport mode '%s'.",
        path.c_str(),
        m_diskSize,
        m_chunkSize,
        selectedtransportMode.c_str());

    return VIX_OK;
}

VMWARE_DISK_RET_CODE VMwareDiskApi::OpenDiskByLocalDevice(const std::string &path, const mp_uint32 &openMode,
    const std::string &diskType, const mp_uint64 &diskSize)
{
    m_path = path;
    m_diskOpenMode = openMode;
    m_devFileHandle = open(path.c_str(), openMode);
    m_diskType = diskType;
    m_diskSize = diskSize;
    if (m_devFileHandle == -1) {
        COMMLOG(OS_LOG_ERROR, "Open Disk '%s' failed: '%s'.", path.c_str(), GetErrString(errno).c_str());
        return VIX_E_FAIL;
    }
    COMMLOG(OS_LOG_INFO,
        "Open Disk '%s' successfully, OpenMode:%d.",
        path.c_str(), openMode);

    return VIX_OK;
}


VMWARE_DISK_RET_CODE VMwareDiskApi::CloseDiskByLocalDevice(const std::string &path)
{
    if (m_devFileHandle != -1) {
        COMMLOG(OS_LOG_INFO, "close Disk");
        close(m_devFileHandle);
    }
    return VIX_OK;
}


EXTER_ATTACK VMWARE_DISK_RET_CODE VMwareDiskApi::QueryAllocatedBlocks(Json::Value &bodyMsg)
{
    if (m_vddkOperations.vixDiskLibQueryAllocateBlocks == NULL) {
        INFOLOG("Function vixDiskLibQueryAllocateBlocks is NULL.");
        return VIX_E_FAIL;
    }

    mp_int32 maxChunkNumber;
    mp_uint64 chunkSize;
    GetAllocatedBlocksParams(maxChunkNumber, chunkSize);

    std::vector<VixDiskLibBlock> vixBlocks;
    VixError vixError;
    mp_uint64 capacity = m_diskCapacity;
    mp_uint64 offset = 0;
    mp_uint64 numChunk = capacity / chunkSize;

    while (numChunk > 0) {
        VixDiskLibBlockList *blockList = NULL;
        mp_uint64 numChunkToQuery = (numChunk > maxChunkNumber) ? (maxChunkNumber) : (numChunk);

        DBGLOG("Call vixDiskLibQueryAllocateBlocks, offset=%llu, numChunkToQuery=%d, ichunkSize=%d.",
            offset, numChunkToQuery, chunkSize);
        vixError = m_vddkOperations.vixDiskLibQueryAllocateBlocks(m_diskHandle, offset,
            numChunkToQuery * chunkSize, chunkSize, &blockList);
        if (vixError != VIX_OK) {
            ERRLOG("Call vixDiskLibQueryAllocateBlocks failed.");
            if (blockList != NULL) {
                m_vddkOperations.vixDiskLib_FreeBlockList(blockList);
            }
            return vixError;
        }
        DBGLOG("Call vixDiskLibQueryAllocateBlocks success.");
        if (blockList != NULL) {
            for (uint32 i = 0; i < blockList->numBlocks; i++) {
                vixBlocks.push_back(blockList->blocks[i]);
            }
        }

        numChunk -= numChunkToQuery;
        offset += numChunkToQuery * chunkSize;
        if (blockList != NULL) {
            m_vddkOperations.vixDiskLib_FreeBlockList(blockList);
        }
    }

    uint64 unalignedPart = capacity % chunkSize;
    if (unalignedPart > 0) {
        VixDiskLibBlock block;
        block.offset = offset;
        block.length = unalignedPart;
        vixBlocks.push_back(block);
    }
    GetAllocatedBlocks(vixBlocks, bodyMsg);

    return VIX_OK;
}

void VMwareDiskApi::GetAllocatedBlocks(const std::vector<VixDiskLibBlock> &vixBlocks, Json::Value &bodyMsg)
{
    COMMLOG(OS_LOG_INFO, "Number of blocks: %d", vixBlocks.size());

    uint64 allocatedSize = 0;
    Json::Value allocatedBlocks;
    Json::Value allocatedBlock;
    for (uint32 i = 0; i < vixBlocks.size(); i++) {
        INFOLOG("vixBlocks[i].offset = %d, vixBlocks[i].length = %d", vixBlocks[i].offset, vixBlocks[i].length);
        allocatedSize += vixBlocks[i].length;
        allocatedBlock["length"] = vixBlocks[i].length * SECTOR_SIZE;
        allocatedBlock["start"] = vixBlocks[i].offset * SECTOR_SIZE;
        allocatedBlocks.append(std::move(allocatedBlock));
    }

    bodyMsg["body"].clear();
    bodyMsg["body"]["AllocatedBlocks"] = std::move(allocatedBlocks);
    COMMLOG(OS_LOG_INFO, "allocated size (%d) / capacity (%d)", allocatedSize, m_diskCapacity);
}

void VMwareDiskApi::GetAllocatedBlocksParams(mp_int32 &max_chunk_number, mp_uint64 &chunkSize)
{
    mp_int32 iRet = CConfigXmlParser::GetInstance().GetValueInt32(CFG_BACKUP_SECTION,
        CFG_MAX_CHUNK_NUMBER, max_chunk_number);
    if (iRet != MP_SUCCESS) {
        ERRLOG("Get max_chunk_number failed, use default value (%d).", MAX_CHUNK_NUMBER_DEFAULT);
        max_chunk_number = MAX_CHUNK_NUMBER_DEFAULT;
    }

    iRet = CConfigXmlParser::GetInstance().GetValueUint64(CFG_BACKUP_SECTION,
        CFG_ALLOCATED_BLOCK_CHUNKSIZE, chunkSize);
    if (iRet != MP_SUCCESS || chunkSize == 0) {
        ERRLOG("Get allocated_block_chunksize failed, use default value (%lld).", ALLOCATED_BLOCK_CHUNKSIZE_DEFAULT);
        chunkSize = ALLOCATED_BLOCK_CHUNKSIZE_DEFAULT;
    }

    DBGLOG("max_chunk_number = %d, chunkSize = %lld.", max_chunk_number, chunkSize);
}

VMWARE_DISK_RET_CODE VMwareDiskApi::CloseDisk(std::string &errDesc)
{
    VddkDeadlockCheck::DeadlockCheck check("CloseDisk");
    VMWARE_DISK_RET_CODE ret = VIX_OK;

    std::promise<VMWARE_DISK_RET_CODE> promise;
    std::future<VMWARE_DISK_RET_CODE> future = promise.get_future();
    m_messageLoop->PostTask(std::bind(&VMwareDiskApi::Close, this, std::ref(promise)));
    future.wait();
    VMWARE_DISK_RET_CODE code = future.get();
    if (VIX_OK != code) {
        COMMLOG(OS_LOG_ERROR, "Close Disk failed: '%s'.", GetErrString(code).c_str());
        ret = code;
    }

    COMMLOG(OS_LOG_INFO, "Close disk completed.");
    return ret;
}

// Based on the connection channel, a disk handle will be obtained
void VMwareDiskApi::Open(const std::string &path, const uint32_t flags, std::promise<VMWARE_DISK_RET_CODE> &promise)
{
    VixError vixError = m_vddkOperations.vixDiskLibOpen(m_connection, path.c_str(), flags, &m_diskHandle);
    if (VIX_OK != vixError) {
        COMMLOG(
            OS_LOG_ERROR, "Unable to open disk file '%s', error: '%s'.", path.c_str(), GetErrString(vixError).c_str());
        m_diskHandle = NULL;
    }
    promise.set_value(vixError);
}

VMWARE_DISK_RET_CODE VMwareDiskApi::Close(std::promise<VMWARE_DISK_RET_CODE> &promise)
{
    auto ret = VIX_OK;
    if (m_devFileHandle != -1) {
        COMMLOG(OS_LOG_INFO, "close rdm Disk");
        close(m_devFileHandle);
        m_devFileHandle = -1;
    }
    if (m_diskHandle == NULL) {
        COMMLOG(OS_LOG_INFO, "The disk handler is NULL!");
        promise.set_value(VIX_OK);
        return ret;
    }

    VixError vixError = m_vddkOperations.vixDiskLibClose(m_diskHandle);
    m_diskHandle = NULL;
    m_diskSize = 0;
    promise.set_value(vixError);
    return ret;
}

VMWARE_DISK_RET_CODE VMwareDiskApi::RetryOp(std::function<VMWARE_DISK_RET_CODE()> internalOp)
{
    int retryTimes = -1;
    if (retryTimes < 0) {
        retryTimes = DEFAULT_RETRY_TIMES;
    }

    VixError vixError = VIX_E_FAIL;
    std::string errDesc;
    std::string selectedtransportMode;

    while (retryTimes-- >= 0) {
        if (m_diskHandle == NULL && m_devFileHandle == -1) {
            if (m_diskType == "rdm") {
                vixError = OpenDiskByLocalDevice(m_path, m_diskOpenMode, m_diskType, m_diskSize);
            } else {
                vixError = OpenDisk(m_path, m_snapshotRef, m_diskOpenMode,
                    m_chunkSize, m_transportModes, selectedtransportMode, errDesc);
            }
            if (VIX_OK != vixError) {
                errDesc = GetErrString(vixError);
                COMMLOG(OS_LOG_ERROR,
                    "Unable to open disk '%s', error: '%s', retry times: '%d'.",
                    m_path.c_str(),
                    errDesc.c_str(),
                    retryTimes + 1);
                DoSleep(SLEEP_TIME_INTERVAL);
                continue;
            }
        }

        vixError = internalOp();
        if (VIX_OK == vixError) {
            break;
        }

        // unnormal branch, will return false.
        (void)CloseDisk(errDesc);
        DoSleep(SLEEP_TIME_INTERVAL);
        COMMLOG(OS_LOG_INFO, "Need retry '%d' times.", retryTimes + 1);
    }

    return vixError;
}

VMWARE_DISK_RET_CODE VMwareDiskApi::DoRead(const uint64_t &offsetInBytes, const uint64_t &parameterBytes,
    uint64_t &bufferSizeInBytes, unsigned char *buffer, std::string &errDesc)
{
    VixError vixError = VIX_E_FAIL;
    errDesc = "Unknow error.";
    // loop to ensure data block of size parameterBytes can be read from disk
    while (bufferSizeInBytes < parameterBytes) {
        uint64_t bytesToRead = parameterBytes - bufferSizeInBytes;
        if ((m_chunkSize != 0) && (bytesToRead >= m_chunkSize)) {
            bytesToRead = m_chunkSize;
        }
        if (m_diskType == "rdm") {
            if (m_devFileHandle == -1) {
                COMMLOG(OS_LOG_ERROR, "The disk file handler is NULL");
                return VIX_E_FAIL;
            }
            if (lseek(m_devFileHandle, offsetInBytes, SEEK_SET) != offsetInBytes) {
                COMMLOG(OS_LOG_ERROR, "lseek error %d, info:%s", errno, strerror(errno));
                return VIX_E_FAIL;
            }

            uint64_t readSize = read(m_devFileHandle, buffer, bytesToRead);
            const char* bufferPtr = reinterpret_cast<const char*>(buffer);
            if (readSize == -1) {
                COMMLOG(OS_LOG_ERROR, "read error %d, info:%s", errno, strerror(errno));
                return VIX_E_FAIL;
            }
            bufferSizeInBytes += bytesToRead;
            vixError = VIX_OK;
            break;
        }
        vixError = m_vddkOperations.vixDiskLibRead(m_diskHandle,
            (offsetInBytes + bufferSizeInBytes) / SECTOR_SIZE,
            bytesToRead / SECTOR_SIZE,
            (uint8 *)buffer + bufferSizeInBytes);
        if (vixError != VIX_OK) {
            errDesc = GetErrString(vixError);
            COMMLOG(OS_LOG_ERROR,
                "Unable to read data chunk with size '%llu', offset '%llu'. Total bytes to read '%llu', bytes has been "
                "read '%llu'. Error detail: '%s'.",
                bytesToRead, offsetInBytes + bufferSizeInBytes, parameterBytes, bufferSizeInBytes,
                GetErrString(vixError).c_str());
            return vixError;
        }
        bufferSizeInBytes += bytesToRead;
    }

    return vixError;
}

VMWARE_DISK_RET_CODE VMwareDiskApi::Read(
    const uint64_t &offsetInBytes, uint64_t &bufferSizeInBytes, unsigned char *buffer, std::string &errDesc)
{
    VddkDeadlockCheck::DeadlockCheck check("Read");
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_diskHandle == NULL && m_devFileHandle == -1) {
        COMMLOG(OS_LOG_ERROR, "The disk handler is NULL!");
        return VIX_E_FAIL;
    }

    // ensure offset value is N(int type) times of SECTOR_SIZE
    bool bCheck = offsetInBytes % SECTOR_SIZE == 0;
    if (!bCheck) {
        COMMLOG(OS_LOG_ERROR, "Invalid offset value: '%llu'.", offsetInBytes);
        return VIX_E_FAIL;
    }

    // ensure offset value is less than disk size
    if (offsetInBytes >= m_diskSize) {
        COMMLOG(OS_LOG_ERROR, "The offset value '%llu' is not less than disk size '%llu'.", offsetInBytes, m_diskSize);
        return VIX_E_FAIL;
    }

    uint64_t parameterBytes = bufferSizeInBytes;
    bufferSizeInBytes = 0;

    /*
     * Note: determine the accutal reading block size
     * 1. ensure sum of offset value and accutal data size is less than ULONG_MAX
     * 2. ensure sum of offset value and accutal data size is less than disk size
     */
    if (parameterBytes > ULONG_MAX - offsetInBytes) {
        parameterBytes = ULONG_MAX - offsetInBytes;
    }

    if (offsetInBytes + parameterBytes > m_diskSize) {
        parameterBytes = m_diskSize - offsetInBytes;
    }

    VixError vixError = RetryOp(std::bind(&VMwareDiskApi::DoRead,
        this,
        std::cref(offsetInBytes),
        std::cref(parameterBytes),
        std::ref(bufferSizeInBytes),
        buffer,
        std::ref(errDesc)));
    if (vixError != VIX_OK) {
        errDesc = GetErrString(vixError);
        COMMLOG(OS_LOG_ERROR, "Unable to read disk data, error: '%s'.", errDesc.c_str());
    }

    return vixError;
}

VMWARE_DISK_RET_CODE VMwareDiskApi::DoWrite(const uint64_t &offsetInBytes, const uint64_t &parameterBytes,
    uint64_t &bufferSizeInBytes, const unsigned char *buffer, std::string &errDesc)
{
    VixError vixError = VIX_OK;
    errDesc = "Unknow error.";
    // loop to ensure data block of size parameterBytes is written to disk
    while (bufferSizeInBytes < parameterBytes) {
        uint64_t bytesToWrite = parameterBytes - bufferSizeInBytes;
        if ((m_chunkSize != 0) && (bytesToWrite >= m_chunkSize)) {
            bytesToWrite = m_chunkSize;
        }

        // ensure both the offset value and buffer size value are N(int type) times of SECTOR_SIZE
        bool bCheck = (offsetInBytes + bufferSizeInBytes) % SECTOR_SIZE == 0 && bytesToWrite % SECTOR_SIZE == 0;
        if (!bCheck) {
            COMMLOG(OS_LOG_ERROR, "Invalid offset value: '%llu', data buffer size value: '%llu', chunksize: '%llu'",
                offsetInBytes + bufferSizeInBytes, bytesToWrite, m_chunkSize);
            return VIX_E_FAIL;
        }
        if (m_diskType == "rdm") {
            if (m_devFileHandle == -1) {
                COMMLOG(OS_LOG_ERROR, "The disk file handler is NULL");
                return VIX_E_FAIL;
            }
            if (lseek(m_devFileHandle, offsetInBytes, SEEK_SET) != offsetInBytes) {
                COMMLOG(OS_LOG_ERROR, "lseek error %d, info:%s", errno, strerror(errno));
                return VIX_E_FAIL;
            }

            int64_t writeSize = write(m_devFileHandle, buffer, bytesToWrite);
            if (writeSize == -1) {
                COMMLOG(OS_LOG_ERROR, "write error %d, info:%s", errno, strerror(errno));
                return VIX_E_FAIL;
            }
            bufferSizeInBytes += bytesToWrite;
            vixError = VIX_OK;
            break;
        }
        vixError = m_vddkOperations.vixDiskLibWrite(m_diskHandle,
            (offsetInBytes + bufferSizeInBytes) / SECTOR_SIZE,
            bytesToWrite / SECTOR_SIZE,
            const_cast<uint8 *>(buffer) + bufferSizeInBytes);
        if (vixError != VIX_OK) {
            errDesc = GetErrString(vixError);
            COMMLOG(OS_LOG_ERROR, "Unable to write data chunk with size '%llu', offset '%llu'."
                "Total bytes to write '%llu', bytes has been written '%llu'. Error detail: '%s'.",
                bytesToWrite, offsetInBytes + bufferSizeInBytes, parameterBytes, bufferSizeInBytes,
                GetErrString(vixError).c_str());
            return vixError;
        }
        bufferSizeInBytes += bytesToWrite;
    }

    return vixError;
}

VMWARE_DISK_RET_CODE VMwareDiskApi::Write(
    const uint64_t &offsetInBytes, uint64_t &bufferSizeInBytes, const unsigned char *buffer, std::string &errDesc)
{
    VddkDeadlockCheck::DeadlockCheck check("Write");
    std::lock_guard<std::mutex> lock(m_mutex);
    errDesc = "Unknow error.";
    if (m_diskHandle == NULL && m_devFileHandle == -1) {
        COMMLOG(OS_LOG_ERROR, "The disk handler is NULL!");
        return VIX_E_FAIL;
    }

    // ensure both the offset value and buffer size value are N(int type) times of SECTOR_SIZE
    bool bCheck = bufferSizeInBytes % SECTOR_SIZE == 0 && offsetInBytes % SECTOR_SIZE == 0;
    if (!bCheck) {
        COMMLOG(OS_LOG_ERROR,
            "Invalid offset value: '%llu', data buffer size value: '%llu'.",
            offsetInBytes,
            bufferSizeInBytes);
        return VIX_E_FAIL;
    }

    if (offsetInBytes >= m_diskSize) {
        COMMLOG(
            OS_LOG_ERROR, "The offset value '%llu' is not less than the disk size '%llu'.", offsetInBytes, m_diskSize);
        return VIX_E_FAIL;
    }

    uint64_t parameterBytes = bufferSizeInBytes;
    bufferSizeInBytes = 0;

    /*
     * Note: determine the accutal reading block size
     * 1. ensure sum of offset value and accutal data size is less than ULONG_MAX
     * 2. ensure sum of offset value and accutal data size is less than disk size
     */
    if (parameterBytes > ULONG_MAX - offsetInBytes) {
        parameterBytes = ULONG_MAX - offsetInBytes;
    }

    if (offsetInBytes + parameterBytes > m_diskSize) {
        parameterBytes = m_diskSize - offsetInBytes;
    }

    VixError vixError = RetryOp(std::bind(&VMwareDiskApi::DoWrite,
        this,
        std::cref(offsetInBytes),
        std::cref(parameterBytes),
        std::ref(bufferSizeInBytes),
        buffer,
        std::ref(errDesc)));
    if (vixError != VIX_OK) {
        errDesc = GetErrString(vixError);
        COMMLOG(OS_LOG_ERROR, "Unable to write data, error: '%s'.", errDesc.c_str());
    }

    return vixError;
}

// Obtain the disk transport mode
std::string VMwareDiskApi::GetTransportMode()
{
    std::string str;
    if (m_diskHandle == NULL) {
        COMMLOG(OS_LOG_ERROR, "The disk handler is NULL!");
        return str;
    }
    const char *transportMode = m_vddkOperations.vixDiskLibGetTransportMode(m_diskHandle);
    if (transportMode != NULL) {
        str = transportMode;
    }
    return str;
}

std::string VMwareDiskApi::GetErrString(const VMWARE_DISK_RET_CODE code)
{
    std::string errString;
    char *ErrorText = m_vddkOperations.vixDiskLibGetErrorText(code, NULL);

    if (ErrorText != NULL) {
        errString = ErrorText;
        m_vddkOperations.vixDiskLibFreeErrorText(ErrorText);
    }
    return errString;
}
