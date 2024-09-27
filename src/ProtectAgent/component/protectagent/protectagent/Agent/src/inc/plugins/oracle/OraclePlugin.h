/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 *
 * @file OraclePlugin.h
 * @brief  Contains function declarations for OraclePlugin
 * @version 1.0.0
 * @date 2020-08-01
 * @author wangguitao 00510599
 */
#ifndef __AGENT_ORACLE_PLUGIN_H__
#define __AGENT_ORACLE_PLUGIN_H__

#include "common/Types.h"
#include "plugins/ServicePlugin.h"
#include "apps/oracle/Oracle.h"

class OraclePlugin : public CServicePlugin {
public:
    OraclePlugin();
    ~OraclePlugin();

    mp_int32 DoAction(CRequestMsg& req, CResponseMsg& rsp);

private:
    Oracle m_oracle;
    thread_id_t m_tid_CheckArchiveArea;

private:
    mp_int32 QueryInfo(CRequestMsg& req, CResponseMsg& rsp);
    mp_int32 QueryLunInfo(CRequestMsg& req, CResponseMsg& rsp);
    mp_int32 Test(CRequestMsg& req, CResponseMsg& rsp);
    mp_int32 StartRACCluster(CRequestMsg& req, CResponseMsg& rsp);
    mp_int32 GetDBAuthParam(CRequestMsg& req, oracle_db_info_t& stDBInfo);
    mp_int32 GetPDBAuthParam(CRequestMsg& req, oracle_pdb_req_info_t& stPDBInfo);
    mp_int32 GetRestDBnput(CRequestMsg& req, mp_int32& iISASM, oracle_db_info_t& stDBInfo, mp_bool isStart);
    mp_int32 QueryTableSpace(CRequestMsg& req, CResponseMsg& rsp);
    mp_void QueryTableSpaceRsp(CResponseMsg &rsp, std::vector<mp_string> &tslist, const oracle_db_info_t &stDBInfo);
    mp_int32 QueryASMInstance(CRequestMsg& req, CResponseMsg& rsp);
    mp_int32 QueryRACInfo(CRequestMsg& req, CResponseMsg& rsp);
    mp_void GetDBInfo2Rsp(std::list<oracle_inst_info_t>& lstOracleInstInfo, CResponseMsg& rsp);
    Json::Value ConvertLunInfo2Json(const oracle_lun_info_t& lunInfo);
};

#endif  // __AGENT_ORACLE_PLUGIN_H__
