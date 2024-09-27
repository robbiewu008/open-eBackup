#ifndef __AGENT_ORACLE_LUN_INFO_H__
#define __AGENT_ORACLE_LUN_INFO_H__

#include <vector>
#include <list>
#include "common/Types.h"
#include "common/Utils.h"
#include "array/array.h"

#include "apps/oracle/OracleDefines.h"

class OracleLunInfo {
public:
    OracleLunInfo();
    ~OracleLunInfo();

    static mp_int32 GetDBLUNInfo(oracle_db_info_t& stDBInfo, std::vector<oracle_lun_info_t>& vecLUNInfos);
private:
    static const int ORALCE_NUM_6 = 6;

private:
    static mp_int32 GetLunInfoByStorageType(
        oracle_db_info_t stDBInfo, std::vector<oracle_lun_info_t>& vecLUNInfos, const mp_string& strStorageType);
    static mp_void BuildLunInfoScriptParam(oracle_db_info_t& stDBInfo, mp_string& strParam);
    static mp_int32 AnalyseLunInfoScriptRST(
        std::vector<mp_string>& vecResult, std::vector<oracle_storage_script_info>& vecDBStorageScriptInfo);
    static mp_int32 AnalyseLunInfoByScriptRST(std::vector<oracle_storage_script_info>& vecDBStorageScriptInfo,
        std::vector<oracle_lun_info_t>& vecLUNInfos, const mp_string& strStorageType);

    static mp_bool CheckLUNInfoExists(std::vector<oracle_lun_info_t>& vecLUNInfos, oracle_lun_info_t& oracle_lun_info);
    static mp_int32 FillLunInfo(mp_string lunArray[], std::size_t iArrayLen, mp_string source, const mp_char cSep);

#ifdef LINUX
    static mp_int32 GetUDEVConfig(mp_string& strUDEVConfDir, mp_string& strUDEVRoot);
    static mp_int32 GetUDEVInfo(
        mp_string strUdevRulesFileDir, mp_string strUdevName, mp_string strUdevResult, mp_string& strUdevDeviceRecord);
#endif

#ifdef WIN32
    static mp_int32 GetLUNInfoWin(
        mp_string& strPath, sub_area_Info_t& stSubareaInfoWin, std::vector<disk_info>& vecDiskInfoWinRes,
        oracle_lun_info_t& stDBLUNInfo);
    static mp_int32 GetLUNInfoByPathWin(mp_string& strPath, std::vector<sub_area_Info_t>& vecSubareaInfoWin,
        std::vector<disk_info>& vecDiskInfoWinRes, std::vector<oracle_lun_info_t>& vecLUNInfo);
    static mp_int32 GetDBLUNInfoWin(
        std::vector<mp_string>& vecDiskPath, std::vector<oracle_storage_script_info>& vecAdaptiveLUNInfo,
        std::vector<oracle_lun_info_t>& vecLUNInfo);
    static mp_int32 GetDBLUNFSInfoWin(std::vector<mp_string>& vecDiskPath, std::vector<disk_info>& vecDiskInfoWinRes,
        std::vector<sub_area_Info_t>& vecSubareaInfoWin, std::vector<oracle_lun_info_t>& vecLUNInfos);
    static mp_int32 GetDBLUNASMInfoWin(std::vector<oracle_storage_script_info>& vecAdaptiveLUNInfo,
        std::vector<disk_info>& vecDiskInfoWinRes, std::vector<sub_area_Info_t>& vecSubareaInfoWin,
        std::vector<oracle_lun_info_t>& vecLUNInfos);
    static mp_int32 AnalyseLunInfoByScriptRSTWIN(std::vector<oracle_storage_script_info>& vecDBStorageScriptInfo,
        std::vector<oracle_lun_info_t>& vecLUNInfos, mp_string strStorageType);
    static mp_int32 AnalyseLunInfoScriptRSTWin(std::vector<mp_string>::iterator& iter,
        oracle_storage_script_info& oracle_stor_info);
#else
    static mp_int32 GetAndCheckArraySN(const mp_string& strDev, mp_string& strArraySN,
        const mp_string& strStorageType);
    static mp_int32 GetVendorAndProduct(
        const mp_string& strDev, mp_string& strVendor, mp_string& strProduct, mp_string strStorageType);
    static mp_int32 AnalyseLunInfoByScriptRSTNoWIN(std::vector<oracle_storage_script_info>& vecDBStorageScriptInfo,
        std::vector<oracle_lun_info_t>& vecLUNInfos, const mp_string& strStorageType);
    static mp_int32 AnalyzeStorageGerInfo(std::vector<oracle_storage_script_info>::iterator& iter,
        const mp_string& strUDEVConfDir,  const mp_string& strUDEVRoot, std::map<mp_string, luninfo_t>& mapLuninfo,
        std::vector<oracle_lun_info_t>& vecLUNInfos, const mp_string& strStorageType, mp_string& strDev);
    static mp_int32 AnalyzeStorageOtherInfo(std::vector<oracle_storage_script_info>::iterator& iter,
        std::vector<oracle_lun_info_t>& vecLUNInfos, const mp_string& strStorageType, const mp_string& strDev,
        std::map<mp_string, luninfo_t>::iterator& mapIter, oracle_lun_info_t& oracle_lun_info);
    static mp_int32 GetDevList(std::vector<oracle_storage_script_info>& vecDBStorageScriptInfo, mp_string& strDev,
        std::vector<mp_string>& vecResult);
    static mp_int32 AnalyzeLunList(std::vector<mp_string>& vecResult, std::map<mp_string, luninfo_t>& mapLuninfo);
    static mp_int32 GetLunUDEVInfo(const mp_string& strUDEVConfDir, const mp_string& strUDEVRoot,
        mp_string& strUDEVName, mp_string& strUDEVRes, oracle_lun_info_t& oracle_lun_info);
    static mp_void CopyValue(
        oracle_lun_info_t& oracle_lun_info, std::vector<oracle_storage_script_info>::iterator& iter);
    static mp_int32 GetHPRawDiskName(mp_string& strDev, mp_string& strSystemDevice);
    static mp_int32 AnalyseLunInfoScriptRSTNoWin(std::vector<mp_string>::iterator& iter,
        oracle_storage_script_info& oracle_stor_info);
#endif
};

#endif  // __AGENT_ORACLE_H__
