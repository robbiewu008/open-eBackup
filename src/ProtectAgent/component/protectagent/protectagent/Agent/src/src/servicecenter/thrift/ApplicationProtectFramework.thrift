namespace cpp AppProtect
namespace java com.huawei.oceanprotect.agent.rpc.thrift

include "ApplicationProtectBaseDataType.thrift"

typedef ApplicationProtectBaseDataType.ActionResult ActionResult
typedef ApplicationProtectBaseDataType.ApplicationPlugin ApplicationPlugin
typedef ApplicationProtectBaseDataType.Authentication Authentication
typedef ApplicationProtectBaseDataType.ApplicationResource ApplicationResource
typedef ApplicationProtectBaseDataType.Application Application
typedef ApplicationProtectBaseDataType.ApplicationEnvironment ApplicationEnvironment
typedef ApplicationProtectBaseDataType.SubJob SubJob
typedef ApplicationProtectBaseDataType.Copy Copy
typedef ApplicationProtectBaseDataType.CopyDataType CopyDataType
typedef ApplicationProtectBaseDataType.StorageRepository StorageRepository
typedef ApplicationProtectBaseDataType.BackupJobType BackupJobType
typedef ApplicationProtectBaseDataType.JobPermission JobPermission
typedef ApplicationProtectBaseDataType.ResourceResultByPage ResourceResultByPage
typedef ApplicationProtectBaseDataType.CmdResult CmdResult

exception AppProtectFrameworkException {
    1:required i32 code;
    2:optional string message;
}

/** resource scope */
enum ResourceScope {
    /** entier system valid */
    ENTIRE_SYSTEM = 0,
    /** single node valid */
    SINGLE_NODE = 1,
    /** single job valid */
    SINGLE_JOB = 2,
    /** production resources valid */
    PRODUCTION_RESOURCE = 3
}

/** job resource description, creating for multiple node using share resource */
struct Resource {
    /** resource scope */
    1:required ResourceScope scope,
    /** resource scope key for identifing different scope 
        scopeKey is 'system' when scope is equal to 'ENTIRE_SYSTEM'
        scopeKey is empty string when scope is equal to 'SINGLE_NODE'
        scopeKey is job id or sub job id when scope is equal to 'SINGLE_JOB'
        scopeKey is  production resources when scope is equal to 'PRODUCTION_RESOURCE'
    */
    2:required string scopeKey,
    /** resource key */
    3:required string resourceKey,
    /** resource value */
    4:required string resourceValue,
    /** if the resource is shared, how many nodes can be use */
    5:optional i32 sharedNum,
    /** lock type, read lock is "r", write lock is "w" */
    6:required string lockType
}

/** resource status */
struct ResourceStatus {
    /** resource information */
    1:required Resource resource,
    /** if the resource is locked, how many nodes have beed lock it */
    2:optional i32 lockNum
}

/** sub job status */
enum SubJobStatus {
    /** initilizing */
    INITIALIZING = 0,
    /** running */
    RUNNING = 1,
    /** aborting */
    ABORTING = 2,
    /** complete successfully */
    COMPLETED = 3,
    /** aborted */
    ABORTED = 4,
    /** abort failed. */
    ABORTED_FAILED = 5,
    /** failed */
    FAILED = 6,
    /** job failed, and it can not been retry */
    FAILED_NOTRY = 7,
    /** partial complete successfully */
    PARTIAL_COMPLETED = 13
}

/** job log level */
enum JobLogLevel {
    /** information log */
    TASK_LOG_INFO = 1,
    /** warning log */
    TASK_LOG_WARNING = 2,
    /** error log */
    TASK_LOG_ERROR = 3,
    /** serious warning log */
    TASK_LOG_SERIOUS_WARN = 4
}

/** job log detail */
struct LogDetail {
    /** jog level */
    1:required JobLogLevel level,
    /** log description label, which must be defined in system */
    2:required string description,
    /** log description parameters */
    3:optional list<string> params,
    /** log time stamp */
    4:required i64 timestamp,
    /** log errorcode, which must be defined in system */
    5:optional i64 errorCode,
    /** log errorcode parameters */
    6:optional list<string> errorParams,
    /** log additional information for detail description */
    7:optional list<string> additionalDesc
}

/** job checkPoint, if the job is need to redo, the checkpoint is required, but plugin need record the checkpoint when execuing job */
struct CheckPoint {
    /** checkpoint value */
    1:required string checkPoint
}

/** report job detail information */
struct SubJobDetails {
    /** main job id */
    1:required string jobId,
    /** sub job id, for subjob report detail */
    2:optional string subJobId,
    /** job status */
    3:optional SubJobStatus jobStatus,
    /** job additional status, define keyword status as follow:
        application_availabe : during instant restoration, when an application is available, the availability status needs to 
                               be reported to proxy framework
    */
    4:optional string additionalStatus,
    /** job log detail */
    5:optional list<LogDetail> logDetail,
    /** job progress */
    6:optional i32 progress,
    /** job have moved data size, unit KB */
    7:optional i64 dataSize,
    /** job move data speed, unit KB/s */
    8:optional i32 speed,
    /** job extend information */
    9:optional string extendInfo
}

