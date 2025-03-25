#include "afs/AfsProcess.h"

#include <cstdio>
#include <functional>
#include <map>
#include <memory>
#include <unordered_map>
#include <string>
#include "dataprocess/vmwarenative/AgentTimer.h"
#include "common/Log.h"
#include "common/Utils.h"

namespace {
const uint32_t BYTE_UNIT = 8;
const mp_double ONE_USEC = 1000.0;
const uint32_t NEED_FLITER_BLOCKS_THRES = 20;
} // namespace

std::unordered_map<int32_t, std::string> g_afsErrMsg = {{(int32_t)AFS_ERR_PARAMETER, "The parameter is incorrect."},
    {(int32_t)AFS_ERR_PARA_BUFFER_SIZE, "Incorrect buffer parameter."},
    {(int32_t)AFS_ERR_PARA_BIT_SIZE, "Bitmap size parameter error."},
    {(int32_t)AFS_ERR_PART_INDEX_INVALID, "The specified partition number is incorrect."},
    {(int32_t)AFS_ERR_PARTITION, "Partition error (support MBR, GPT, LVM)"},
    {(int32_t)AFS_ERR_LVM_PART, "LVM analysis error."},
    {(int32_t)AFS_ERR_LVM_VERSION, "The LVM version is not supported (support LVM2 version)."},
    {(int32_t)AFS_ERR_FS_VERSION, "Unsupported file system (only support ntfs, ext4, xfs)."},
    {(int32_t)AFS_ERR_FS_SUPPORT, "Partition type not recognized (only support ntfs, ext4, xfs)."},
    {(int32_t)AFS_ERR_FS_ANALYZE, "Analysis failed due to unknown reasons."},
    {(int32_t)AFS_ERR_FILE_SYMLINKS, "Unsupported symbolic link."},
    {(int32_t)AFS_ERR_FILE_TYPE, "Unsupported file type."},
    {(int32_t)AFS_ERR_IMAGE_READ, "Failed to read image data."}};

AfsProcess::AfsProcess() : m_blockSize(DEFULAT_CHUNK_SIZE), m_bitmapFileSize(0), m_diskBitmapOp(false)
{}

AfsProcess::~AfsProcess()
{
}

bool AfsProcess::CheckDisksHandle()
{
    for (auto &it : m_diskHandles) {
        if (!it.IsValid()) {
            ERRLOG("Invalid handle.");
            return false;
        }
    }
    return true;
}

int64 AfsProcess::ReadDeviceCallback(AFS_HANDLE handle, char *buf, int64 offset, int32 len)
{
    DiskHandleInfo *pHandle = reinterpret_cast<DiskHandleInfo *>(handle);
    if (pHandle == nullptr) {
        ERRLOG("pHandle is nullptr");
        return -1;
    }
    if (buf == nullptr && offset == -1 && len == 0) {
        return pHandle->m_diskSizeInBytes;
    }
    if (len < 0 || offset < 0) {
        ERRLOG("Invalid read len:%d", len);
        return -1;
    }
    uint64_t needReadLen = (uint64_t)len;
    int32_t ret = pHandle->m_reader->Read((uint64_t)offset, needReadLen, (unsigned char *)buf);
    if (ret != 0) {
        ERRLOG("Read failed, ret:%d", ret);
        return -1;
    }
    return len;
}

template <typename T>
void ReleaseMemory(T *ptr)
{
    if (ptr != nullptr) {
        free(ptr);
        ptr = nullptr;
    }
}

template <typename T>
void ReleaseArrMemory(T *arr, size_t count)
{
    for (size_t i = 0; i < count; i++) {
        if (arr[i] != nullptr) {
            free(arr[i]);
            arr[i] = nullptr;
        }
    }
    if (arr != nullptr) {
        free(arr);
        arr = nullptr;
    }
}

