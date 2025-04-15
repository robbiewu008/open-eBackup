/*
* This file is a part of the open-eBackup project.
* This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
* If a copy of the MPL was not distributed with this file, You can obtain one at
* http://mozilla.org/MPL/2.0/.
*
* Copyright (c) [2024] Huawei Technologies Co.,Ltd.
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*/
#include "apps/oracle/OracleInfo.h"
#include "common/Log.h"
#include "common/Path.h"
#include "common/ErrorCode.h"
#include "securecom/RootCaller.h"
#include "common/CSystemExec.h"
#include "securecom/SecureUtils.h"
using namespace std;
OracleInfo::OracleInfo()
{}

OracleInfo::~OracleInfo()
{}

/* ------------------------------------------------------------
Description  : get oracle list
Input        : lstOracleInstInfo -- oracle database list to be returned
Return       : MP_SUCCESS -- success
               NO MP_SUCCESS -- failed,return error code
Create By    : wangguitao 90006164
------------------------------------------------------------- */
mp_int32 OracleInfo::GetDBInfo(list<oracle_inst_info_t>& lstOracleInstInfo, mp_string& strVer, mp_string& strHome)
{
    mp_int32 iRet;
    mp_string strParam;
    vector<mp_string> vecResult;

    COMMLOG(OS_LOG_DEBUG, "Begin get oracle info.");
#ifdef WIN32
    // windows�µ��ýű�
    iRet = SecureCom::SysExecScript(mp_string(WIN_ORACLE_INFO), strParam, &vecResult);
    if (iRet != MP_SUCCESS) {
        mp_int32 iNewRet = ErrorCode::GetInstance().GetErrorCode(iRet);
        COMMLOG(
            OS_LOG_ERROR, "Exec script failed, initial return code is %d, tranformed return code is %d", iRet, iNewRet);
        return iNewRet;
    }
#else
    // oracle���л���Oracle�û���ִ��ִ��SQLPLUS������root��ִ��
    CRootCaller rootCaller;
    iRet = rootCaller.Exec((mp_int32)ROOT_COMMAND_SCRIPT_QUERYORACLEINFO, strParam, &vecResult);
    TRANSFORM_RETURN_CODE(iRet, ERROR_COMMON_SCRIPT_EXEC_FAILED);
#endif
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Excute oracle info script failed, iRet %d", iRet);
        return iRet;
    }

    // find oracle database info by script
    (mp_void) AnalyseInstInfoScriptRst(vecResult, strVer, strHome, lstOracleInstInfo);

    // anlyse config file, use config database info first when the result of script has this instance info still.
    iRet = AnalyseDatabaseNameByConfFile(lstOracleInstInfo);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Analyse oracle instance mapping info failed, iRet %d", iRet);
        return iRet;
    }

    COMMLOG(OS_LOG_DEBUG, "Get oracle info succ.");
    return MP_SUCCESS;
}

/* ------------------------------------------------------------
Description  : build the script parameter for freeze or thaw database
Input        : stDBInfo -- the database structor
                strFrushType -- freeze type {freeze|thaw}
Output      : strParam -- the prameter to be returned
Return       : MP_SUCCESS -- success
               NO MP_SUCCESS -- failed,return error code
Create By    : wangguitao 90006164
------------------------------------------------------------- */
mp_void OracleInfo::BuildConsistentScriptParam(oracle_db_info_t& stDBInfo, mp_string& strParam,
    const mp_string& strFrushType)
{
    strParam = mp_string(ORACLE_SCRIPTPARAM_INSTNAME) + stDBInfo.strInstName + mp_string(NODE_COLON) +
               mp_string(ORACLE_SCRIPTPARAM_DBNAME) + stDBInfo.strDBName + mp_string(NODE_COLON) +
               mp_string(ORACLE_SCRIPTPARAM_DBUSERNAME) + stDBInfo.strDBUsername + mp_string(NODE_COLON) +
               mp_string(ORACLE_SCRIPTPARAM_DBPASSWORD) + stDBInfo.strDBPassword + mp_string(NODE_COLON) +
               mp_string(ORACLE_SCRIPTPARAM_ORACLE_HOME) + stDBInfo.strOracleHome + mp_string(NODE_COLON) +
               mp_string(ORACLE_SCRIPTPARAM_FRUSHTYPE) + strFrushType;
}

