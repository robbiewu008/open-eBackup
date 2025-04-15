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
#ifndef __DB_COMMON_H__
#define __DB_COMMON_H__

#ifdef __WINDOWS__
#include <WinSock2.h>
#include <Windows.h>
#endif

#include <string>
//#include <vector>
//#include <set>
//#include <stdint.h>
//#include <json/json.h>
//#include <boost/optional.hpp>
//#include <boost/concept_check.hpp>
#include "DBTypes.h"
#include "common/CommonDefs.h"

#define ADMINDB_MODULE_NAME "AdminDB"

#ifndef UINT_TYPES
#define UINT_TYPES
#endif

#ifdef __WINDOWS__
#  ifdef AddJob
#    undef AddJob
#  endif
#endif

// error masks

using namespace std;

enum ReturnCode
{
    ADAP_OK,
    ADAP_CREATED,
    ADAP_UPDATED,
    ADAP_NOTHING_FOUND,

    ADAP_DB_ACCESS_ERR,
    ADAP_JSON_PARSE_ERR,
    ADAP_UNKNOWN_ENTITY,
    ADAP_BUSY_ENTITY,
    ADAP_WRONG_PERCENT,
    ADAP_TABLE_TOO_LONG,
    ADAP_INSUFFICIENT_SPACE,

    ADAP_DUPLICATE_ADDR,

    ADAP_NAME_TOO_LONG,
    ADAP_ILLEGAL_NAME,
    ADAP_DUPLICATE_NAME,
    ADAP_DUPLICATE_GUID,

    ADAP_SPOOLNAME_TOO_LONG,
    ADAP_ILLEGAL_SPOOLNAME,
    ADAP_UNKNOWN_SPOOLNAME,
    ADAP_DUPLICATE_SPOOLNAME,
    ADAP_SPOOL_USED_BY_NAMESPACE,
    ADAP_BRICK_BUSY,

    ADAP_BRICKNAME_TOO_LONG,
    ADAP_ILLEGAL_BRICKNAME,
    ADAP_UNKNOWN_BRICKNAME,
    ADAP_DUPLICATE_BRICKNAME,
    ADAP_CAPACITY_DECREASE,

    ADAP_NSPACENAME_TOO_LONG,
    ADAP_ILLEGAL_NSPACENAME,
    ADAP_UNKNOWN_NSPACENAME,
    ADAP_DUPLICATE_NSPACENAME,
    ADAP_USED_BY_DOMAIN,


    ADAP_HYPERNAME_TOO_LONG,
    ADAP_ILLEGAL_HYPERNAME,
    ADAP_UNKNOWN_HYPERNAME,
    ADAP_DUPLICATE_HYPERNAME,

    ADAP_MACHINENAME_TOO_LONG,
    ADAP_ILLEGAL_MACHINENAME,
    ADAP_UNKNOWN_MACHINENAME,
    ADAP_DUPLICATE_MACHINENAME,

    ADAP_USERNAME_TOO_LONG,
    ADAP_ILLEGAL_USERNAME,
    ADAP_UNKNOWN_USER,
    ADAP_DUPLICATE_USERNAME,
    ADAP_LDAP_USER,

    ADAP_PASSWORD_TOO_LONG,
    ADAP_ILLEGAL_PASSWORD,
    ADAP_BEFORE_PASSWORD_SHORTEST_TIME,
    ADAP_SAME_AS_OLD_PASSWORD,
    ADAP_SAME_AS_HISTORY_PASSWORD,
    ADAP_WRONG_OLD_PASSWORD,
    ADAP_WRONG_CREDENTIAL,

    ADAP_PATH_TOO_LONG,
    ADAP_ILLEGAL_PATH,
    ADAP_DUPLICATE_PATH,

    ADAP_DUPLICATE_JOB,
    ADAP_UNKNOWN_JOB,

    ADAP_POLICYNAME_TOO_LONG,
    ADAP_ILLEGAL_POLICYNAME,
    ADAP_UNKNOWN_POLICYNAME,
    ADAP_DUPLICATE_POLICYNAME,
    ADAP_POLICY_ILLEGAL_SCHEDULE_TYPE,
    ADAP_POLICY_BUSY,
    ADAP_POLICY_ILLEGAL_RETENTION_TYPE,
    ADAP_POLICY_ILLEGAL_EXPIRATION_DATE,
    ADAP_POLICY_ILLEGAL_RETENTION_VAULE,

