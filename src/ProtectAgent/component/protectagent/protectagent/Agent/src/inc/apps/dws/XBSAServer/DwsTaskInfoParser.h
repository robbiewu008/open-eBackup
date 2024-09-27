#include "common/Types.h"
#include "common/MpString.h"
#include "apps/dws/XBSAServer/DwsTaskCommonDef.h"
#include "apps/dws/XBSAServer/xbsa_types.h"

class DwsTaskInfoParser {
public:
    EXTER_ATTACK mp_int32 ReadFile(const mp_string &filePath, mp_string &readLine);
    mp_int32 ParseTaskInfo(const DwsCacheInfo &cacheInfo, DwsTaskInfo &taskInfo);
    mp_int32 ParseCacheInfo(DwsCacheInfo &info, mp_string cacheFilePath);
    mp_int32 ParseFsRelation(const mp_string &filePath, DwsFsRelation &relation);
    mp_string GetBackupCacheInfoPath(const BsaObjectDescriptor &objDesc);
    mp_string GetRestoreCacheInfoPath(const BsaQueryDescriptor &objDesc);
    mp_int32 ParseBusConfig(const mp_string &cachePath, const mp_string &copyId, XbsaBusinessConfig &busConfig);

private:
    bool CheckTaskInfo(const DwsTaskInfo &taskInfo);
    bool CheckRepositories(const DwsTaskInfo &taskInfo);
    bool CheckFileServers(const DwsTaskInfo &taskInfo);
    bool CheckFilesystems(const std::vector<DwsFsInfo> &filesystems);
    bool CheckFsRelations(const DwsFsRelation &relation);
    bool CheckBusConfig(const XbsaBusinessConfig &busConfig);
};