/* ------------------------------------------------------------
Description  : build the parameter for the script to be excuted
Input        : stPDBInfo -- the oracle database information
Output       : strParam -- the paramter to be returned
------------------------------------------------------------- */
mp_void OracleInfo::BuildPDBInfoScriptParam(oracle_pdb_req_info_t& stPdbInfo, mp_string& strParam)
{
    strParam = mp_string(ORACLE_SCRIPTPARAM_INSTNAME) + stPdbInfo.strInstName + mp_string(NODE_COLON) +
               mp_string(ORACLE_SCRIPTPARAM_ORACLE_HOME) + stPdbInfo.strOracleHome + mp_string(NODE_COLON) +
               mp_string(ORACLE_SCRIPTPARAM_DBUSERNAME) + stPdbInfo.strDBUsername + mp_string(NODE_COLON) +
               mp_string(ORACLE_SCRIPTPARAM_DBPASSWORD) + stPdbInfo.strDBPassword + mp_string(NODE_COLON) +
               mp_string(ORACLE_SCIPRTPARAM_PDBNAME) + stPdbInfo.strPdbName;
}

mp_int32 OracleInfo::FillOracleInsInfo(mp_string vecResult, oracle_inst_info_t& oracleInstInfo)
{
    vector<mp_string> vecRst;
    CMpString::StrSplit(vecRst, vecResult, CHAR_SEMICOLON);
#ifdef WIN32
    static const mp_int32 oracleProNum = 9;
#else
    static const mp_int32 oracleProNum = 9;
#endif

    if (vecRst.size() != oracleProNum) {
        COMMLOG(OS_LOG_ERROR, "Get db inst info failed, inst info [%s].", vecResult.c_str());
        return MP_FAILED;
    }

    mp_int32 idx = 0;
    oracleInstInfo.strVersion = vecRst[idx++];
    oracleInstInfo.strInstName = vecRst[idx++];
    oracleInstInfo.strDBName = vecRst[idx++];
    oracleInstInfo.iState = atoi(vecRst[idx++].c_str());
    oracleInstInfo.iIsASMDB = atoi(vecRst[idx++].c_str());
    oracleInstInfo.authType = atoi(vecRst[idx++].c_str());
    oracleInstInfo.dbRole = atoi(vecRst[idx++].c_str());

#ifdef WIN32
    oracleInstInfo.iVssWriterStatus = atoi(vecRst[idx++].c_str());
    oracleInstInfo.strOracleHome = vecRst[idx++];
#else
    oracleInstInfo.strOracleHome = vecRst[idx++];
#endif
    oracleInstInfo.strDBUUID = vecRst[idx++];
    oracleInstInfo.iArchiveLogMode = 0;
    return MP_SUCCESS;
}

/* ------------------------------------------------------------
Description  : analyse database instance info from the result of script
Input        : vecResult -- the results returned by the script
Output       : lstOracleInstInfo -- the oracle instance list to be returned
Return       : MP_SUCCESS -- success
               NO MP_SUCCESS -- failed,return error code
Create By    : wangguitao 90006164
------------------------------------------------------------- */
mp_int32 OracleInfo::AnalyseInstInfoScriptRst(
    vector<mp_string>& vecResult, mp_string& strVer, mp_string& strHome, list<oracle_inst_info_t>& lstOracleInstInfo)
{
    oracle_inst_info_t struDBInstInfo;
    vector<mp_string>::iterator iter;
    mp_bool bFirst = MP_TRUE;

    for (iter = vecResult.begin(); iter != vecResult.end(); ++iter) {
        // first line is oracle common information, include version and oracle home
        if (bFirst == MP_TRUE) {
            vector<mp_string> oraComm;
            CMpString::StrSplit(oraComm, *iter, CHAR_SEMICOLON);
            if (oraComm.size() != ORALCE_COMMON_FIELD_NUM) {
                COMMLOG(OS_LOG_ERROR, "Get oracle common info failed, common info [%s].", iter->c_str());
                return MP_FAILED;
            }
            mp_int32 idx = 0;
            strVer = oraComm[idx++];
            strHome = oraComm[idx++];
            bFirst = MP_FALSE;
            continue;
        }

        mp_int32 iRet = FillOracleInsInfo(*iter, struDBInstInfo);
        if (iRet == MP_FAILED) {
            continue;
        }
        lstOracleInstInfo.push_back(struDBInstInfo);

        COMMLOG(OS_LOG_DEBUG,
            "The analyse result of script is:instname(%s), dbname(%s), version(%s), state(%d), "
            "authtype(%d), dbRole(%d), oraclehome(%s).",
            struDBInstInfo.strInstName.c_str(),
            struDBInstInfo.strDBName.c_str(),
            struDBInstInfo.strVersion.c_str(),
            struDBInstInfo.iState,
            struDBInstInfo.authType,
            struDBInstInfo.dbRole,
            struDBInstInfo.strOracleHome.c_str());
    }

    COMMLOG(OS_LOG_DEBUG, "Analyse oracle info script result succ.");
    return MP_SUCCESS;
}

