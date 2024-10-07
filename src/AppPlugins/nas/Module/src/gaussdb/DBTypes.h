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
#ifndef __DBTYPES_H__
#define __DBTYPES_H__

#include <stdint.h>
#include <stdarg.h>
#include <list>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/trim.hpp>
#include "json/json.h"
//#include "security/Common.h"
using namespace std;

#define MAX_NUM_OF_SESSIONS 1000
#define MAX_NUM_OF_HB_TIMEOUT 6000
#define MAX_NUM_OF_PARALLEL_TASKS 40
#define MAX_NUM_OF_ALARM_EMAIL_SEND_INTERVAL 1440

#define MAX_CAPACITY_THRES          100
#define MIN_CAPACITY_THRES          1
#define MAX_CAPACITY                2048*1024*1024 //GB
#define MIN_CAPACITY                1 //GB
#define MAX_NAME_SIZE               512 //Max character is 128, because the name contain chinese character
#define MAX_PATH_SIZE               512 //Max character is 128
#define MAX_PASSWORD_SIZE           512
#define MAX_BRICK_PASSWORD_SIZE           1024

#define MAX_IP_ADDR_SIZE            32
#define MAX_DESCRIPTION_SIZE        4096 //Max character is 1024, because the description contain chinese character
#define MAX_CONFIGVALUE_SIZE        1048576 //1M
#define MAX_HOST_DEF_SIZE           65535
#define MAX_SERIALIZED_OBJECT_SIZE  20000
#define MAX_SERIALIZED_DESCRIPTION_SIZE  (4*1024*1024)
#define MAX_TASK_DETAILS_SIZE       40960
#define MAX_CHANNEL_AUTH_REPORT_SIZE 4096
#define MAX_ENV_OBJ_NAME_SIZE        4096
#define MIN_OBJECT_ID_VALUE         1
#define MIN_OBJECT_ID_VALUE_WITH_ZERO   0
#define MAX_OBJECT_ID_VALUE         LLONG_MAX
#define MIN_ENEVT_SN                1
#define MAX_ENEVT_SN                4294967295
#define MIN_TIME_YEAR               2000
#define MAX_TIME_YEAR               2037
#define MIN_USER_PASSWORD_LENGTH    8
#define MAX_USER_PASSWORD_LENGTH    32
#define MAX_CERT_NUMBER             10005
#define CA_CERT_MAX_MUN             100
#define MAX_CERT_FILE_LEN          (1024*1024)
#define MAX_CERT_FINGERPRINT_LEN    256
#define MAX_CERT_ISSUE_LEN          512
#define MAX_CERT_PASSWORD_LEN       4096
#define MAX_CERT_JOBID_LEN          128
#define MAX_CERT_JOB_STATUS_LEN     32
#define MAX_CERT_JOB_ERROR_MSG_LEN  4096
#define MAX_CACHEDATA_SIZE          524288//512KB
#define MAX_MGMT_IP_LEN             512
#define MAX_PRODUCT_MODE_LEN        32
#define MAX_VERSION_LEN             64
#define MAX_PUBKEY_LEN              1024

#ifdef __WINDOWS__
    #define MAX_BRICK_PATH      (MAX_PATH + 18)     // with the extra length of IP address and so on
#else
    #define MAX_BRICK_PATH     (_POSIX_PATH_MAX + 18)
#endif

#define MAX_ISCSI_INITIATOR_NAME    976 //Max character is 244, because the description contain chinese character
#define MAX_GUID_SIZE               37 
#define MAX_POLICYNAME_SIZE         512 //Max character is 128
#define MAX_DOMAINNAME_SIZE         512 //Max character is 128
#define MAX_HOSTGROUPNAME_LEN       512 //Max character is 128
#define MAX_NAMESPACESNAME_LEN      512 //Max character is 128
#define MAX_VIRTENVNAME_SIZE        512 //Max character is 128
#define MAX_VM_NAME_SIZE            512 //Max character is 128, because the name contain chinese character
#define MAX_DISK_NAME_SIZE          512 //Max character is 128
#define MAX_COMMON_NAME_SIZE        512 //Max character is 128
#define MAX_NAME_LIST_SIZE          2048
#define MAX_UPSESSION_ID_SIZE       1024
#define MAX_MO_REF_SIZE             256
#define MAX_DISK_LIST_SIZE          12000 //200*60
#define MAX_COMMON_NAME_SIZE        512
#define MAX_EMAIL_ADDR_SIZE         512
#define MAX_EMAIL_USERNAME          256
#define MAX_EMAIL_USERPASSWORD      256
#define MAX_EVENT_PARAMETER_SIZE    4096
#define MAX_EVENT_EXTEND_INFO_SIZE  8192
#define MAX_UINT64_SIZE             20
#define MAX_VERSION_LEN             64

