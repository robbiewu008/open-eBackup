#include <string>
#include <vector>
#include <string.h>
#include <iostream>
#include <stdint.h>
#include "driver/share/ctl_define.h"
#include "common/Log.h"
#include "common/Utils.h"
#include "common/Defines.h"
#include "common/Path.h"
#include "common/ConfigXmlParse.h"
#include "common/RootCaller.h"
#include "common/JsonUtils.h"
#include "mobility/OMDetector.h"

using namespace std;

enum MENU_LIST {
    MENU_SYNC_DATA = 1,   
    MENU_SYNC_PERCENT, 
    MENU_IOPS,
    MENU_THROUGHOUT,   
    MENU_SEND_SPEND,  
    MENU_RPO,  
    MENU_DRV_DATA_STATE,    
    MENU_DRV_LINK_STATE,
    MENU_DRV_WORK_MODE,
    MENU_DRV_WORK_STATE,
    MENU_TOTAL_NUM
};

const mp_string G_SHOW_SYNC_DATA     = "1"; // clean code suggest use string replace char *
const mp_string G_SHOW_SYNC_PERCENT  = "2";
const mp_string G_SHOW_IOPS          = "3";
const mp_string G_SHOW_THROUGHOUT    = "4";
const mp_string G_SHOW_SEND_SPEND    = "5";
const mp_string G_SHOW_RPO           = "6";
const mp_string G_SHOW_DRV_DATA_STATE = "7";
const mp_string G_SHOW_DRV_LINK_STATE = "8";
const mp_string G_SHOW_DRV_WORK_MODE  = "9";
const mp_string G_SHOW_DRV_WORK_STATE = "10";
const mp_string G_SHOW_HELP          = "?";

const mp_int32 MENU_INFO_SIZE = 256;

namespace {
    const mp_char MENU_INFO[][MENU_INFO_SIZE] = {
        "reamin sync data (byte), synced data (byte)",
        "synced data percent (%), expected complete time (sec)",
        "read iops, write iops",
        "write throughout (KB/s)",
        "send data speed (KB/s)",
        "RPO time (sec)",
        "driver data state",
        "driver link state",
        "driver work mode",
        "driver work state"
    };
}

const mp_int32 G_ARGC_NUM_ONE = 1;
const mp_int32 G_ARGC_NUM_TWO = 2;

void PrintHelp()
{
    cout << "usage:\n" << endl;
    cout << "[" << MENU_SYNC_DATA    << "] " << MENU_INFO[MENU_SYNC_DATA - 1]    << endl;
    cout << "[" << MENU_SYNC_PERCENT << "] " << MENU_INFO[MENU_SYNC_PERCENT - 1] << endl;
    cout << "[" << MENU_IOPS         << "] " << MENU_INFO[MENU_IOPS - 1]         << endl;
    cout << "[" << MENU_THROUGHOUT   << "] " << MENU_INFO[MENU_THROUGHOUT - 1]   << endl;    
    cout << "[" << MENU_SEND_SPEND   << "] " << MENU_INFO[MENU_SEND_SPEND - 1]   << endl;
    cout << "[" << MENU_RPO          << "] " << MENU_INFO[MENU_RPO - 1]          << endl;
    cout << "[" << MENU_DRV_DATA_STATE  << "] " << MENU_INFO[MENU_DRV_DATA_STATE - 1]  << endl;
    cout << "[" << MENU_DRV_LINK_STATE  << "] " << MENU_INFO[MENU_DRV_LINK_STATE - 1]  << endl;
    cout << "[" << MENU_DRV_WORK_MODE   << "] " << MENU_INFO[MENU_DRV_WORK_MODE - 1]   << endl;
    cout << "[" << MENU_DRV_WORK_STATE  << "] " << MENU_INFO[MENU_DRV_WORK_STATE - 1]  << endl;
    cout << "Enter the corresponding number to show info." << endl;
}
bool CheckInput(int argc, char **argv)
{
    bool bCheck = ((argc == G_ARGC_NUM_TWO) && ((strcmp(argv[1], G_SHOW_SYNC_DATA.c_str()) == 0) 
        || (strcmp(argv[1], G_SHOW_SYNC_PERCENT.c_str()) == 0) || (strcmp(argv[1], G_SHOW_IOPS.c_str()) == 0)
        || (strcmp(argv[1], G_SHOW_THROUGHOUT.c_str()) == 0) || (strcmp(argv[1], G_SHOW_SEND_SPEND.c_str()) == 0)
        || (strcmp(argv[1], G_SHOW_RPO.c_str()) == 0) || (strcmp(argv[1], G_SHOW_DRV_DATA_STATE.c_str()) == 0) 
        || (strcmp(argv[1], G_SHOW_DRV_LINK_STATE.c_str()) == 0)
        || (strcmp(argv[1], G_SHOW_DRV_WORK_MODE.c_str()) == 0) 
        || (strcmp(argv[1], G_SHOW_DRV_WORK_STATE.c_str()) == 0) 
        || (strcmp(argv[1], G_SHOW_HELP.c_str()) == 0)));
    bCheck = (bCheck || (argc == G_ARGC_NUM_ONE));
    return bCheck;
}