mp_int32 OracleInfo::AnalyseDatabaseNameByConfFile(list<oracle_inst_info_t>& lstOracleInstInfo)
{
    mp_int32 iRet;
    mp_string strOraCfgFile = CPath::GetInstance().GetConfPath() + PATH_SEPARATOR + ORA_INSTANCE_DATABASE_CONFIGFILE;
    // check config exists
    if (MP_FALSE == CMpFile::FileExist(strOraCfgFile)) {
        COMMLOG(OS_LOG_DEBUG, "Oracle config file not exist, no need to analyse mapping info.");
        return MP_SUCCESS;
    }

    vector<mp_string> cfgContent;
    iRet = CMpFile::ReadFile(strOraCfgFile, cfgContent);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Read config file failed, iRet %d", iRet);
        return iRet;
    }

    std::size_t idxSep;
    mp_string strIntance;
    mp_string strDB;
    // anlayse database and instance config information
    for (vector<mp_string>::iterator iter = cfgContent.begin(); iter != cfgContent.end(); ++iter) {
        // find 1st separator( )
        idxSep = iter->find(" ");
        if (idxSep == mp_string::npos) {
            COMMLOG(OS_LOG_ERROR,
                "Get db inst mapping info failed when find 1nd separator, inst info is [%s].",
                (*iter).c_str());
            continue;
        }
        strIntance = iter->substr(0, idxSep);
        strDB = iter->substr(idxSep + 1);

        // check mapping info
        for (list<oracle_inst_info_t>::iterator iterScript = lstOracleInstInfo.begin();
             iterScript != lstOracleInstInfo.end();
             ++iterScript) {
            if (iterScript->strInstName == strIntance) {
                iterScript->strDBName = std::move(strDB);
            }
        }
    }

    return MP_SUCCESS;
}

/* ------------------------------------------------------------
Description  : analyse database pdb info from the result of script
Input        : vecResult -- the results returned by the script
Output       : vecOraclePdbInfo -- the oracle PDB vector to be returned
Return       : MP_SUCCESS -- success
               NO MP_SUCCESS -- failed,return error code
------------------------------------------------------------- */
mp_int32 OracleInfo::AnalysePDBInfoScriptRst(
    vector<mp_string>& vecResult, vector<oracle_pdb_rsp_info_t>& vecOraclePdbInfo)
{
    oracle_pdb_rsp_info_t stPDBInfo;

    COMMLOG(OS_LOG_INFO, "Begin analyse oracle PDB info script result");

    if (vecResult.empty()) {
        COMMLOG(OS_LOG_ERROR, "Get PDB info failed, result file is null.");
        return ERROR_SQLSERVER_DB_NOT_EXIST;
    }

    for (vector<mp_string>::iterator iter = vecResult.begin(); iter != vecResult.end(); ++iter) {
        COMMLOG(OS_LOG_DEBUG, "Result file: %s.", (*iter).c_str());

        // find 1st separator(;) version
        std::size_t idxSep = iter->find(STR_SEMICOLON);
        if (idxSep == mp_string::npos) {
            COMMLOG(
                OS_LOG_ERROR, "Get PDB inst info failed when find 1st separator, PDB info is [%s].", (*iter).c_str());
            continue;
        }
        mp_string strConID = iter->substr(0, idxSep);

        // find 2nd separator(;)  instance name
        std::size_t idxSepSec = iter->find(STR_SEMICOLON, idxSep + 1);
        if (idxSepSec == mp_string::npos) {
            COMMLOG(
                OS_LOG_ERROR, "Get PDB inst info failed when find 2nd separator, PDB info is [%s].", (*iter).c_str());
            continue;
        }
        mp_string strPdbName = iter->substr(idxSep + 1, (idxSepSec - idxSep) - 1);
        mp_string strStatus = iter->substr(idxSepSec + 1);

        stPDBInfo.iConID = atoi(strConID.c_str());
        if (stPDBInfo.iConID < 0) {
            COMMLOG(OS_LOG_ERROR, "PDB cond_id not defined, con_id: %d.", stPDBInfo.iConID);
            return ERROR_SQLSERVER_DB_NOT_EXIST;
        }

        stPDBInfo.strPdbName = strPdbName;
        if (stPDBInfo.strPdbName.empty()) {
            COMMLOG(OS_LOG_ERROR, "PDB name is null");
            return ERROR_SQLSERVER_DB_NOT_EXIST;
        }

        mp_int32 iRet = TranslatePDBStatus(strStatus, stPDBInfo.iStatus);
        if (iRet != MP_SUCCESS) {
            COMMLOG(OS_LOG_ERROR, "PDB status not defined, status: %s.", strStatus.c_str());
            return ERROR_SQLSERVER_DB_NOT_EXIST;
        }

        vecOraclePdbInfo.push_back(stPDBInfo);

        COMMLOG(OS_LOG_DEBUG,
            "The analyse result of script is:conID(%d), pdbName(%s), status(%d).",
            stPDBInfo.iConID,
            stPDBInfo.strPdbName.c_str(),
            stPDBInfo.iStatus);
    }

    COMMLOG(OS_LOG_INFO, "Analyse oracle PDB info script result succ.");
    return MP_SUCCESS;
}