int32_t AfsProcess::PrepareFreeBitmapParams(AFS_HANDLE *&pHandles, int64_t *&bufSize, char **&ppBitmapBuf)
{
    int32_t ret = MP_FAILED;
    size_t diskNum = m_diskHandles.size();
    pHandles = (AFS_HANDLE *)calloc(diskNum, sizeof(void *));
    if (pHandles == nullptr) {
        ERRLOG("Calloc AFS_HANDLE[] failed");
        return MP_FAILED;
    }

    bufSize = (int64_t *)calloc(diskNum, sizeof(int64_t));
    if (bufSize == nullptr) {
        ERRLOG("Calloc buffer_size[] failed");
        return MP_FAILED;
    }

    ppBitmapBuf = (char **)calloc(diskNum, sizeof(void *));
    if (ppBitmapBuf == nullptr) {
        ERRLOG("Calloc ppBitmapBuf[] failed");
        return MP_FAILED;
    }

    for (size_t i = 0; i < diskNum; i++) {
        pHandles[i] = reinterpret_cast<void *>(&m_diskHandles[i]);
        uint64_t image_size = m_diskHandles[i].m_diskSizeInBytes;
        bufSize[i] = (image_size + (m_blockSize * BYTE_UNIT) - 1) / (m_blockSize * BYTE_UNIT);
        ppBitmapBuf[i] = (char *)calloc(1, bufSize[i]);
        if (ppBitmapBuf[i] == nullptr) {
            ERRLOG("calloc buffer_size[] error, bufSize[i]=%llu", bufSize[i]);
            return MP_FAILED;
        }
    }
    return MP_SUCCESS;
}

std::string AfsProcess::BuildDiskFreeBitmapFile(const DiskHandleInfo &info)
{
    return info.m_diskBitmapPath + "_free_bitmap.dat";
}

std::string AfsProcess::BuildDiskFilterBitmapFile(const DiskHandleInfo &info)
{
    return info.m_diskBitmapPath + "_Filter_bitmap.dat";
}

std::string AfsProcess::BuildDiskTmpBitmapFile(const DiskHandleInfo &info)
{
    return info.m_diskBitmapPath + "_tmp_bitmap.dat";
}

std::string AfsProcess::GetBitmapFile(const DiskHandleInfo &info, AfsBitmapType type)
{
    if (type == AfsBitmapType::FreeBitmap) {
        return BuildDiskFreeBitmapFile(info);
    } else if (type == AfsBitmapType::TmpBitmap) {
        return BuildDiskFilterBitmapFile(info);
    } else if (type == AfsBitmapType::SpecifiedBitmap) {
        return BuildDiskTmpBitmapFile(info);
    } else {
        return std::string("");
    }
}
int32_t AfsProcess::SaveDirtyRangeToFile(char **ppBitmapBuf, int64_t *bufSize, AfsBitmapType type)
{
    auto startTime = std::chrono::high_resolution_clock::now();
    size_t diskNum = m_diskHandles.size();
    for (size_t i = 0; i < diskNum; ++i) {
        std::string bitmapFile = GetBitmapFile(m_diskHandles[i], type);
        if (bitmapFile.empty()) {
            continue;
        }
        FILE *file = fopen(bitmapFile.c_str(), "wb");
        if (file == nullptr) {
            const int err = errno;
            ERRLOG("fopen the file failed: %s, reason: %s", bitmapFile.c_str(), std::strerror(err));
            return MP_FAILED;
        }
        size_t result = fwrite(ppBitmapBuf[i], bufSize[i], 1, file);
        if (result != 1) {
            (void)fclose(file);
            const int err = errno;
            ERRLOG("Write the file failed: %s", std::strerror(err));
            return MP_FAILED;
        }
        if (fflush(file) != 0 || fsync(fileno(file) != 0)) {
            const int err = errno;
            ERRLOG("Flush the file failed: %s", std::strerror(err));
            (void)fclose(file);
            return MP_FAILED;
        }
        (void)fclose(file);
        DBGLOG("Save the disk %s dirty range ok, file:%s.", m_diskHandles[i].m_diskUuid.c_str(), bitmapFile.c_str());
    }
    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
    INFOLOG("Save the dirty range ok duration: %d ms.", duration);
    return MP_SUCCESS;
}