#define MAX_RETENTION_KEEP_FOR_YEARS  25
#define MAX_RETENTION_KEEP_FOR_MONTHS 300
#define MAX_RETENTION_KEEP_FOR_WEEKS  1300
#define MAX_RETENTION_KEEP_FOR_DAYS   9125

/* BEGIN: Added by zhaoyan 90006115, 2015/3/21   PN: use this field to query which host the VM running or residing on*/
#define MAX_LICENSE_ITEM_VALUE 64
/* BEGIN: Added by zhaoyan 90006115, 2015/3/21 */
#define MAX_BACKUP_VERSION_SIZE 64

#define PLAN_TYPE_BACKUP_PLAN 0
#define PLAN_TYPE_COPY_PLAN 1

#define MAX_CONFIG_VALUE    (1024*2)

#define MAX_DATA_ENC_INFO_VK_SIZE     512
#define MAX_DATA_ENC_INFO_DEK_SIZE    512
#define MAX_PREVSNAP_REF_SIZE  (4*1024)


#define QUARY_ALARMDB_TIME_RANGE_BEGIN    "beginTime"
#define QUARY_ALARMDB_TIME_RANGE_END      "endTime"
#define QUARY_ALARMDB_TYPE                "type"

#define INVALID_OFFSET_SIZE -1
const uint64_t MAX_DB_AUTENTICATION_INFO_SIZE = 4096;
// *** COMMON *** BEGIN ***
enum PARAM_TYPE_E
{
    STRING_PARAM,
    ENCRYPTED_PARAM,
    INT_PARAM,
    PATTERN_PARAM,
    BLOB_PARAM,
    VARIABLE_LENGEH_STRING_PARAM,
    INVALID_PARAM
};

class CommonField{
public:
    CommonField(): iptr(nullptr),sptr(nullptr),strSize(0),type(STRING_PARAM) {     }
    virtual ~CommonField(){}
    PARAM_TYPE_E getType(){return type;}
    uint64_t*   getIntPtr(){/*cout<< "CommonField - iptr = " << iptr << " value: " << *iptr << endl;*/return iptr;}
    string*     getStrPtr(){/*cout<< "CommonField - sptr = " << sptr << " value: " << *sptr << endl;*/return sptr;}
    uint64_t    getSize(){/*cout<< "CommonField - getSize = " << strSize << endl;*/return strSize;}
    uint64_t*   iptr;
    string*     sptr;
    uint64_t    strSize;
    PARAM_TYPE_E type;
};

class FieldRouter
{
public:
//  std::vector <CommonField>&  getRouter(){return field;}
    std::vector <CommonField> field;
    PARAM_TYPE_E getFieldType(int i){return field[i].getType();}
    uint64_t*   getIntPtr(int i)    {return field[i].getIntPtr();}
    string*     getStrPtr(int i)    {return field[i].getStrPtr();}
    uint64_t    getFieldSize(int i) {return field[i].getSize();}                        
    uint64_t    getSize()           {return field.size();}
};

class CCDB_FieldRouter
{
public:
    /*存放CCDB数据库字段与程序定义的结构体字段的映射关系，{表名.列名,结构体字段}*/
    std::map<std::string, CommonField> ccdb_fields; 
    void add(CommonField field, ...)
    {
        va_list ap;        
        va_start(ap, field);
        
        string args = va_arg(ap, const char*);//第一个可变参数为字符串化可变参数列表
        if(string::npos != args.find(","))
        {//如果包含逗号，则说明指定了表名和列名       
            std::string table = va_arg(ap, const char*);       
            std::string column = va_arg(ap, const char*);
            std::string colKey = boost::trim_copy(table) + std::string(".") + boost::trim_copy(column);//组合成 "表名.列名" 的格式
            boost::to_lower(colKey);
            ccdb_fields.insert(make_pair(colKey, field));     
        }
        
        va_end(ap);
    }

    PARAM_TYPE_E getFieldType(const std::string& tableName, const std::string& colName)
    {
        std::string colKey = boost::trim_copy(tableName) + std::string(".") + boost::trim_copy(colName);        
        boost::to_lower(colKey);
        if(ccdb_fields.count(colKey))
        {
            return ccdb_fields[colKey].getType();
        }
        
        return INVALID_PARAM;
    }

    uint64_t* getIntPtr(const std::string& tableName, const std::string& colName)  
    {
        std::string colKey = boost::trim_copy(tableName) + std::string(".") + boost::trim_copy(colName); 
        boost::to_lower(colKey);
        if(ccdb_fields.count(colKey))
        {
            return ccdb_fields[colKey].getIntPtr();
        }
        
        return NULL;
    }

    string* getStrPtr(const std::string& tableName, const std::string& colName)
    {
        std::string colKey = boost::trim_copy(tableName) + std::string(".") + boost::trim_copy(colName); 
        boost::to_lower(colKey);
        if(ccdb_fields.count(colKey))
        {
            return ccdb_fields[colKey].getStrPtr();
        }
        
        return NULL;
    }