    ADAP_WRONG_CAPACITY,
    ADAP_ILLEGAL_TYPE,
    ADAP_ILLEGAL_STATUS,
    ADAP_WRONG_STATUS,
    ADAP_HG_NAME_TOO_LONG,
    ADAP_HG_EMPTY_MACHINEDEF,
    ADAP_ILLEGAL_HG_NAME,
    ADAP_ILLEGAL_HG_MACHINEDEF,
    ADAP_UNKNOWN_HOSTGROUP,
    ADAP_DM_DUPLICATE,
    ADAP_DM_NAME_TOO_LONG,
    ADAP_DM_NAME_ILLEGAL,
    ADAP_DM_CIFS_OR_S3_DEDUPE,
    ADAP_DM_IS_ACTIVE,
    ADAP_DESC_TOO_LONG,
    ADAP_ILLEGAL_DESC,
    ADAP_UNKNOWN_HYPERVISOR,
    ADAP_INTERNAL_ERR,
    ADAP_ILLEGAL_SNAP,
    ADAP_SNAP_TOO_LONG,
    ADAP_UNKNOWN_SNAP,
    ADAP_HYPER_IS_SCANNING,
    ADAP_INVALID_PATH_OBJ,
    ADAP_INVALID_DATALAYOUT,
    ADAP_UNKNOWN_COMP_RESOURCE,
    ADAP_ILLEGAL_COMP_RESOURCE,
    ADAP_UNKNOWN_STOR_RESOURCE,
    ADAP_STOR_RESOURCE_INACCESSIBLE,
    ADAP_ILLEGAL_SN,
    ADAP_COMP_RESOURCE_NOT_FOUND,
    /* BEGIN: Added by zhenming 254911, 2014/7/8   PN: Perform required changes for GUI integration,add a interface for deleting a record by brick id*/
    ADAP_BR_NAME_TOO_LONG,
    ADAP_ILLEGAL_BR_NAME,
    ADAP_BR_DES_TOO_LONG,
    ADAP_ILLEGAL_BR_DES,
    /* END:   Added by zhenming 254911, 2014/7/8 */
    ADAP_INVALID_DISK_RULE,
    ADAP_INVALID_HOST_RULE,
    ADAP_QUERY_DISKUUID_ERR,
    ADAP_UNKNOWN_DISK_FOR_RESTORE,   //when do disk restore, the specific disk does not exist in the snap
    ADAP_UNKNOWN_DATASTORE_FOR_RESTORE,//when do disk restore, the specific datastore does not exist
    ADAP_MACHINE_SNAP_NOT_MATCH,     //input machine does not match the machine in the snap when restore
    ADAP_HYPERVISOR_TYPE_NOT_MATCH,  //the target hypervisor type does not match the backup hypervisor
    ADAP_USER_CANNOT_BE_DELETED,
    ADAP_USER_RELATED_RESOURCE, // there are some resource related to this user
    ADAP_USER_CANNOT_BE_LOCKED,
    ADAP_BEGINTIME_GREATER_ENDTIME,
    ADAP_SNAP_CORRUPT,          //snap is corrupted
    ADAP_EMPTY_VM_LIST_IN_HG,
    ADAP_USER_NO_RESTORE_PRIORITY, //for regular user who has no priority to do a restore
    ADAP_DM_INCOMPATIBLE_VERIFY_TYPE,

    /* BEGIN :Added by caojun  */
    ADAP_UNKNOWN_STORAGE,
    /* END:   Added by caojun */
    /* BEGIN: Added by xiaochong, 2014/9/30   PN: Perform required changes for AdminDB backup function*/
    ADAP_BACKUP_OPENDB_FAILED,
    ADAP_BACKUP_INIT_FAILED,
    ADAP_BACKUP_DB_FAILED,
    /* END:   Added by xiaochong, 2014/9/30 */