int32_t AfsProcess::GetDisksFreeBlockBitmap(std::string &errMsg)
{
    INFOLOG("Enter GetDisksFreeBlockBitmap");
    AFS_READ_CALLBACK_FUNC_t readCB = &AfsProcess::ReadDeviceCallback;
    AFS_HANDLE *pHandles = nullptr;
    int64_t *bufSize = nullptr;
    char **ppBitmapBuf = nullptr;
    size_t diskNum = m_diskHandles.size();
    auto startTime = std::chrono::high_resolution_clock::now();
    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
    int32_t ret = PrepareFreeBitmapParams(pHandles, bufSize, ppBitmapBuf);
    if (ret != MP_SUCCESS) {
        errMsg = "Prepare free bitmap failed";
        ERRLOG("Prepare free bitmap failed.");
        goto Tail;
    }
    ret = getMulipleDisksFreeBlockBitmap(pHandles, diskNum, readCB, ppBitmapBuf, bufSize, m_blockSize, errMsg);
    if (ret != MP_SUCCESS) {
        errMsg = "Internal error";
        if (g_afsErrMsg.count(ret) != 0) {
            errMsg = g_afsErrMsg[ret];
        }
        ERRLOG("Get disks free block bitmap failed result: %d, errMsg:%s", ret, errMsg.c_str());
        goto Tail;
    }
    endTime = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
    INFOLOG("Get multi disks free block bitmap end, duration: %d ms.", duration);
    ret = SaveDirtyRangeToFile(ppBitmapBuf, bufSize, AfsBitmapType::FreeBitmap);
    if (ret != MP_SUCCESS) {
        errMsg = "Save dirty range failed.";
        ERRLOG("Save dirty range failed result: %d", ret);
        goto Tail;
    }
    INFOLOG("Exit GetDisksFreeBlockBitmap");
    ret = MP_SUCCESS;
Tail:
    ReleaseMemory(pHandles);
    ReleaseMemory(bufSize);
    ReleaseArrMemory(ppBitmapBuf, m_diskHandles.size());
    return ret;
}

int32_t AfsProcess::PrepareFilterBitmapParams(AFS_HANDLE *&pHandles, bool specified, int64_t *&bufSize,
    char **&ppBitmapBuf, char **&filelist)
{
#ifdef WIN32
    for (int i = 0; i < m_tmpFilesWin.size(); ++i) {
        m_tmpFilesWin[i] = GetSystemDiskChangedPathInWin(m_tmpFilesWin[i]);
    }
#endif
    std::vector<std::string> filterList = m_tmpFilesWin;
    size_t fileTotal = m_tmpFilesWin.size();
    if (specified) {
        fileTotal = m_dataExclude.m_specifiedList.size();
        filterList = m_dataExclude.m_specifiedList;
    }

    if (PrepareFreeBitmapParams(pHandles, bufSize, ppBitmapBuf) != MP_SUCCESS) {
        return MP_FAILED;
    }

    filelist = (char **)calloc(fileTotal, sizeof(void *));
    if (filelist == nullptr) {
        ERRLOG("calloc filelist[] error");
        return MP_FAILED;
    }

    for (size_t i = 0; i < fileTotal; i++) {
        filelist[i] = (char *)calloc(1, (sizeof(char) * filterList[i].size() + 1));
        if (filelist[i] == nullptr) {
            ERRLOG("calloc file failed.");
            return MP_FAILED;
        }
        int ret = strcpy_s(filelist[i], filterList[i].size(), filterList[i].c_str());
        if (ret != EOK) {
            ERRLOG("strcpy_s failed");
            return MP_FAILED;
        }
    }
    return MP_SUCCESS;
}

int32_t AfsProcess::GetDisksFilterFilesBitmap(const std::string &fsUUid, bool specified)
{
    INFOLOG("Enter GetDisksFilterFilesBitmap");
    AFS_READ_CALLBACK_FUNC_t readCB = &AfsProcess::ReadDeviceCallback;
    AFS_HANDLE *pHandles = nullptr;
    int64_t *bufSize = nullptr;
    char **ppBitmapBuf = nullptr;
    char **filelist = nullptr;

    AfsBitmapType type = AfsBitmapType::TmpBitmap;
    size_t fileTotal = m_tmpFilesWin.size();
    if (specified) {
        type = AfsBitmapType::SpecifiedBitmap;
        fileTotal = m_dataExclude.m_specifiedList.size();
    }
    auto startTime = std::chrono::high_resolution_clock::now();
    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
    int32_t ret = PrepareFilterBitmapParams(pHandles, specified, bufSize, ppBitmapBuf, filelist);
    if (ret != MP_SUCCESS) {
        goto Tail;
    }

    ret = getMulipleDisksFileListBlockBitmap(pHandles, m_diskHandles.size(), readCB, filelist, fileTotal,
        const_cast<char *>(fsUUid.c_str()), ppBitmapBuf, bufSize, m_blockSize);
    if (ret != MP_SUCCESS) {
        ERRLOG("Get multi disk files failed result: %d", ret);
        goto Tail;
    }
    endTime = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);

    INFOLOG("Get multi filter file bitmap end, duration: %d ms.", duration);
    ret = SaveDirtyRangeToFile(ppBitmapBuf, bufSize, type);
    if (ret != MP_SUCCESS) {
        ERRLOG("SaveDirtyRangeToFile failed result: %d", ret);
        goto Tail;
    }

    INFOLOG("Exit GetDisksFreeBlockBitmap");
    ret = AFS_SUCCESS;
