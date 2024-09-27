#ifndef AFS_PROCESS_H_
#define AFS_PROCESS_H_

#include <stdint.h>

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include "afs/DirtyRanges.h"

#include "Afslibmain.h"
#include "apps/vmwarenative/VMwareDef.h"
#include "dataprocess/ioscheduler/FileIOEngine.h"
#include "dataprocess/ioscheduler/VMwareIOEngine.h"

const uint32_t DEFULAT_CHUNK_SIZE = 8192;

enum class AfsBitmapType {
    InvalidType = -1,
    FreeBitmap = 0,
    TmpBitmap = 1,
    SpecifiedBitmap = 2
};

struct DiskHandleInfo {
    DiskHandleInfo()
        : m_diskSizeInBytes(0),
          m_trunkSize(DEFULAT_CHUNK_SIZE),
          m_diskHeadSize(0),
          m_reader(nullptr),
          m_diskApi(nullptr)
    {}
    DiskHandleInfo(const std::string &diskUuid, const std::string &diskPath, const std::string &taskId,
        const std::string &parentTaskID, const std::string &diskStoragePath, uint64_t diskSizeInBytes,
        uint64_t trunkSize, uint64_t diskHeadSize, const std::shared_ptr<IOEngine> &reader,
        const std::shared_ptr<VMwareDiskApi> &diskApi)
        : m_diskUuid(diskUuid),
          m_diskPath(diskPath),
          m_taskId(taskId),
          m_parentTaskID(parentTaskID),
          m_diskBitmapPath(diskStoragePath),
          m_diskSizeInBytes(diskSizeInBytes),
          m_trunkSize(trunkSize),
          m_diskHeadSize(diskHeadSize),
          m_reader(reader),
          m_diskApi(diskApi)
    {}

    DiskHandleInfo &operator = (const DiskHandleInfo &info)
    {
        copy(info);
        return *this;
    }

    bool IsValid()
    {
        bool bCheck = (m_diskSizeInBytes == 0 || m_reader == nullptr);
        if (bCheck || !(m_trunkSize >= AFS_SECTOR_SEZE && m_trunkSize % AFS_SECTOR_SEZE == 0)) {
            return false;
        }
        if (m_diskUuid.empty() || m_diskPath.empty() || m_taskId.empty()) {
            return false;
        }
        return true;
    }

    void copy(const DiskHandleInfo &info)
    {
        this->m_diskUuid = info.m_diskUuid;
        this->m_diskPath = info.m_diskPath;
        this->m_taskId = info.m_taskId;
        this->m_parentTaskID = info.m_parentTaskID;
        this->m_diskBitmapPath = info.m_diskBitmapPath;
        this->m_diskSizeInBytes = info.m_diskSizeInBytes;
        this->m_trunkSize = info.m_trunkSize;
        this->m_diskHeadSize = info.m_diskHeadSize;
        this->m_reader = info.m_reader;
        this->m_diskApi = info.m_diskApi;
    }
    std::string m_diskUuid;
    std::string m_diskPath;
    uint64_t m_diskSizeInBytes;
    uint64_t m_trunkSize;
    uint64_t m_diskHeadSize;
    std::string m_taskId;
    std::string m_parentTaskID; // 由于是VM层下发，和m_taskId相同
    // 路径格式： /opt/advbackup/vmware/data/<m_taskID>/<diskID>_xxx.dat
    std::string m_diskBitmapPath; // 磁盘副本仓库路径
    std::shared_ptr<IOEngine> m_reader;
    std::shared_ptr<VMwareDiskApi> m_diskApi;
};

class AfsProcess {
public:
    using BlockOffsetMap = std::map<uint64_t, uint64_t>;

    AfsProcess();
    virtual ~AfsProcess();

