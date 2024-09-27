#include "apps/vmwarenative/TaskStepPrepareTargetLun.h"
#include "plugins/DataProcessClientHandler.h"
#include "apps/vmwarenative/VMwareDef.h"
#include "common/JsonUtils.h"
#include "common/Utils.h"
#include "host/host.h"
#include "common/ErrorCode.h"

using namespace std;
TaskStepPrepareTargetLun::TaskStepPrepareTargetLun(
    const mp_string &id, const mp_string &taskId, const mp_string &name, mp_int32 ratio, mp_int32 order)
    : TaskStepVMwareNative(id, taskId, name, ratio, order)
{
    m_reqMsgToDataProcess.clear();
    m_respMsgFromDataProcess.clear();
}

TaskStepPrepareTargetLun::~TaskStepPrepareTargetLun()
{
    m_reqMsgToDataProcess.clear();
    m_respMsgFromDataProcess.clear();
}

// due to one vm can has more than one disk, so should map each wwn to one lun correctlly
mp_int32 TaskStepPrepareTargetLun::Init(const Json::Value &param)
{
    mp_int32 iRet = MP_FALSE;
    static const mp_string keyStorageLunMounted = "StorageLunMounted";
    const static mp_string strKeyLunID = "lunId";
    const static mp_string strKeyWWN = "wwn";
    const static mp_string strKeyDeviceName = "deviceName";
    const static mp_string strKeyDiskNumber = "diskNumber";
    // parse task id
    mp_string strTaskID;
    GET_JSON_STRING(param, PARAM_KEY_TASKID, strTaskID);

    // get all disk list on host agent
    CHost host;
    vector<host_lun_info_t> lunList;
    iRet = host.GetDiskInfo(lunList);
    if (iRet != MP_SUCCESS || lunList.empty()) {
        ERRLOG("get disk list failed for task '%s', ret '%d'.", m_taskId.c_str(), iRet);
        return ERROR_DISK_GET_DISK_INFO_FAILED;
    }

    // append all storage lun(s)
    Json::Value jsonBodyContent;
    Json::Value lunItem;
    vector<host_lun_info_t>::iterator iter;
    for (iter = lunList.begin(); iter != lunList.end(); ++iter) {
        lunItem[strKeyLunID] = iter->lunId;
        lunItem[strKeyWWN] = iter->wwn;
        lunItem[strKeyDeviceName] = iter->deviceName;
        lunItem[strKeyDiskNumber] = iter->diskNumber;
        jsonBodyContent[keyStorageLunMounted].append(std::move(lunItem));
    }
    // append task id
    jsonBodyContent[MANAGECMD_KEY_TASKID] = strTaskID;

    // copy the backup params body
    m_reqMsgToDataProcess = std::move(jsonBodyContent);

    m_stepStatus = STATUS_INITIAL;

    return iRet;
}

// consider sending all luns mounted to dataprocess service
mp_int32 TaskStepPrepareTargetLun::Run()
{
    mp_int32 iRet = DataProcessLogic(m_reqMsgToDataProcess,
        m_respMsgFromDataProcess,
        EXT_CMD_VMWARENATIVE_PREPARE_BACKEND_STORAGE_RESOURCE,
        EXT_CMD_VMWARENATIVE_PREPARE_BACKEND_STORAGE_RESOURCE_ACK);
    if (iRet != MP_SUCCESS) {
        ERRLOG("Data process logic exec failure, task id '%s'.", m_taskId.c_str());
        return iRet;
    }

    return iRet;
}