/** prepare repository by Plugin */
struct PrepareRepositoryByPlugin {
    /** repository information */
    1:required list<StorageRepository> repository,
    /** repository permission*/
    2:required JobPermission permission,
    /** extend information */
    3:optional string extendInfo
}

/** alarm detail information */
struct AlarmDetails {
    /** alarm ID */
    1:required string alarmId;
    /** sequence id */
    2:required i64 sequence = 1;
    /** alarm type: invalid(0) communication(1) environment(2) device(3) business(4) operation(5) security(6) */
    3:required i32 alarmType = 4;
    /** source type: user,alarm,event,notify,resource,proctection,recovery,backup_cluster,network_entity */
    4:required string sourceType;
    /** alarm severity: 0:event 1:warning 2:minor 3:important 4:critical */
    5:required i32 severity = 1;
    /** alarm parameter */
    6:required string parameter;
    /** resource ID */
    7:required string resourceId;

}

/** 
 share resource service, the service name is struct name (ShareResource) with TMultiplexedProcessor.registerProcessor 
 plugin -> application protect system
*/
service ShareResource {
    /** 
        Function description
            create new resource for job 
        Parameters
            Resource : resource object
            mainJobId : mainJob id
        Return value
            successful if the ActionResult.code is 0, failed otherwise
    */
    ActionResult CreateResource(1:Resource resource, 2:string mainJobId);

    /** 
        Function description
            query job information 
        Parameters
            resource : resource object
            mainJobId : mainJob id
        Return value
            the ResourceStatus in application protect system, which will throw AppProtectFrameworkException if query failed
    */
    ResourceStatus QueryResource(1:Resource resource, 2:string mainJobId) throws(1:AppProtectFrameworkException e);

    /** 
        Function description
            update job valueb 
        Parameters
            resource : resource object
            mainJobId : mainJob id
        Return value
            successful if the ActionResult.code is 0, failed otherwise
    */
    ActionResult UpdateResource(1:Resource resource, 2:string mainJobId);

    /** 
        Function description
            delete job
        Parameters
            resource : resource object
            mainJobId : mainJob id
        Return value
            successful if the ActionResult.code is 0, failed otherwise
    */
    ActionResult DeleteResource(1:Resource resource, 2:string mainJobId);

    /** 
        Function description
            lock the job
        Parameters
            resource : resource object
            mainJobId : mainJob id
        Return value
            successful if the ActionResult.code is 0, failed otherwise
    */
    ActionResult LockResource(1:Resource resource, 2:string mainJobId);

    /** 
        Function description
            unlock the job 
        Parameters
            resource : resource object
            mainJobId : mainJob id
        Return value
            successful if the ActionResult.code is 0, failed otherwise
    */
    ActionResult UnLockResource(1:Resource resource, 2:string mainJobId);

    /** 
        Function description
            lock the job essource
        Parameters
            resource : resource object
            mainJobId : mainJob id
        Return value
            successful if the ActionResult.code is 0, failed otherwise
    */
    ActionResult LockJobResource(1:Resource resource, 2:string mainJobId);
}