mp_int32 GetStatisticsInfo(StatisticsInfo &info)
{
    mp_int32 iRet;
    vector<mp_string>vecResult;

    COMMLOG(OS_LOG_DEBUG, "Begin get statistics info.");

#ifdef WIN32
    iRet = SendIOControl(mp_string(MOBILITY_DEVICE_NAME), OM_IOCTL_GET_STATISTICS, static_cast<mp_void*>(&info), 0);
#else
    CRootCaller rootCaller;
    iRet = rootCaller.Exec((mp_int32)ROOT_COMMAND_OM_GET_STATISTICS, "", &vecResult);    
#endif
    if (iRet != MP_SUCCESS || vecResult.empty()) {        
        COMMLOG(OS_LOG_ERROR, "Send statistics info ioctl failed.");
        return iRet;
    }

#ifndef WIN32 
    Json::Value jv;
    Json::Reader r;
    mp_string strStatistics = vecResult[0];
    COMMLOG(OS_LOG_DEBUG, "strStatistics is %s", strStatistics.c_str());

    mp_bool bRet = r.parse(strStatistics, jv);
    if (bRet != MP_TRUE) {
        COMMLOG(OS_LOG_ERROR, "Parse statistics info failed.");
        return MP_FAILED;
    }

    GET_JSON_UINT64(jv, REMAIN_SYNC_DATA, info.remain_sync_data);
    GET_JSON_UINT64(jv, SYNCED_DATA, info.synced_data);
    GET_JSON_UINT32(jv, SYNCED_DATA_RATE, info.synced_data_rate);
    GET_JSON_UINT32(jv, EXPECTED_TIME, info.expected_time);
    GET_JSON_UINT32(jv, WRITE_IOPS, info.write_iops);
    GET_JSON_UINT64(jv, WRITE_THROUGHOUT, info.write_throughout);
    GET_JSON_UINT64(jv, DATA_SEND_SPEED, info.data_send_speed);
    GET_JSON_UINT32(jv, DRIVER_RPO_TIME, info.driver_rpo_time);
    GET_JSON_INT32(jv, DRIVER_DATA_STATE, info.driver_pair_state.data_state);
    GET_JSON_INT32(jv, DRIVER_LINK_STATE, info.driver_pair_state.link_state);
    GET_JSON_INT32(jv, DRIVER_WORK_MODE,  info.driver_pair_state.work_mode);
    GET_JSON_INT32(jv, DRIVER_WORK_STATE, info.driver_pair_state.work_state);

#endif
    COMMLOG(OS_LOG_DEBUG, "Get statistics info succ.");
    return iRet;   
}

void ShowSyncData(const StatisticsInfo info)
{
    if (info.driver_pair_state.data_state != DATA_STATE_NORMAL) { 
        cout << "remain sync data : " << info.remain_sync_data << "(bytes)" << ", synced data : " 
            << info.synced_data << "(bytes)" << endl;
    } else {
        cout << "remain sync data : --, synced data : -- (driver in normal state)" << endl;
    }
}

void ShowSyncPercent(const StatisticsInfo info)
{
    if (info.driver_pair_state.work_state == WORK_STATE_NORMAL 
        && info.driver_pair_state.link_state == LINK_STATE_NORMAL) {
        cout << "synced data percent : " << info.synced_data_rate << "%" << ", expected time : " 
            << info.expected_time  << "(sec)" << endl;
    } else {
        cout << "synced data percent : " << info.synced_data_rate << "%" << 
            ", expected time : -- (driver can not send data)" << endl;
    }
}

void ShowIOPS(const StatisticsInfo info)
{
    cout << "write iops : " << info.write_iops << endl;
}

void ShowThroughout(const StatisticsInfo info)
{
    cout << "write throughout : " << info.write_throughout << "(KB/s)" << endl;
}

void ShowSendSpend(const StatisticsInfo info)
{
    cout << "send data speed : " << info.data_send_speed << "(KB/s)" << endl;
}