    /* *
     * 虚拟机任务层生成无效数据差量位图文件调用接口,所有磁盘m_trunkSize相同，默认取第一个记录的m_trunkSize
     * @param vecDiskHandle 备份虚拟机所有磁盘的句柄信息列表
     * @param DataExclusion 无效数据识别选项
     * @return 返回结果，0-成功，其他失败
     */
    int32_t GetAndSaveAfsDirtyRanges(const std::vector<DiskHandleInfo> &vecDiskHandle, const DataExclusion &info,
        std::string &errMsg);

    /* *
     * 卷级任务层解析无效数据差量位图调用接口: 从文件中解析差量位图
     * @param src 卷备份收到的差量位图
     * @param info 备份磁盘的句柄信息
     * @param exclInfo 磁盘的无效数据识别信息
     * @param newDr 新的差量位图
     * @return 返回结果，0-成功，其他失败
     */
    int32_t GetDiskFilterDirtyRanges(const std::vector<dirty_range> &src, const DiskHandleInfo &info,
        const DataExclusion &exclInfo, std::vector<dirty_range> &newDr);

    // afs内部回调读取数据的函数
    static int64 ReadDeviceCallback(AFS_HANDLE handle, char *buf, int64 offset, int32 len);

private:
    int32_t RemoveDiskBitMapFile();
    bool CheckDisksHandle();
    // 解析差量位图
    int32_t PrepareFreeBitmapParams(AFS_HANDLE *&pHandles, int64_t *&bufSize, char **&pp_bitmap_buffer);
    int32_t PrepareFilterBitmapParams(AFS_HANDLE *&pHandles, bool specified, int64_t *&bufSize,
        char **&pp_bitmap_buffer, char **&filelist);

    // 获取磁盘空闲块的差量位图
    int32_t GetDisksFreeBlockBitmap(std::string &errMsg);
    // 获取指定文件差量位图
    int32_t GetDisksFilterFilesBitmap(const std::string &fsUUid, bool specified);

    // 保存差量位图文件
    std::string BuildDiskFreeBitmapFile(const DiskHandleInfo &info);
    std::string BuildDiskFilterBitmapFile(const DiskHandleInfo &info);
    std::string BuildDiskTmpBitmapFile(const DiskHandleInfo &info);
    std::string GetBitmapFile(const DiskHandleInfo &info, AfsBitmapType type);

    int32_t SaveDirtyRangeToFile(char **ppBitmapBuf, int64_t *bufSize, AfsBitmapType type);

    // 加载差量位图
    void AddDirtyRange(uint64_t endAddr, uint64_t curAddr, unsigned int c, DirtyRanges &dirtyRanges);
    int32_t PreReadBitmapFile(AfsBitmapType type, int &fd);
    int32_t DoRead(int fd, uint64_t offset, uint64_t &bufferSize, char *buffer);

    void TransformDataBlock(const std::vector<dirty_range> &src, BlockOffsetMap &mapDirtyRangs);
    void TransformDataBlock(const DirtyRanges &src, BlockOffsetMap &mapDirtyRangs);

    void IntersectDirtyRanges(const BlockOffsetMap &src, const BlockOffsetMap &afs, BlockOffsetMap &dst);
    void FilterDirtyRanges(const std::vector<dirty_range> &src, const DirtyRanges &afs,
        std::vector<dirty_range> &newDirty);
    int32_t CalculateDirtyRange(const std::vector<dirty_range> &src, AfsBitmapType type, DirtyRanges &dirtyRanges);
    int32_t CalDiskAfsDirtyRanges(const std::vector<dirty_range> &src, std::vector<dirty_range> &newDr);

private:
    AfsProcess(const AfsProcess &c);
    AfsProcess &operator = (const AfsProcess &c);

    DiskHandleInfo m_diskHandle;
    uint64_t m_blockSize;
    uint64_t m_bitmapFileSize;
    DataExclusion m_dataExclude;
    bool m_diskBitmapOp;
    std::vector<DiskHandleInfo> m_diskHandles;
    std::vector<std::string> m_tmpFilesWin = { "C:\\hiberfil.sys", "C:\\pagefile.sys", "C:\\swapfile.sys" };
};

#endif