/** 
 platform job  service, the service name is struct name (JobService) with TMultiplexedProcessor.registerProcessor 
 plugin -> application protect system
*/
service JobService {
    /** 
        Function description
            when generating job, the interface can be used to create new sub job, 
            successful if job name is exists, the new job will not created otherwise
        Parameters
            jobs : new job list
        Return value
            successful if the ActionResult.code is 0, failed otherwise
    */
    ActionResult AddNewJob(1:list<SubJob> jobs);

    /** 
        Function description
            report job information to platform, then the job will be display in GUI, 
            the JobDetails.logDetail only contain latest log details from last reporting, and reporting period is [30s, 300s]
        Parameters
            jobInfo : report information
        Return value
            successful if the ActionResult.code is 0, failed otherwise
    */
    ActionResult ReportJobDetails(1:SubJobDetails jobInfo);

    /** 
        Function description
            report data copy additional information after backup complete, .eg after log backup, report log directory name
        Parameters
            jobId : main job id
            copy : update copy information, Copy.name, Cope.timestamp, Copy.transactionNo, Copy.repositories.remotePath are supported to modify,
                  which is used as follow scenaio,
                  1. when protecting HUAWEI NAS, plugin should report HUAWEI NAS snapshot name, copy.name will store extenal snapshow name
                  2. when backup application log, plugin should report new log directory, copy.repositories.remotePath will store the new log directory name, 
                    copy.extendInfo will store the new log range, including time and application transaction sn .etc
        Return value
            successful if the ActionResult.code is 0, failed otherwise
    */
    ActionResult ReportCopyAdditionalInfo(1:string jobId, 2:Copy copy);

    /** 
        Function description
            when protect an appliation using multiple filesystem, every file have fix location in multiple file system
        Parameters
            files : computer file list
            filesystems : multiple file system list
        Return value
            the computer result, which will throw AppProtectFrameworkException if computer failed
    */
    map<string, string> ComputerFileLocationInMultiFileSystem(1:list<string> files, 2:list<string> fileSystems) throws(1:AppProtectFrameworkException e);

    /** 
        Function description
            Query the previous most recent copy from copy data type and specified copy
        Parameters
            application : protect application information
            types : previous backup copy data type
            copyId : specified copy id, it's optional
            mainJobId : mainJob id
        Return value
            the most recent copy from copy data type and parameter copy, which will throw AppProtectFrameworkException when no found
    */
    Copy QueryPreviousCopy(1:Application application, 2:set<CopyDataType> types, 3:string copyId, 4:string mainJobId) throws(1:AppProtectFrameworkException e);

    /**
        Function description
            Plugin send msg to pm to update async list task results
        Parameters
            jobId : main job id
            code : return value code
            results : ResourceResultByPage
        Return value
            validity if the ActionResult.code is 0, invalidity otherwise
    */
    ActionResult ReportAsyncJobDetails(1:string jobId, 2:i32 code, 3:ResourceResultByPage results);

    /**
        Function description
            Plugin call this interface to mount Repository, synchronization function
        Parameters
            PrepareRepositoryByPlugin : mount filesystem parameter
        Return value
            return Mount Repository result
    */
    ActionResult MountRepositoryByPlugin(1:PrepareRepositoryByPlugin mountinfo);

    /**
        Function description
            Plugin call this interface to unmount Repository, synchronization function
        Parameters
            PrepareRepositoryByPlugin : unmount repository parameter
        Return value
            return unMount repository result
    */
    ActionResult UnMountRepositoryByPlugin(1:PrepareRepositoryByPlugin mountinfo);

    /**
        Function description
            Plugin send alram
        Parameters
            AlarmDetails - alarm details
        Return value
            validity if the ActionResult.code is 0, invalidity otherwise
    */
    ActionResult SendAlarm(1:AlarmDetails alarm);

    /**
        Function description
            Plugin clear alram
        Parameters
            AlarmDetails - alarm details
        Return value
            validity if the ActionResult.code is 0, invalidity otherwise
    */
    ActionResult ClearAlarm(1:AlarmDetails alarm);

    /**
        Function description
            Plugin send msg to ubc to add ip white list
        Parameters
            jobId : main job id
            ipListStr : ip list string
        Return value
            validity if the ActionResult.code is 0, invalidity otherwise
    */
    ActionResult AddIpWhiteList(1:string jobId, 2:string ipListStr);

    /**
        Function description
            Plugin send msg to PM to get lasted hcs token
        Parameters
            projectId : HCS project id
            isWorkSpace: 0:HCS ecs, 1:workspace ecs
        Return value
            new hcs token will save in ApplicationEnvironment's extendInfo
    */
    ApplicationEnvironment GetHcsToken(1:string projectId, 2:string isWorkSpace);
}

/** 
 plugin service, the service name is struct name (RegisterPluginService) with TMultiplexedProcessor.registerProcessor 
 plugin -> application protect system
*/
service RegisterPluginService {
    /** 
        Function description
            when plugin starting, the plugin need call the interface for register to platform 
        Parameters
            plugin : application plugin object
        Return value
            successful if the ActionResult.code is 0, failed otherwise
    */
    ActionResult RegisterPlugin(1:ApplicationPlugin plugin);

    /** 
        Function description
            when plugin is uninstalled, the plugin need call the interface for unregister from platform 
        Parameters
            plugin : application plugin object
        Return value
            successful if the ActionResult.code is 0, failed otherwise
    */
    ActionResult UnRegisterPlugin(1:ApplicationPlugin plugin);
}

/** 
 security service, the service name is struct name (SecurityService) with TMultiplexedProcessor.registerProcessor 
 plugin -> application protect system
*/
service SecurityService {
    /** 
        Function description
            when plugin need check Certificate thumbprint info validity call the interface
        Parameters
            ip : cert service ip information
            port: cert service port information
            thumbprint: need check certificate thumbprint information
        Return value
            validity if the ActionResult.code is 0, invalidity otherwise
    */
    ActionResult CheckCertThumbPrint(1:string ip, 2:i32 port, 3:string thumbPrint);

    /** 
        Function description
            when plugin need run commmand call the interface
        Parameters
            cmdPara : command parameters
            cmdOutput: output of command
        Return value
            validity if the ActionResult.code is 0, invalidity otherwise
    */
    CmdResult RunCommand(1:string cmdPara);
}

/** 
 framework service, the service name is struct name (FrameworkService) with TMultiplexedProcessor.registerProcessor 
 plugin -> application protect system
*/
service FrameworkService {
    /** 
        Function description
            Plugin check whether the Agent framework is normal
        Parameters
        Return value
            validity if the ActionResult.code is 0, invalidity otherwise
    */
    ActionResult HeartBeat();
}