/* ------------------------------------------------------------
Description  : Translate the string of PDB status into int32
Input        : strStatus -- status in string
Output       : iStatus -- status in int32
Return       : MP_SUCCESS -- success
               MP_FAILED -- failed
------------------------------------------------------------- */
mp_int32 OracleInfo::TranslatePDBStatus(mp_string& strStatus, mp_int32& iStatus)
{
    mp_string strPdbMounted(INIT_PDB_STATUS_MOUNTED);
    mp_string strPdbReadOnly(INIT_PDB_STATUS_READ_ONLY);
    mp_string strPdbReadWrite(INIT_PDB_STATUS_READ_WRITE);
    COMMLOG(OS_LOG_INFO, "Translate PDB status.");
    if (strStatus == strPdbMounted) {
        iStatus = PDB_MOUNTED;
    } else if (strStatus == strPdbReadOnly) {
        iStatus = PDB_READ_ONLY;
    } else if (strStatus == strPdbReadWrite) {
        iStatus = PDB_READ_WRITE;
    } else {
        COMMLOG(OS_LOG_ERROR, "Translate PDB status fialed, status: %s.", strStatus.c_str());
        return MP_FAILED;
    }
    COMMLOG(OS_LOG_INFO, "Translate PDB status succ.");
    return MP_SUCCESS;
}

mp_int32 OracleInfo::AnalyseClsInfoScriptRst(std::vector<mp_string>& vecResult, Json::Value& clsInfo)
{
    static const mp_int32 LEN_TWO = 2;
    static const mp_int32 LEN_THREE = 3;

    std::vector<mp_string> vecDomainName;
    for (vector<mp_string>::iterator iter = vecResult.begin(); iter != vecResult.end(); ++iter) {
        vector<mp_string> cls;
        CMpString::StrSplit(cls, *iter, CHAR_SEMICOLON);

        if (cls.empty()) {
            COMMLOG(OS_LOG_ERROR, "split string failed, %s.", iter->c_str());
            return ERROR_COMMON_OPER_FAILED;
        }
        // clusterType
        if (cls[0] == "ClusterType") {
            if (cls.size() != LEN_TWO) {
                return ERROR_COMMON_OPER_FAILED;
            }
            clsInfo["ClusterType"] = atoi(cls[1].c_str());
            continue;
        }
        // clusterName
        if (cls[0] == "ClusterName") {
            if (cls.size() != LEN_TWO) {
                return ERROR_COMMON_OPER_FAILED;
            }
            clsInfo["ClusterName"] = cls[1];
            continue;
        }

        // clusterIP
        if (cls[0] == "ClusterIP") {
            if (cls.size() != LEN_TWO) {
                return ERROR_COMMON_OPER_FAILED;
            }
            clsInfo["ClusterIP"] = cls[1];
            continue;
        }

        // nodes info
        Json::Value nodeInfo;
        if (cls[0] == "ClusterIPHost") {
            if (cls.size() != LEN_THREE) {
                return ERROR_COMMON_OPER_FAILED;
            }
            Json::Value nodeInfo;
            nodeInfo["NodeName"] = cls[1];
            nodeInfo["NodeIP"] = cls[LEN_TWO];
            clsInfo["Nodes"].append(std::move(nodeInfo));
        }
    }

    COMMLOG(OS_LOG_DEBUG, "Analyse cluster info success.");
    return MP_SUCCESS;
}