Tail:
    ReleaseMemory(pHandles);
    ReleaseMemory(bufSize);
    ReleaseArrMemory(ppBitmapBuf, m_diskHandles.size());
    ReleaseArrMemory(filelist, fileTotal);
    return ret;
}

void AfsProcess::TransformDataBlock(const std::vector<dirty_range> &src, BlockOffsetMap &mapDirtyRangs)
{
    for (const auto &it : src) {
        mapDirtyRangs.insert(make_pair(it.start, it.length));
    }
}

void AfsProcess::TransformDataBlock(const DirtyRanges &dirtyRanges, BlockOffsetMap &mapDirtyRangs)
{
    DirtyRanges::iterator end = dirtyRanges.end(VMWARE_BLOCK_SIZE);
    for (DirtyRanges::iterator it = dirtyRanges.begin(VMWARE_BLOCK_SIZE); it != end; ++it) {
        mapDirtyRangs.insert(make_pair(it->Offset(), it->Length()));
    }
}

void AfsProcess::IntersectDirtyRanges(const BlockOffsetMap &src, const BlockOffsetMap &afs, BlockOffsetMap &dst)
{
    auto srcHead = src.lower_bound(afs.begin()->first);
    auto srcTail = src.upper_bound(afs.rbegin()->first);
    auto afsHead = afs.lower_bound(src.begin()->first);
    auto afsTail = afs.upper_bound(src.rbegin()->first);

    while ((srcHead != srcTail) && (afsHead != afsTail)) {
        if (srcHead->first < afsHead->first) {
            ++srcHead;
        } else if (afsHead->first < srcHead->first) {
            ++afsHead;
        } else {
            dst[srcHead->first] = srcHead->second;
            ++srcHead;
            ++afsHead;
        }
    }
}

void PrintDirtyRagnes(const std::string &name, const std::vector<dirty_range> &vecDr)
{
    const size_t outLen = 20;
    std::ostringstream oss;
    size_t size = vecDr.size();
    if (size <= outLen) {
        for (size_t i = 0; i < size; ++i) {
            oss << "{" << vecDr[i].start << ", " << vecDr[i].length << "}, ";
        }
        DBGLOG("The %s records:%s", name.c_str(), oss.str().c_str());
    } else {
        for (size_t i = 0; i < outLen; ++i) {
            oss << "{" << vecDr[i].start << ", " << vecDr[i].length << "}, ";
        }
        DBGLOG("The %s first 20 records:%s", name.c_str(), oss.str().c_str());
        oss.str("");
        oss.clear();
        for (size_t i = size - outLen; i < size; ++i) {
            oss << "{" << vecDr[i].start << ", " << vecDr[i].length << "}, ";
        }
        DBGLOG("The %s last 20 records:%s", name.c_str(), oss.str().c_str());
    }
}

void AfsProcess::FilterDirtyRanges(const std::vector<dirty_range> &src, const DirtyRanges &afsDr,
    std::vector<dirty_range> &newDirty)
{
    // DME_VMware下发的差量位图已经安按照4M粒度
    PrintDirtyRagnes("origin_dirty_range", src);
    BlockOffsetMap mapSrcBlocks;
    TransformDataBlock(src, mapSrcBlocks);

    BlockOffsetMap mapAfsBlocks;
    TransformDataBlock(afsDr, mapAfsBlocks);

    BlockOffsetMap newMapBlocks;
    if (mapAfsBlocks.size() > 0) {
        IntersectDirtyRanges(mapSrcBlocks, mapAfsBlocks, newMapBlocks);
    }

    newDirty.clear();
    auto iter = newMapBlocks.begin();
    for (; iter != newMapBlocks.end(); ++iter) {
        newDirty.push_back(dirty_range { iter->first, iter->second });
    }
    INFOLOG("Src block size:%d, afs block size:%d, newDirty size:%d", mapSrcBlocks.size(), mapAfsBlocks.size(),
        newDirty.size());
    PrintDirtyRagnes("new_dirty_range", newDirty);
}