void ShowRPO(const StatisticsInfo info)
{
    if (info.driver_pair_state.data_state == DATA_STATE_NORMAL) {
        cout << "RPO time : " << info.driver_rpo_time << "(sec)" << endl;
    } else {
        cout << "RPO time : -- (only show rpo time when data state is normal)" << endl;
    }
}

void ShowDrvDataState(const StatisticsInfo info)
{
    cout << "data state : " << info.driver_pair_state.data_state << "(0:cbt, 1:mormal)" << endl;
}

void ShowDrvLinkState(const StatisticsInfo info)
{
    cout << "link state : " << info.driver_pair_state.link_state << "(0:break, 1:normal)" << endl;
}

void ShowDrvWorkMode(const StatisticsInfo info)
{
    cout << "work mode : " << info.driver_pair_state.work_mode << "(0:resync, 1:initialize, 2:syncing)" << endl;
}

void ShowDrvWorkState(const StatisticsInfo info)
{
    cout << "work state : " << info.driver_pair_state.work_state << "(0:pause, 1:no buffer, 2:normal)" << endl;
}

void ShowAllItems(const StatisticsInfo info)
{
    ShowSyncData(info);
    ShowSyncPercent(info);
    ShowIOPS(info);
    ShowThroughout(info);
    ShowSendSpend(info);
    ShowRPO(info);
    ShowDrvDataState(info);
    ShowDrvLinkState(info);
    ShowDrvWorkMode(info);
    ShowDrvWorkState(info);
}

void ShowStatisticsItems(const StatisticsInfo info, const std::string idStr)
{
    if (idStr == G_SHOW_SYNC_DATA) {
        ShowSyncData(info);
    } else if (idStr == G_SHOW_SYNC_PERCENT) {
        ShowSyncPercent(info);
    } else if (idStr == G_SHOW_IOPS) {
        ShowIOPS(info);
    } else if (idStr == G_SHOW_THROUGHOUT) {
        ShowThroughout(info);
    } else if (idStr == G_SHOW_SEND_SPEND) {
        ShowSendSpend(info);
    } else if (idStr == G_SHOW_RPO) {
        ShowRPO(info);
    }
}

void ShowDriverStateItmes(const StatisticsInfo info, const std::string idStr)
{
    if (idStr == G_SHOW_DRV_DATA_STATE) {
        ShowDrvDataState(info);
    } else if (idStr == G_SHOW_DRV_LINK_STATE) { 
        ShowDrvLinkState(info);
    } else if (idStr == G_SHOW_DRV_WORK_MODE) {
        ShowDrvWorkMode(info);
    } else if (idStr == G_SHOW_DRV_WORK_STATE) { 
        ShowDrvWorkState(info);
    }    
}

void ShowItem(const StatisticsInfo info, const std::string idStr)
{
    ShowStatisticsItems(info, idStr);
    ShowDriverStateItmes(info, idStr);
}
mp_int32 main(int argc, char **argv)
{
    if (!CheckInput(argc, argv)) {
        cout << "input paramters are error." << endl;
        PrintHelp();
        return MP_FAILED;
    }
    
    // 初始化路径
    mp_int32 iRet = CPath::GetInstance().Init(argv[0]);
    if (iRet != MP_SUCCESS) {
        cout << "Init path " << argv[0] << " failed.\n" << endl;
        return iRet;
    }

    // 初始化xml配置
    mp_string strXMLConfFilePath = CPath::GetInstance().GetConfFilePath(AGENT_XML_CONF);
    iRet = CConfigXmlParser::GetInstance().Init(strXMLConfFilePath);
    if (iRet != MP_SUCCESS) {
        cout << "Init xml conf file " << AGENT_XML_CONF <<" failed.\n" << endl;
        return iRet;
    }

    // 初始化日志路径
    mp_string strFilePath = CPath::GetInstance().GetLogPath();
    CLogger::GetInstance().Init(mp_string(SHOW_STATISTICS).c_str(), strFilePath);

    COMMLOG(OS_LOG_DEBUG, "Begin get driver statistics info.");
    
    StatisticsInfo info;
    if (GetStatisticsInfo(info) == MP_FAILED) {
        COMMLOG(OS_LOG_ERROR, "send ioctl to driver failed.");
        cout << "Get statistics info failed, and may be the VM is not in protect.\n" << endl;
        return MP_FAILED;
    }

    if (argc == G_ARGC_NUM_ONE) {
        ShowAllItems(info);    
    } else if (strcmp(argv[1], G_SHOW_HELP.c_str()) == 0) {
        PrintHelp();
    } else {
        ShowItem(info, string(argv[1]));
    }
    
    COMMLOG(OS_LOG_INFO, "Get driver statistics succ.");
    return MP_SUCCESS;
}