    uint64_t getFieldSize(const std::string& tableName, const std::string& colName) 
    {
        std::string colKey = boost::trim_copy(tableName) + std::string(".") + boost::trim_copy(colName); 
        boost::to_lower(colKey);
        if(ccdb_fields.count(colKey))
        {
            return ccdb_fields[colKey].getSize();
        }
        
        return 0;
    } 
};

class IntField : public CommonField
{
public:
    IntField(uint64_t* p){
        iptr = p; 
        type = INT_PARAM; 
//      cout<< "IntField constructor: ptr = " << iptr << endl;
    }
    virtual ~IntField() = default;
};

class StrField : public CommonField
{
public:
    StrField(string* p, int s){
        sptr = p; 
        type = STRING_PARAM; 
        strSize = s;
//      cout<< "StrField constructor: ptr = " << sptr << " value: " << *p << endl;
    }
    virtual ~StrField() = default;
};

class EncField : public CommonField
{
public:
    EncField(string* p, int s){
        sptr = p; 
        type = ENCRYPTED_PARAM; 
        strSize = s;
//      cout<< "EncField constructor: ptr = " << sptr << " value: " << *p << endl;
    }
    virtual ~EncField() = default;
};

class VariableLengthStrField : public CommonField
{
public:
    VariableLengthStrField(string* p)
    {
        sptr = p;
        type = VARIABLE_LENGEH_STRING_PARAM;
    }
    virtual ~VariableLengthStrField(){}
};

class BlobField : public CommonField
{
public:
    BlobField(string* p, int s){
        sptr = p; 
        type = BLOB_PARAM; 
        strSize = s;

    }
    virtual ~BlobField() = default;
};

#define SET_INT_FIELD(name, ...)         \
        router.field.push_back(IntField(&name));\
        ccdbRouter.add(IntField(&name), #__VA_ARGS__, ##__VA_ARGS__); 

#define SET_STRING_FIELD(name, size, ...)    \
        router.field.push_back(StrField(&name, (size)+1));\
        ccdbRouter.add(StrField(&name, (size)+1), #__VA_ARGS__, ##__VA_ARGS__);

#define SET_STRING_FIELD_VARIABLE_LENGTH(name)    \
        router.field.push_back(VariableLengthStrField(&name));

#define SET_ENCRYPTED_FIELD(name, size, ...) \
        router.field.push_back(EncField(&name, (size)+1));\
        ccdbRouter.add(EncField(&name, (size)+1), #__VA_ARGS__, ##__VA_ARGS__);

#define SET_BLOB_FIELD(name, size, ...)  \
        router.field.push_back(BlobField(&name, (size)+1));\
        ccdbRouter.add(BlobField(&name, (size)+1), #__VA_ARGS__, ##__VA_ARGS__);

#define SETTING_START(type) \
    type()                  \
    {                       \
        init();             \
    }                       \
    void init(){
            

#define SETTING_END(type)               \
    }                                   \
    type& operator=(const type& rhs){   \
        if(this == &rhs) return *this;  \
        copy(rhs);                      \
        return *this;                   \
    }                                   \
    type(const type& rhs){              \
        init();                         \
        copy(rhs);                      \
    }                                   \
    FieldRouter router;                 \
    uint64_t getSize(){return router.getSize();}    \
    FieldRouter&    getRouter(){return router;} \
    CCDB_FieldRouter ccdbRouter; \
    CCDB_FieldRouter& getCcdbRouter(){return ccdbRouter;}

#define SETTING_NEW_END(type)               \
    }                                   \
    type& operator=(const type& rhs){   \
        if(this == &rhs) return *this;  \
        Copy(rhs);                      \
        return *this;                   \
    }                                   \
    type(const type& rhs){              \
        init();                         \
        Copy(rhs);                      \
    }                                   \
    FieldRouter router;                 \
    uint64_t getSize(){return router.getSize();}    \
    FieldRouter&    getRouter(){return router;} \
    CCDB_FieldRouter ccdbRouter; \
    CCDB_FieldRouter& getCcdbRouter(){return ccdbRouter;}

typedef struct QueryCondition
{
    QueryCondition()
    {
        start = "";
        count = 50;
    }
    ~QueryCondition(){}

    Json::Value condition;
    std::string start;
    uint64_t count;

    void Condition(const Json::Value& inValue) { condition = inValue; }
    Json::Value Condition() { return condition; }
    void Start(const std::string& inValue) { start = inValue; }
    std::string Start() const { return start; }
    void Count(const uint64_t& inValue) { count = inValue; }
    uint64_t Count() const { return count; }
}QueryCondition_t;

typedef struct AssociateQueryCondition
{
    AssociateQueryCondition()
    {
        associateObjID = "";
        associateObjType = 0;
    }
    ~AssociateQueryCondition(){}

    Json::Value associateMetadata;
    std::string associateObjID;
    uint64_t    associateObjType;
}AssociateQueryCondition_t;

// *** COMMON *** END ***

#endif