int32_t AfsProcess::GetAndSaveAfsDirtyRanges(const std::vector<DiskHandleInfo> &vecDiskHandle,
    const DataExclusion &info, std::string &errMsg)
{
    INFOLOG("Enter GetAndSaveAfsDirtyRanges");
    auto startTime = std::chrono::high_resolution_clock::now();
    m_dataExclude = info;
    m_diskHandles = vecDiskHandle;
    if (!m_dataExclude.IsEnable()) {
        WARNLOG("The disk InvalidDataReduction is not enable.");
        return MP_SUCCESS;
    }

    if (m_diskHandles.size() == 0 || !CheckDisksHandle()) {
        errMsg = "The disk handle is invalid";
        ERRLOG("The disk param is invalid.");
        return MP_FAILED;
    }
    // 多磁盘场景blocksize相同
    m_blockSize = vecDiskHandle[0].m_trunkSize;
    INFOLOG("m_blockSize: %d", m_blockSize);

    if (m_dataExclude.m_deletedFiles && GetDisksFreeBlockBitmap(errMsg) != MP_SUCCESS) {
        ERRLOG("Get the disk free bitmap failed.");
        return MP_FAILED;
    }

    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
    INFOLOG("Exit GetAndSaveAfsDirtyRanges, duration: %d ms", duration);
    return MP_SUCCESS;
}

void AfsProcess::AddDirtyRange(uint64_t endAddr, uint64_t curAddr, unsigned int c, DirtyRanges &dirtyRanges)
{
    if (curAddr == 0) {
        if (c) {
            dirtyRanges.AddRange(DirtyRange(0, BYTE_UNIT * m_blockSize));
        }
        return;
    } else if (curAddr == endAddr - 1) {
        if (c) {
            uint64_t offset = curAddr * BYTE_UNIT * m_blockSize;
            uint64_t length = BYTE_UNIT * m_blockSize;
            if ((offset + length) > m_diskHandle.m_diskSizeInBytes) {
                length = m_diskHandle.m_diskSizeInBytes - offset;
            }
            dirtyRanges.AddRange(DirtyRange(offset, length));
        }
        return;
    }
    if (c == 0) {
        return;
    }

    uint64_t offset = curAddr * BYTE_UNIT * m_blockSize;
    dirtyRanges.AddRange(DirtyRange(offset, m_blockSize * BYTE_UNIT));
}

int32_t AfsProcess::DoRead(int fd, uint64_t offset, uint64_t &bufferSize, char *buffer)
{
    size_t needReadLen = (size_t)bufferSize;
    if ((size_t)offset + (size_t)bufferSize > m_bitmapFileSize) {
        needReadLen = m_bitmapFileSize - offset;
    }

    size_t readedLen = 0;
    while (needReadLen > readedLen) {
        ssize_t retLen = pread(fd, buffer + readedLen, needReadLen - readedLen, offset + readedLen);
        if (retLen <= 0) {
            ERRLOG("Read file failed. retLen: %d", retLen);
            break;
        }
        readedLen = readedLen + retLen;
    }

    if (readedLen != needReadLen) {
        ERRLOG("Read from file failed, fd=%d, offset:%llu, readedLen:%d", fd, offset, readedLen);
        return MP_FAILED;
    }
    bufferSize = readedLen;
    return MP_SUCCESS;
}

int32_t AfsProcess::PreReadBitmapFile(AfsBitmapType type, int &fd)
{
    std::string bitmapFile = GetBitmapFile(m_diskHandle, type);
    fd = open(bitmapFile.c_str(), O_RDONLY);
    if (fd == -1) {
        WARNLOG("Open file failed: %s, errno:%d", bitmapFile.c_str(), errno);
        return MP_FAILED;
    }

    struct stat statInfo;
    if (stat(bitmapFile.c_str(), &statInfo) != 0) {
        ERRLOG("Invoke stat failed.");
        close(fd);
        return MP_FAILED;
    }
    m_bitmapFileSize = statInfo.st_size;
    INFOLOG("Open file ok: %s, file size: %d", bitmapFile.c_str(), m_bitmapFileSize);
    return MP_SUCCESS;
}

