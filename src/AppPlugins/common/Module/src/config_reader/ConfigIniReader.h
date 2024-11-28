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
#ifndef CONFIGREADER_H
#define CONFIGREADER_H

#include <string>
#include <list>
#include <set>
#include <vector>
#ifdef WIN32
#include "define/Defines.h"
#endif

namespace Module {

const std::string MS_CFG_GENERAL_SECTION = "General";
const std::string MS_CFG_BACKUPNODE_SECTION = "BackupNode";
const std::string MS_CFG_MICROSERVICE_SECTION = "MicroService";
const std::string MS_CFG_WRITE_RETRY_TIMES = "reTryTimes";
const std::string MS_CFG_DATABASE_SECTION = "DataBase";

const std::string MS_CFG_PORT = "Port";
const std::string MS_CFG_LOG_LEVEL = "LogLevel";
const std::string MS_CFG_LOG_COUNT = "LogCount";
const std::string MS_CFG_LOG_MAX_SIZE = "LogMaxSize";
const std::string MS_CFG_LOG_KEEP_TIME = "LogKeepTime";
const std::string MS_CFG_LOG_CACHE_THRESHOLD = "LogCacheThreshold";
const std::string MS_CFG_LOG_FLUSH = "LogFlushTime";
const std::string MS_CFG_LOG_PATH = "EbkSDKLogPath";
const std::string MS_CFG_MICRO_SRV_NAME = "MicroServiceName";
const std::string MS_CFG_PID = "PID";
const std::string MS_CFG_LOGLIMITINTERVALTIME = "LogLimitIntervalTime";
const std::string MS_CFG_LOGLIMITFREQUENCY = "LogLimitFrequency";

const std::string MS_CFG_FCGI_HANDLER_SIZE = "FCGI_HANDLER_SIZE";
const std::string MS_CFG_REQ_MAX_SIZE = "REQ_MAX_SIZE";
const std::string MS_CFG_HTTP_BUSY_REPEAT_TIME = "HTTP_BUSY_REPEAT_TIME";
const std::string MS_CFG_HTTP_BUSY_REPEAT_INTERVAL = "HTTP_BUSY_REPEAT_INTERVAL";
const std::string MS_CFG_HW_TRACER = "GrayTrace";

const std::string MS_CFG_WRITE_BLOCK_TASK_NUM = "CommonTaskUsingMaxThread4Backup";
const std::string MS_CFG_WRITE_BLOCK_THREAD_TOTAL = "WriteBlockThreadTotal";
const std::string MS_CFG_WRITE_BLOCK_TASK_SLEEP_TIME = "WriteBlockTaskSleepTime";
const std::string MS_CFG_READ_BLOCK_TASK_SLEEP_TIME = "ReadBlockTaskSleepTime";
const std::string MS_CFG_READ_BLOCK_BUF_SIZE = "ReadBlockBufSize";
const std::string MS_CFG_READ_BLOCK_BUF_SIZE_ON_32G_MEM = "ReadBlockBufSizeOn32GMem";
const std::string MS_CFG_READ_BLOCK_BUF_TOTAL_SIZE = "ReadBlockBufTotalSize";
const std::string MS_CFG_READ_BLOCK_BUF_TOTAL_SIZE_ON_32G_MEM = "ReadBlockBufTotalSizeOn32GMem";
const std::string MS_CFG_UPDATE_SNAP_FILE_INTERVAL = "UpdateSnapFileInterval";
const size_t MS_CFG_DEFAULT_VALUE_UPDATE_SNAP_FILE_INTERVAL = 15000;
const std::string MS_CFG_MAX_VERIFY_TASK_NUM = "CommonTaskUsingMaxThread4Verify";
const std::string MS_CFG_CONNECTION_CHECK_INTERVAL = "ConnectionCheckInterval";

const std::string MS_CFG_READ_BLOCK_TASK_NUM = "CommonTaskUsingMaxThread4Restore";
const std::string MS_CFG_S3_URL_STYLE = "S3URLStyle";
const std::string MS_CFG_RW_Parallel_Num = "ProductStorageRWParallelNum";
const std::string MS_CFG_RW_QOS_HEAVY_LEVEL = "ProductStorageRWVelocityHeavy";
const std::string MS_CFG_RW_QOS_MIDDLE_LEVEL = "ProductStorageRWVelocityMiddle";
const std::string MS_CFG_RW_QOS_LOW_LEVEL = "ProductStorageRWVelocityLow";
const std::string MS_CFG_RW_QOS_NA_LEVEL = "ProductStorageRWVelocityNA";
const std::string MS_CFG_RW_QOS_EMPTY_LEVEL = "ProductStorageRWVelocityEmpty";
const std::string MS_CFG_RW_QOS_BUTT_LEVEL = "ProductStorageRWVelocityFaultTolerant";
const std::string MS_CFG_PRODUCT_RETRY_TIMES = "ProductStorageRetryTimes";
const std::string MS_CFG_PRODUCT_RETRY_INTERVAL = "ProductStorageRetryInterval";
const std::string MS_CFG_S3_RETRY_TIMES = "S3RetryTimes";
const std::string MS_CFG_S3_RETRY_TIMEOUT = "S3RetryTimeout";
const std::string MS_CFG_NODE_NORMAL_RETRY_TIMES    = "NodeNormalRetryTimes";
const std::string MS_CFG_NODE_NORMAL_RETRY_INTERVAL = "NodeNormalRetryInterval";

const std::string MS_CFG_DSWARE_CLEANER_MAX_TIME = "DSWareCleanerMaxTime";
const std::string MS_CFG_FORCE_FULLBACKUP_DISKID = "ForceFullBackupDiskID";
const std::string MS_CFG_FORCE_RESTORE = "ForceRestore";
const std::string MS_CFG_FORCE_RESTORE_DISKID = "ForceRestoreDiskID";
const std::string MS_CFG_DO_VERIFY_SNAP = "DoSnapVerify";

const std::string MS_CFG_DATABASE_SLEEPTIME = "DBOpRetrySleepTime";
const std::string MS_CFG_DATABASE_RETRY_TIMES = "DBOpRetrys";
const std::string MS_CFG_DATABASE_RETRY_TRANSACTION_TIMES = "DBTRansactionRetrys";

const std::string MS_CFG_V3_RETRY_TIMES = "V3OpRetryTimes";
const std::string MS_CFG_V3_RETRY_INTERVAL_TIME = "V3OpRetryIntervlTimeMillisecond";
const std::string MS_CFG_V3_MAX_CONCUR_CONN = "V3MaxConcurrencyConnection";

const std::string MS_CFG_AUTO_MEMORY_CHECK = "AutoMemoryCheck";
const std::string MS_CFG_COPY_SERVER_MEMORY_LIMIT = "CopyServiceMemoryLimit";

const std::string MS_CFG_RUN_SHELL_BY_BOOST = "RunShellByBoost";

const std::string MS_CFG_FAILED_BLOCK_RETRY_TIMES = "FailedBlockRetryTimes";
const std::string MS_CFG_FAILED_BLOCK_INTERVAL = "FailedBlockRetryInterval";
const std::string MS_CFG_FORCE_UPGRADE_SNAP_VERSION = "ForceUpgradeSnapVersion";

const std::string MS_CFG_REGISTER_NAME = "RegisterName";
const std::string MS_CFG_INDEX_NODE_MEMORY_MAX_SIZE = "IndexNodeMemoryMaxSize";
const std::string MS_CFG_INDEX_DOMAIN_MEMORY_MAX_SIZE = "IndexDomainMemoryMaxSize";
const std::string MS_CFG_DOMAIN_SIZE_REVISE_CYCLE = "DomainSizeReviseCycle";
const std::string MS_CFG_DEFRAG_QUERY_PASSIVE_TIMETOUT = "DefragQueryPassiveTimeout";
const std::string MS_CFG_VERIFY_SNAP_AFTER_BACKUP_COMPLETED = "VerifySnapAfterBackupCompleted";

const std::string MS_CFG_UPDATE_INTERVAL_FOR_REMOTE_RESTORE = "RemoteRestoreRecordUpdateInterval";
const std::size_t MS_CFG_DEFAULT_INTERVAL_FOR_REMOTE_RESTORE = 250;

const std::string MS_CFG_HTTP_RETRY_ERROR_CODES = "HttpStatusCodesForRetry";
const std::string MS_CFG_DORADO_NAS_RETRY_ERROR_CODES = "DoradoNasRetryErrorCodes";
const std::string MS_CFG_OCEANSTOR_NAS_RETRY_ERROR_CODES = "OceanstorNasRetryErrorCodes";

#ifdef WIN32
class AGENT_API ConfigReaderInterface
#else
class ConfigReaderInterface
#endif
{
public:
	// virtual ~ConfigReaderInterface() = 0;
};

#ifdef WIN32
class AGENT_API ConfigReader: public ConfigReaderInterface
#else
class ConfigReader: public ConfigReaderInterface
#endif
{
public:
    //Get int value of configuration item

    static int getInt(const std::string & sectionName, const std::string & keyName,bool logFlag = true);

    static unsigned int getUint(const std::string & sectionName, const std::string & keyName);

    //Get string value of configuration item
    static std::string getString(const std::string & sectionName, const std::string & keyName,bool logFlag=true);

    //Get management IP
    static std::string getManagementIP();

    //Get internal IP
    static std::string getInternalIP();

    //Get protected environment management IP
    static std::string getPEManagementIP();

    //Get protected environment storage IP
    static std::string getPEStorageIP();

    static std::string getBackupStorageIP();

    static std::string AdminNodeGetLeaderIP();

    static std::string getLoadbalanceIP();


    static std::set<int> getOceanStorDiskArrayNotRetryErrorCode();

    static std::string getAdminNodeIAMUser();
    static std::string getAdminNodeIAMPasswd();
    static std::string getBackupNodeIAMUser();
    static std::string getBackupNodeIAMPasswd();
    static std::string GetLeaderIP();
    static std::string GetPrimaryAndStandbyIP();
    static std::string GetEbkMgrBindPort();
    static std::string GetEbkAdminBindPort();

    static void getStringValueSet(
        const std::string& sectionName, const std::string& keyName, std::set<std::string>& t);


    //Call this function at the end of stopping service, after no more getXX is called
    static void destroy();

    static void setIntConfigInfo(const std::string & sectionName, const std::string & keyName, int minValue, int maxValue, int defaultValue);

    static void setStringConfigInfo(const std::string & sectionName, const std::string & keyName, const std::string & defaultValue);
    
    static void setPConfigInfo(const std::string & sectionName, const std::string & keyName, const std::string & defaultValue);

    static bool checkConfigInfo(const std::string & , const std::string & , const std::string & , bool &);

    static std::string getConfigVersion();

    static void getIntValueSet(const std::string& sectionName, const std::string& keyName, const std::string &delim, std::set<int>& t);
    static void getIntValueVector(
    const std::string& sectionName, const std::string& keyName, const std::string &delim, std::vector<int>& t);
    static int GetLogLevelFromK8s();
    static bool GetFileServerSslConfig();
    static void ReadLogLevelFromPM(bool readLoglevelFromPM);
    static void SetUpdateCnf(bool updateCnf);
private:
    static std::string getPlaneIP(const std::string & sectionName, const std::string & keyName);
};

} // namespace Module

#endif