    ADAP_DUPLICATE_BRICK_DEVICE, // duplicated value for a device (of a brick)

    ADAP_NAMESP_USER_NOT_ALLOWED_BOUND, //namespace user list can only contains regular user, if any administrative user or the super user, return this error
    ADAP_MACHINE_NOT_EXIST, //virtual machine not exist,but the snap may contain the machine info
    ADAP_REACH_QUALIFICATION,
    ADAP_DB_BAK_NOT_ACCESSIBLE,//the remote repository which save backup data of database can't accessible.add by zhangmi 90007002 2015/04/29
    ADAP_RECOVERED_OBJECT_NAME_USED,
    ADAP_RECOVERED_OBJECT_NOT_MODIFIED,
    ADAP_OPENSTACK_OBJECT_NAME_USED,
    ADAP_NOT_SUPPORT_HTTP,
    ADAP_DEL_NASP_HAS_JOB,
    ADAP_COPYPLAN_SAME_REPOSITORIED,
    ADAP_BACKUP_DB_NOT_FOUNT,
    ADAP_USED_BY_COPY_PLAN,
    ADAP_COPYPLAN_DUP_REPS,
    ADAP_DEL_V3_MAP_FAILED, //Added by b00334562 for DTS2016031703331 2016-03-31
    ADAP_OVER_LIMIT,
    ADAP_CERT_TYPE_INVALID,
    ADAP_CERT_NO_NEED_UPDATE_FINGERPRINT,
	ADAP_SRC_SIZE_NOT_QEUAL_DES_SIZE,

    ADAP_DOMAIN_BRICK_LEVEL_INVALID,
    ADAP_COPYPLAN_SRC_BRICK_LEVEL_INVALID,
    ADAP_COPYPLAN_DEST_BRICK_LEVEL_INVALID,

	ADAP_HOSTGROUP_NO_LUN_INFO,
	ADAP_POLICY_TIMES_OF_DAY_INVALID_STEP,

    ADAP_ADD_HTTPINFO_DB_ERROR,
    ADAP_ADD_HTTPINFO_PARA_INFORMAL,
    ADAP_GET_HTTPINFO_DB_ERROR,
    ADAP_CHG_HTTPINFO_DB_ERROR,
    ADAP_CHG_HTTPINFO_ID_INVALID,
    ADAP_CHG_HTTPINFO_NAME_INVALID,
    ADAP_CHG_HTTPINFO_URL_INVALID,
    ADAP_CHG_HTTPINFO_NOT_EXIST,
    ADAP_CHG_HTTPINFO_PARA_INFORMAL,
    ADAP_DEL_HTTPINFO_DB_ERROR,
    ADAP_DEL_HTTPINFO_BRICKS_IN_USE,
    ADAP_DEL_HTTPINFO_NOT_ALL_SUCC,
    ADAP_AT_HTTPINFO_PROXY_NOT_REACH,
    ADAP_AT_HTTPINFO_CAN_NOT_ACCESS,
    
    ADAP_NOT_ALLOW_RUNNING_JOBS_IN_SUSPEND_OFFINE,

    ADAP_ADD_STATIC_MUTEX_FAILED,
    ADAP_CANCEL_STATIC_MUTEX_FAILED,

    ADAP_USER_MISMATCH,
    
    ADAP_ENCRYPTPWD_ERR, //add by mwx1021344 for DTS202101060JJ4I0P0D00
    ADAP_JSON_CHECK_ERR,
    ADAP_REQARGS_CHECK_ERR,

    ADAP_LAST_RC
};

enum CHECK_RC_E
{
    DONE,
    NAME_FOUND,
    NAME_NOT_FOUND,
    STRING_TOO_LONG,
    CONDITION_TOO_LONG,
    ENCRYPTION_ERR,
    DB_ERROR,
    NEED_UPDATA_DB,
    DB_FAILED_TO_GET_VMS,
    INVALID_PARAMETER,
    SYSTEM_OUT_OF_MEM,
    RECOVER_BRICK_NOT_SAME,
    DB_LAST_RC,
    DB_CONNECTION_LIMIT,
    DB_EVENT_ERROR
};