int32_t AfsProcess::CalculateDirtyRange(const std::vector<dirty_range> &src, AfsBitmapType type,
    DirtyRanges &dirtyRanges)
{
    int fd = -1;
    if (PreReadBitmapFile(type, fd) != MP_SUCCESS) {
        return MP_FAILED;
    }

    uint64_t startOffset = src[0].start;
    uint64_t endAddrData = src[src.size() - 1].start + src[src.size() - 1].length;
    if (endAddrData >= m_diskHandle.m_diskSizeInBytes) {
        endAddrData = m_diskHandle.m_diskSizeInBytes;
    }

    uint64_t startAddr = startOffset / (BYTE_UNIT * m_blockSize);
    uint64_t endAddr = (endAddrData + (BYTE_UNIT * m_blockSize - 1)) / (BYTE_UNIT * m_blockSize);

    INFOLOG("The data range [%llu,%llu], bitmap range[%llu,%llu], m_blockSize: %llu.", startOffset, endAddrData,
        startAddr, endAddr, m_blockSize);

    const int PAGE_SIZE = 8 * 1024; // 8KB
    char buf[PAGE_SIZE] = {0};
    uint64_t readSize = PAGE_SIZE;
    if ((endAddr - startAddr) < PAGE_SIZE) {
        readSize = (endAddr - startAddr);
    }

    int iRet = MP_SUCCESS;
    while (startAddr < endAddr) {
        memset_s(buf, PAGE_SIZE, 0, PAGE_SIZE);
        iRet = DoRead(fd, startAddr, readSize, buf);
        if (iRet != MP_SUCCESS) {
            break;
        }

        for (uint64_t i = 0; i < readSize; ++i) {
            if (startAddr == endAddr) {
                break;
            }
            AddDirtyRange(endAddr, startAddr, buf[i], dirtyRanges);
            startAddr++;
        }
    }

    close(fd);
    return iRet;
}

int32_t AfsProcess::CalDiskAfsDirtyRanges(const std::vector<dirty_range> &src, std::vector<dirty_range> &newDr)
{
    auto startTime = std::chrono::high_resolution_clock::now();
    DirtyRanges afsDr;
    if (m_dataExclude.m_deletedFiles && CalculateDirtyRange(src, AfsBitmapType::FreeBitmap, afsDr) != MP_SUCCESS) {
        WARNLOG("Cal disk free dirty ranges failed.");
        return MP_FAILED;
    }

    // 过滤差量位图
    FilterDirtyRanges(src, afsDr, newDr);
    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
    INFOLOG("Exit CalDiskAfsDirtyRanges, duration:%d ms", duration);
    return MP_SUCCESS;
}

int32_t AfsProcess::GetDiskFilterDirtyRanges(const std::vector<dirty_range> &src, const DiskHandleInfo &info,
    const DataExclusion &exclInfo, std::vector<dirty_range> &newDr)
{
    INFOLOG("Enter GetDiskFilterDirtyRanges. m_blockSize: %d, Src Dirty size:%d", info.m_trunkSize, src.size());
    auto startTime = std::chrono::high_resolution_clock::now();
    m_diskBitmapOp = true;
    m_diskHandle = info;
    m_dataExclude = exclInfo;
    if (!m_dataExclude.IsEnable()) {
        WARNLOG("The excel option is not enable.");
        return MP_FAILED;
    }

    if (!m_diskHandle.IsValid() || src.size() == 0) {
        WARNLOG("The disk handle param is invalid");
        return MP_FAILED;
    }

    if (src.size() < NEED_FLITER_BLOCKS_THRES) {
        WARNLOG("No need to filter dirty ranges");
        return MP_FAILED;
    }

    m_blockSize = info.m_trunkSize;
    if (CalDiskAfsDirtyRanges(src, newDr) != MP_SUCCESS) {
        WARNLOG("Cal disk afs dirty ranges failed.");
        return MP_FAILED;
    }
    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
    INFOLOG("Exit GetDiskFilterDirtyRanges, duration:%d ms", duration);
    return MP_SUCCESS;
}

int32_t AfsProcess::RemoveDiskBitMapFile()
{
    if (!m_diskBitmapOp) {
        return MP_SUCCESS;
    }
    if (!m_diskHandle.IsValid() || !m_dataExclude.IsEnable()) {
        ERRLOG("The excel option is not enable.");
        return MP_SUCCESS;
    }
    std::string filePath;
    if (m_dataExclude.m_deletedFiles) {
        filePath = BuildDiskFreeBitmapFile(m_diskHandle);
        remove(filePath.c_str());
    } else if (m_dataExclude.m_tmpFiles) {
        filePath = BuildDiskTmpBitmapFile(m_diskHandle);
        remove(filePath.c_str());
    } else {
        filePath = BuildDiskFilterBitmapFile(m_diskHandle);
        remove(filePath.c_str());
    }
    INFOLOG("Exit RemoveDiskBitMapFile");
    return MP_SUCCESS;
}
