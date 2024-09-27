#include <iostream>
#include <sstream>
#include <vector>
#include "mobility/MobilityDB.h"
#include "mobility/OMDriver.h"
#include "mobility/MobilityDisk.h"
#include "common/Log.h"
#include "common/Utils.h"
#include "common/Defines.h"
#include "common/Path.h"
#include "common/ConfigXmlParse.h"
#include "securec.h"
using namespace std;

class UpgradeKernel {
public:
    static mp_int32 Handle(mp_string& strAction);

private:
    static mp_int32 StopProtect();
    static mp_int32 StartProtectWithResync();
    static mp_int32 StartMonitorWithResyncMode(om_drv_protect_strategy_t& protectStrategy);
    static mp_int32 AddProtectedDisk(std::vector<om_db_disk_info_t>& disksInfo);
    static mp_int32 HandleStartProtect();
    static mp_int32 DoIoctl(mp_uint32 cmd, mp_void* data);
};

static const mp_string STOP_OLD_DRIVER = "stop";
static const mp_string RESYNC_NEW_DRIVER = "resync";

static const mp_int32  UPGRADE_KERNEL_CMD_NUM = 2;
static const mp_uint32 DRIVER_SEND_NOW = 1; 

/* ------------------------------------------------------------
Description  : handle the request of register external MK
Return       : MP_SUCCESS -- register successfully
               MP_FAILED -- register failed
Create By    : 00455045  2020/02/25
------------------------------------------------------------- */
mp_int32 UpgradeKernel::Handle(mp_string& strAction)
{
    COMMLOG(OS_LOG_DEBUG, "Begin to handle upgrade action.");
    if (strAction == STOP_OLD_DRIVER) {
        return StopProtect();
    } else if (strAction == RESYNC_NEW_DRIVER) {
        return StartProtectWithResync();
    } else {
        cout << "Not supprt the command." << endl;
        COMMLOG(OS_LOG_ERROR, "Upgrade kernel only support \"%s\" and \"%s\" commands", 
            STOP_OLD_DRIVER.c_str(), RESYNC_NEW_DRIVER.c_str());
    }

    return MP_FAILED;
}

/* ------------------------------------------------------------
Description  : stop protect thread before upgrading driver
Return       : MP_SUCCESS -- 执行成功
               MP_FAILED -- 执行失败
Create By    : 00455045  2020/02/25
------------------------------------------------------------- */
mp_int32  UpgradeKernel::StopProtect()
{
    MobilityDB db;
    mp_string strVMId;
    om_db_protect_info_t protectInfo;

    COMMLOG(OS_LOG_DEBUG, "Begin to stop driver.");

    mp_int32 iRet = db.GetProtectInfo(strVMId, protectInfo);
    if (iRet == OM_RECORD_NOT_EXIST) {
        cout << "Stop old driver success." << endl;
        COMMLOG(OS_LOG_INFO, "old driver has been stop protect.");
        return MP_SUCCESS;
    }

    iRet = DoIoctl(OM_IOCTL_STOP, NULL);
    if (iRet == MP_SUCCESS) {
        cout << "Stop old driver success." << endl;
        COMMLOG(OS_LOG_INFO, "Stop old driver success.");
    } else {
        cout << "Stop old driver failed." << endl;
        COMMLOG(OS_LOG_INFO, "Stop old driver failed.");
    }
    return iRet;
}

/* ------------------------------------------------------------
Description  : 升级之后的内核开启resync模式
Return       : MP_SUCCESS -- 执行成功
               MP_FAILED -- 执行失败
Create By    : 00455045  2020/02/25
------------------------------------------------------------- */
mp_int32 UpgradeKernel::StartProtectWithResync()
{
    COMMLOG(OS_LOG_DEBUG, "Begin to start protect with resync mode.");
    mp_int32 iRet = HandleStartProtect();
    if (iRet == MP_SUCCESS) {
        cout << "Start driver with resync mode success." << endl;
        COMMLOG(OS_LOG_INFO, "Start driver with resync mode success.");
    } else {
        cout << "Start driver with resync mode failed." << endl;
        COMMLOG(OS_LOG_INFO, "Start driver with resync mode failed.");
    }

    return iRet;
}

/* ------------------------------------------------------------
Description  : 开启driver的保护线程并添加保护磁盘
Return       : MP_SUCCESS -- 执行成功
               MP_FAILED -- 执行失败
Create By    : 00455045  2020/02/25
------------------------------------------------------------- */
mp_int32  UpgradeKernel::HandleStartProtect()
{
    MobilityDB db;
    mp_string strVMId;
    om_db_protect_info_t protectInfo;
    COMMLOG(OS_LOG_DEBUG, "Begin to handle start protect.");

    mp_int32 iRet = db.GetProtectInfo(strVMId, protectInfo);
    if (iRet == OM_RECORD_NOT_EXIST) {
        COMMLOG(OS_LOG_ERROR, "There is no protect record, start with resync success.");
        return MP_SUCCESS;
    }

    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Get protect infos failed, iRet %d.", iRet);
        return iRet;
    }

    if (db.DeleteAlarmTableInfo() != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "delete old alarm info in OMAlarmTable failed, but continue to start driver.");
    }

    // temporary set a invaild tokenid
    protectInfo.protectStrategy.strTokenId = "00000000-0000-0000-0000-000000000000";
    om_drv_protect_strategy_t drvProtectStra;
    MobilityDisk mobility;
    iRet = mobility.SetProtectStra(protectInfo, drvProtectStra);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Set protect strategy failed");
        return MP_FAILED;
    }

    iRet = StartMonitorWithResyncMode(drvProtectStra);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Start monitor with resync mode failed.");
        return MP_FAILED;
    }

    iRet = AddProtectedDisk(protectInfo.hwInfo.disksInfo);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Add protected failed.");
        return MP_FAILED;
    }

    return MP_SUCCESS;
}

