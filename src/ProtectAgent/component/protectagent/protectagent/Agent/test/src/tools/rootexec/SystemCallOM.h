#ifndef _SYSTEM_CALL_OM_H_
#define _SYSTEM_CALL_OM_H_
#include "common/Defines.h"
#include "common/Types.h"
#include "mobility/OMDriver.h"
#include "mobility/MobilityDB.h"
#include "mobility/OMDetector.h"

#include "driver/share/ctl_define.h"
#include "driver/share/persist_data.h"

#include <map>
#include <vector>

class SystemCallOM {
public:
    static mp_int32 GetDiskSize(mp_string& strUniqueID);
    static mp_int32 OMStartProtect(mp_string& strUniqueID);
    static mp_int32 OMStopProtect(mp_string& strUniqueID);
    static mp_int32 OMModifyConf(mp_string& strUniqueID);
    static mp_int32 OMPauseProtect(mp_string& strUniqueID);
    static mp_int32 OMResumeProtect(mp_string& strUniqueID);
    static mp_int32 OMAddVolume(mp_string& strUniqueID);
    static mp_int32 OMDelVolume(mp_string& strUniqueID);
    static mp_int32 OMModVolume(mp_string& strUniqueID);
    static mp_int32 OMStartResync(mp_string& strUniqueID);
    static mp_int32 OMInitDriverConfig(mp_string& strUniqueID);
    static mp_int32 OMDelDriverConfig(mp_string& strUniqueID);
    static mp_int32 OMRetrieveBitMap(mp_string& strUniqueID);
    static mp_int32 OMGetStatistics(mp_string& strUniqueID);
    static mp_int32 OMGetKernelAlarm(mp_string& strUniqueID);
    static mp_int32 OMSetKernelTokenId(mp_string& strUniqueID);
    static mp_int32 StartDataProcess(mp_string& strUniqueID);
    static mp_int32 GetParamFromTmpFile(mp_string& strUniqueID, mp_string& strParam);

private:
    static mp_int32 ParseProtectStrategy(mp_string& strParams, om_drv_protect_strategy_t& protectStrategy);
    static mp_int32 ParseProtectVolume(mp_string& strParams, om_drv_protect_vol_t& protectVol);
    static mp_uchar ComputeBitmapGranularity(mp_uint32 protectSize, mp_uint32 memThreshold);
    // update linux driver config file, which is used to reload config for protection when OS reboot
    static mp_int32 CreateConfigFile(
        om_db_protect_info_t& dbProtectInfo, const ProtectVol pPartition[], mp_int32 iVolNum);
    static ProtectVol* GenerateProtectVols(std::vector<om_db_disk_info_t>& disksInfo);
    static mp_int32 ParseProtectConfig(mp_string& strParams, om_db_protect_info_t& dbProtectConf);
    static mp_int32 LoadBitmap(mp_int32 fd, uint64_t bitmapOffset, PPERS_VOL_BITMAP_DESC pBitmapDesc);
    static mp_int32 LoadConfigFile();
    static mp_int32 DoIoctl(mp_uint32 cmd, void* data);
    static mp_void CreateLockFile();
    static mp_int32 SetConfigureFile(
        struct im_config_pg config[], om_db_protect_info_t& dbProtectInfo, mp_bool fileExist, mp_int32 fd);
    static mp_int32 OMPersistPauseState(mp_uchar pause_state);
    static mp_int32 LoadBitmapAndSetInvaild(mp_int32 fd, mp_char headerDescBuff[], mp_int32 headerSize);
    static mp_int32 GetBitmapInfo(
        struct im_ctl_bitmap bitmapInfo[], const PPER_VOL_BITMAP_INFO pVolInfo, mp_int32 fd, uint64_t bitmapOffset);
    static mp_int32 WriteConfigFile(
        om_db_protect_info_t& dbProtectInfo, const ProtectVol pPartition[], mp_int32 iVolNum);
    static mp_int32 DecryptToken(mp_string& strToken, om_drv_protect_strategy_t& protectStrategy);
};
#endif