enum DB_LOGGING_MODE_E
{
    DB_PRINT_NO_LOGS,
    DB_PRINT_DEBUG_LOGS
};

enum DB_LOGIN_SPECIAL_CODE_E
{
    DB_LOGIN_OK,
    DB_LOGIN_FIRST_TIME,
    DB_LOGIN_PASSWORD_EXPIRED,
    DB_LOGIN_PASSWORD_WILL_EXPIRED,
    DB_LOGIN_INTERNAL_ERROR,
    DB_LOGIN_LAST_RC
};

enum PROLOG_RC_E
{
    PROLOG_OK,
    PROLOG_DB_NAME_NOT_FOUND,
    PROLOG_DB_ERROR,
    PROLOG_DIRECT_END_TASK,
    PROLOG_SNAP_IN_USE,
    PROLOG_SNAP_DELETING,
    PROLOG_SNAP_ERROR,
    PROLOG_SNAP_DELETED,
    PROLOG_SNAP_REQUEST_ERROR,
    PROLOG_LAST_RC
};

#define HCP_UNREGISTERED        0
#define HCP_REGISTERED          1
#define HCP_UNREGISTER_IN_PROG  2

#define UNACCESSIBLE 0
#define ACCESSIBLE   1

#define DO_REGISTER   1
#define DO_UNREGISTER 0

#define UPDATE_REGISTER 0
#define UPDATE_ACCESS   1
#define UPDATE_SESSION  2
#define UPDATA_VERSION  3
#define UPDATA_BACKUP_PROXY_IP  4
#define UPDATE_HA  5

#define FETCH_PORTION_SIZE    50
/* BEGIN: Added by zhenming 254911, 2014/7/8   PN: Perform required changes for GUI integration,add a interface for deleting a record by brick id*/
#define MAX_BRICKNAME_LEN 32
/* END:   Added by zhenming 254911, 2014/7/8 */

/* BEGIN: Added by zhaoyan 90006115, 2015/3/21   PN: use this field to query which host the VM running or residing on*/
const uint64_t LICENSE_TYPE_CPU = 0;
const uint64_t LICENSE_TYPE_SPACE = 1;
/* BEGIN: Added by zhaoyan 90006115, 2015/3/21 */

const uint64_t  MAX_DISK_BUS_NUMBER = 9; //disk bus number maximum value
const uint64_t  MAX_DISK_SLOT_NUMBER = 60; //disk slot number maximum value


struct ResponseGetVer
{
    ReturnCode  rc;
    std::string hcpVers;
    std::string hcpRelease;
    std::string hcpC;
    std::string hcpServicePack;
    std::string dbSchemaVers;
};

class RequestMsDBCommandBasic
{
public:
    RequestMsDBCommandBasic(const std::string& tlvVersion) : m_tlvVersion(tlvVersion) { }
    virtual ~RequestMsDBCommandBasic() { }
    std::string tlvVersion() const
    {
        return m_tlvVersion;
    }
    void tlvVersion(const std::string& tlvVersion)
    {
        m_tlvVersion = tlvVersion;
    }
private:
    std::string m_tlvVersion;
};

//base response class
class MsDBResponse
{
public:
    MsDBResponse(){}
    virtual ~MsDBResponse(){}
    virtual void setEventParam(std::string eventMsg) {eventParam = eventMsg;}
    std::string getEventParam()const {return eventParam;}
    std::string eventParam;
};

class RequestChangeCommon
{
public:
    Json::Value chgCondition;
    std::size_t  requestID;
    uint64_t     objId;
    RequestChangeCommon(): requestID(0), objId(0) {}
    virtual ~RequestChangeCommon() = default;
};

class ResponseChangeCommon: public MsDBResponse
{
public:
    ReturnCode    rc;
    std::string errParam;
    std::string resultParam;
    ResponseChangeCommon(): rc(ADAP_LAST_RC), errParam("") {}
    virtual ~ResponseChangeCommon() = default;
};

class ResponseReturnCode
{
public:
    ReturnCode    rc;
    ResponseReturnCode(): rc(ADAP_LAST_RC) {}
    virtual ~ResponseReturnCode() = default;
};

#endif