/* ------------------------------------------------------------
Description  : notify driver to peotect VM with resync mode
Return       :  MP_SUCCESS -- 执行成功
               MP_FAILED -- 执行失败
Create By    : 00455045  2020/02/25
------------------------------------------------------------- */
mp_int32 UpgradeKernel::StartMonitorWithResyncMode(om_drv_protect_strategy_t& protectStrategy)
{
    COMMLOG(OS_LOG_DEBUG, "Begin to start driver with resync mode.");

    protectStrategy.sendNow = DRIVER_SEND_NOW;
    CHECK_NOT_OK(memset_s(protectStrategy.tokenId, OM_DRIVER_TOKEN_ID_LEN, 0, OM_DRIVER_TOKEN_ID_LEN));
    mp_int32 iRet = DoIoctl(OM_IOCTL_VERIFY, &protectStrategy);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Start driver withb resync mode failed, iRet %d.", iRet);
        return iRet;
    }

    COMMLOG(OS_LOG_INFO, "Start driver with resync mode.");
    return iRet;
}

/* ------------------------------------------------------------
Description  : 将数据库中的磁盘重新添加到drievr的保护中
Return       : MP_SUCCESS -- 执行成功
               MP_FAILED -- 执行失败
Create By    : 00455045  2020/02/25
------------------------------------------------------------- */
mp_int32 UpgradeKernel::AddProtectedDisk(vector<om_db_disk_info_t>& disksInfo)
{
    vector<om_db_disk_info_t>::iterator iter;
    MobilityDisk mobility;
    COMMLOG(OS_LOG_DEBUG, "Begin to add protected disk.");

    for (iter = disksInfo.begin(); iter != disksInfo.end(); iter++) {
        om_drv_protect_vol_t drvProtectVol;
        mp_int32 iRet = mobility.GetDriverProtectVol(*iter, drvProtectVol);
        if (iRet != MP_SUCCESS) {
            COMMLOG(OS_LOG_ERROR, "Get params for driver add volume failed, iRet %d.", iRet);
            return iRet;
        }

        iRet = DoIoctl(OM_IOCTL_VOL_ADD, &drvProtectVol);
        if (iRet != MP_SUCCESS) {
            COMMLOG(OS_LOG_ERROR, "Add volume failed, iRet %d.", iRet);
            return iRet;
        }

        COMMLOG(OS_LOG_INFO, "Add disk: %s into driver.", drvProtectVol.disk_path);
    }

    COMMLOG(OS_LOG_INFO, "Add protected disk success.");
    return MP_SUCCESS;
}

/* ------------------------------------------------------------
Description  : ioctl传递给driver
Return       : MP_SUCCESS -- 执行成功
               MP_FAILED -- 执行失败
Create By    : 00455045  2020/02/25
------------------------------------------------------------- */
mp_int32 UpgradeKernel::DoIoctl(mp_uint32 cmd, mp_void* data)
{
    mp_int32 fd = open(MOBILITY_DEVICE_NAME.c_str(), O_RDWR);
    if (fd < 0) {
        COMMLOG(OS_LOG_ERROR, "open device: %s failed, error: %d.", MOBILITY_DEVICE_NAME.c_str(), errno);
        return MP_FAILED;
    }

    if (ioctl(fd, cmd, data) < 0) {
        COMMLOG(OS_LOG_ERROR, "ioctl failed, error: %d.", errno);
        close(fd);
        return MP_FAILED;
    }

    close(fd);
    return MP_SUCCESS;
}

mp_bool CheckInput(int argc, char **argv)
{
    mp_bool bCheck = (argc == UPGRADE_KERNEL_CMD_NUM && (strcmp(argv[1], STOP_OLD_DRIVER.c_str()) == 0 ||
        strcmp(argv[1], RESYNC_NEW_DRIVER.c_str()) == 0));

    return bCheck;
}

void PrintHelp()
{
    cout << "usage:" << endl;
    cout << "  [full path]upgradeKernel " << STOP_OLD_DRIVER << endl;
    cout << "  [full path]upgradeKernel " << RESYNC_NEW_DRIVER << endl;
}

mp_int32 main(int argc, char **argv)
{
    if (!CheckInput(argc, argv)) {
        cout << "input paramters are error." << endl;
        PrintHelp();
        return MP_FAILED;
    }
    // UpgradeKernel.cpp is to build upgradekernel tool
    mp_int32 iRet = CPath::GetInstance().Init(argv[0]);
    if (iRet != MP_SUCCESS) {
        cout << "Init path " << argv[0] << " failed.\n" << endl;
        return iRet;
    }

    mp_string strXMLConfFilePath = CPath::GetInstance().GetConfFilePath(AGENT_XML_CONF);
    iRet = CConfigXmlParser::GetInstance().Init(strXMLConfFilePath);
    if (iRet != MP_SUCCESS) {
        cout << "Init xml conf file " << AGENT_XML_CONF <<" failed.\n" << endl;
        return iRet;
    }

    mp_string strFilePath = CPath::GetInstance().GetLogPath();
    CLogger::GetInstance().Init(mp_string(UPGRADE_KERNEL).c_str(), strFilePath);

    mp_string upgradeAction = argv[1];
    iRet = UpgradeKernel::Handle(upgradeAction);
    
    COMMLOG(OS_LOG_INFO, "execute [upgradekernel %s]succ.", upgradeAction.c_str());
    return iRet;
}
