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
typedef ApplicationProtectBaseDataType.StorageRepository StorageRepository
typedef ApplicationProtectBaseDataType.BackupJobType BackupJobType
typedef ApplicationProtectBaseDataType.QueryByPage QueryByPage
typedef ApplicationProtectBaseDataType.JobPermission JobPermission

exception AppProtectPluginException {
    1:required i32 code;
    2:required list<string> codeParams;
    3:optional string message;
}

/** job qos limit */
struct Qos {
    /** qos limit speed, unit megabyte */
    1:optional i32 bandwidth,
    /** qos limit iops with protect environment */
    2:optional i32 protectIops,
    /** qos limit iops with backup storage */
    3:optional i32 backupIops
}

struct ResourceFilter {
    /** filter id, filter by name, ID, Formate, ModifyTime, or CreateTime */
    1:required string filterBy,
    /** filter type */
    2:required string type,
    /** filter rule, fuzzy match or exact match and so on */
    3:required string rule,
    /** filter mode, INCLUDE or EXCLUDE */
    4:required string mode,
    /** filter values */
    5:required list<string> values
}

struct DataLayout {
    /** whether encryption */
    1:required bool encryption,
    /** whether deduption */
    2:required bool deduption,
    /** whether data compression */
    3:required bool compression,
    /** whether backup data is native backup mode */
    4:required bool nativeData,
    /** data layout extend information, json string format, .eg encryption algorithm */
    5:optional string extendInfo
}

struct JobScripts {
    /** pre-processing script, which will be called before executing job */
    1:required string preScript,
    /** post-processing script, which will be called after executing job successfully */
    2:required string postScript,
    /** post-processing script, which will be called after executing job failed */
    3:required string failPostScript,
}

/** backup job parameters */
struct BackupJobParam {
    /** backup job type */
    1:required BackupJobType backupType,
    /** backup resource filter */
    2:optional list<ResourceFilter> filters,
    /** backup advance param */
    3:required DataLayout dataLayout,
    /** backup qos limit */
    4:optional Qos qos,
    /** backup script configuration */
    5:optional JobScripts scripts,
    /** backup advance parameters, json string format */
    6:optional string advanceParams
}

/** backup job description */
struct BackupJob {
    /** current requestid in system, using write log for fix issue */
    1:required string requestId,
    /** current main job id */
    2:required string jobId,
    /** backup job parameter*/
    3:required BackupJobParam jobParam,
    /** backup protect environment */
    4:required ApplicationEnvironment protectEnv,
    /** backup protect application */
    5:required Application protectObject,
    /** backup protect application resource*/
    6:optional list<ApplicationResource> protectSubObject,
    /** backup storage respository information, including meta data, data, log, cache */
    7:required list<StorageRepository> repositories,
    /** the copy information to be created */
    8:required Copy copy,
    /** Job extend information, json string format */
    9:optional string extendInfo
}

/** restore job type */
enum RestoreJobType {
    /** restore byte-for-byte */
    NORMAL_RESTORE = 1,
    /** instant restore */
    INSTANT_RESTORE = 2,
    /** fine grained restore */
    FINE_GRAINED_RESTORE = 3
}

/** restore job parameters */
struct RestoreJobParam {
    /** restore type */
    1:required RestoreJobType restoreType,
    /** restore mode,
        RemoteRestore : proxy restore data from archive microservice archive
        LocalRestore  : proxy restore data from mounted file system
    */
    2:required string restoreMode,
    /** restore resource filter */
    3:optional list<ResourceFilter> filters,
    /** restore qos limit */
    4:optional Qos qos,
    /** restore script configuration */
    5:optional JobScripts scripts,
    /** restore advance parameters, json string format, .eg restore by timestamp and restore to exact timestamp */
    6:optional string advanceParams
}

/** restore job description */
struct RestoreJob {
    /** current requestid in system, using write log for fix issue */
    1:required string requestId,
    /** current main job id */
    2:required string jobId,
    /** restore job parameter */
    3:required RestoreJobParam jobParam,
    /** restore target environment */
    4:required ApplicationEnvironment targetEnv,
    /** restore target application */
    5:required Application targetObject,
    /** restore target application resource, it can be used in fine grained restore */
    6:optional list<ApplicationResource> restoreSubObjects,
    /** copy is used for restoring */
    7:required list<Copy> copies,
    /** Job extend information, json string format */
    8:optional string extendInfo
}

/** livemount job parameters */
struct LivemountJobParam {
    /** livemount script configuration */
    1:optional JobScripts scripts,
    /** mount advance parameters */
    2:optional string advanceParams
}

/** cancel livemount job parameters */
struct CancelLivemountJobParam {
    /** livemount script configuration */
    1:optional JobScripts scripts,
    /** mount advance parameters */
    2:optional string advanceParams
}

/** livemount job description */
struct LivemountJob {
    /** current requestid in system, using write log for fix issue */
    1:required string requestId,
    /** current main job id */
    2:required string jobId,
    /** mount job parameter */
    3:optional LivemountJobParam jobParam,
    /** mount target environment */
    4:required ApplicationEnvironment targetEnv,
    /** mount target application */
    5:required Application targetObject,
    /** mount target application resource */
    6:optional list<ApplicationResource> targetSubObjects,
    /** copy is used for restoring */
    7:required Copy copy,
    /** Job extend information, json string format */
    8:optional string extendInfo
}

/** cancel livemount job description */
struct CancelLivemountJob {
    /** current requestid in system, using write log for fix issue */
    1:required string requestId,
    /** current main job id */
    2:required string jobId,
    /** cancel livemount job parameter */
    3:optional CancelLivemountJobParam jobParam,
    /** cancel livemount target environment */
    4:required ApplicationEnvironment targetEnv,
    /** cancel livemount target application */
    5:required Application targetObject,
    /** cancel livemount target application resource */
    6:optional list<ApplicationResource> targetSubObjects,
    /** copy is used for restoring */
    7:required Copy copy,
    /** Job extend information, json string format */
    8:optional string extendInfo
}

/** build index job parameters */
struct BuildIndexJobParam {
    /** previous copy id */
    1:required string preCopyId,
    /** build index metadata path */
    2:required string indexPath
}

/** build index job description */
struct BuildIndexJob {
    /** current requestid in system, using write log for fix issue */
    1:required string requestId,
    /** current main job id */
    2:required string jobId,
    /** build index job parameter */
    3:optional BuildIndexJobParam jobParam,
    /** environment for building index */
    4:required ApplicationEnvironment indexEnv,
    /** application for building index */
    5:required Application indexProtectObject,
    /** application resource for building index */
    6:optional list<ApplicationResource> indexProtectSubObject,
    /** build index respository list, including meta data, data, cache */
    7:required list<StorageRepository> repositories,
    /** copy list is used for restoring */
    8:required list<Copy> copies,
    /** Job extend information, json string format */
    9:optional string extendInfo
}

/** Delete copy job description */
struct DelCopyJob {
    /** current requestid in system, using write log for fix issue */
    1:required string requestId,
    /** current main job id */
    2:required string jobId,
    /** environment for deleting copy */
    3:required ApplicationEnvironment protectEnv,
    /** application for deleting copy */
    4:required Application protectObject,
    /** application resource for deleting copy */
    5:required list<StorageRepository> repositories,
    /** copy list is used for deleting */
    6:required list<Copy> copies,
    /** Job extend information, json string format */
    7:optional string extendInfo
}

/** Check copy job description */
struct CheckCopyJob {
    /** current requestid in system, using write log for fix issue */
    1:required string requestId,
    /** current main job id */
    2:required string jobId,
    /** environment for checking copy */
    3:required ApplicationEnvironment protectEnv,
    /** application for checking copy */
    4:required Application protectObject,
    /** application resource for checking copy */
    5:required list<StorageRepository> repositories,
    /** copy list is used for checking */
    6:required list<Copy> copies,
    /** Job extend information, json string format */
    7:optional string extendInfo
}

/** executing backup node limit */
enum BackupLimit {
    /** any cluste node can be execute */
    NO_LIMIT = 0,
    /** only master node can be execute */
    ONLY_MASTER = 1,
    /** only slave node can be execute */
    ONLY_SLAVE = 2,
    /** backup on master node first */
    FIRST_MASTER = 3,
    /** backup on slave node first */
    FIRST_SLAVE = 4
}

/** jobs results */
enum JobResult {
    /** all previous jobs are successfully executed */
    SUCCESS = 0,
    /** some previous jobs fail to be executed */
    FAILED = 1,
    /** some previous jobs are aborted */
    ABORTED = 2
}

/** list resource result with page */
struct ResourceResultByPage {
    /** resource elements list */
    1:required list<ApplicationResource> items,
    /** current page no */
    2:required i32 pageNo,
    /** maximum number of elements in one page */
    3:required i32 pageSize,
    /** total page number */
    4:required i32 pages,
    /** total elements number */
    5:required i32 total
}

/** list resource by page request */
struct ListResourceRequest {
    /** environment for list resource */
    1:required ApplicationEnvironment appEnv,
    /** parent application resource for list resource */
    2:optional list<Application> applications,
    /** condition for list resource */
    3:required QueryByPage condition
}

/** list resource by page request */
struct OracleDBInfo {
    /** oracle software user */
    1:required string oracleUser,
    /** oracle db name */
    2:required string dbName,
    /** oracle db instance name */
    3:required string instanceName,
    /** oracle db user */
    4:optional string dbUser,
    /** oracle db password */
    5:optional string dbPassword,
    /** grid software user */
    6:optional string gridUser,
    /** oracle asm instance name */
    7:optional string asmName,
    /** oracle asm user */
    8:optional string asmUser,
    /** oracle asm password */
    9:optional string asmPassword,
    /** oracle archive space usage threshold */
    10:optional string archThreshold,
    /**  */
    11:optional string runUserPwd,
    /**  */
    12:optional string accessOracleHome,
    /**  */
    13:optional string accessOracleBase
}

service PluginServiceBase {

}

/** 
 protect platform service, the service name is struct name (ProtectService) with TMultiplexedProcessor.registerProcessor 
 application protect system -> plugin 
*/
service ProtectService extends PluginServiceBase {
    /** 
        Function description
            abort running job, asynchronization function, aborting result will be reported by interface 'JobService.ReportJobDetails'
        Parameters
            jobId : main job id
            subJobId : abort sub job id, which is emtpy string when abort prerequisite, generate job
            apptype : job application type
        Return value
            successful if the ActionResult.code is 0, failed otherwise
    */
    ActionResult AsyncAbortJob(1:string jobId, 2:string subJobId, 3:string appType);

    /** 
        Function description
            pause running job, synchronization function, plugin need record cache data for next executing
        Parameters
            jobId : main job id
            subJobId : pause sub job id, which is emtpy string when abort prerequisite, generate job
            apptype : job application type
        Return value
            successful if the ActionResult.code is 0, failed otherwise
    */
    ActionResult PauseJob(1:string jobId, 2:string subJobId, 3:string appType);

    /** 
        Function description
            check the backup job type, synchronization function, .eg whether the backup job need to be covert from incremental to full
        Parameters
            job : backup job information
        Return value
            successful if the ActionResult.code is 0, failed otherwise
    */
    ActionResult CheckBackupJobType(1:BackupJob job);

    /** 
        Function description
            check the backup job whether can be executed in this node, scene as follow
                This interface is used in a cluster or a system which has master and slave node. In these systems, backup activity can 
                only be executed in master node. Recover activity doesn't have to consider this scene.
        Parameters
            job : backup job information
            limit : backup node policy
        Return value
            successful if the ActionResult.code is 0, failed otherwise
    */
    ActionResult AllowBackupInLocalNode(1:BackupJob job, 2:BackupLimit limit);

    /** 
        Function description
            check the backup sub job whether can be executed in this node
        Parameters
            job : backup job information
            subJob : sub job information, it's created in generating step
        Return value
            successful if the ActionResult.code is 0, failed otherwise
    */
    ActionResult AllowBackupSubJobInLocalNode(1:BackupJob job, 2:SubJob subJob);

    /** 
        Function description
            query job permission, synchronization function
        Parameters
            appEnv : protect environment information
            application : protect application information
        Return value
            return job permission, return empty if the nas share does not need to be modified 
    */
    JobPermission QueryJobPermission(1:ApplicationEnvironment appEnv, 2:Application application);

    /** 
        Function description
            execute backup prerequisite job, asynchronization and idempotency function 
        Parameters
            job : backup job information
        Return value
            successful if the ActionResult.code is 0, failed otherwise
    */
    ActionResult AsyncBackupPrerequisite(1:BackupJob job);

    /** 
        Function description
            execute backup generate job, asynchronization function and idempotency function, reporting job detail is required
        Parameters
            job : backup job information
            nodeNum : the proxy node number of executing sub job, the number of sub jobs to be generated can refer to this value.
        Return value
            successful if the ActionResult.code is 0, failed otherwise
    */
    ActionResult AsyncBackupGenerateSubJob(1:BackupJob job, 2:i32 nodeNum);

    /** 
        Function description
            execute backup job, asynchronization function and idempotency function, reporting job detail with jobId=BackupJob.jobId 
            and subJobId=SubJob.subJobId
        Parameters
            job : backup job information
            subJob : sub job information, it's created in generating step
        Return value
            successful if the ActionResult.code is 0, failed otherwise
    */
    ActionResult AsyncExecuteBackupSubJob(1:BackupJob job, 2:SubJob subJob);

    /** 
        Function description
            execute backup post job, asynchronization function and idempotency function
        Parameters
            job : backup job information
            subJob : sub job information
            backupJobResult: backup job result, according this result, the plugin can execute normal post job or failed post job 
        Return value
            successful if the ActionResult.code is 0, failed otherwise
    */
    ActionResult AsyncBackupPostJob(1:BackupJob job, 2:SubJob subJob, 3:JobResult backupJobResult);

    /** 
        Function description
            check the restore job whether can be executed in this node.
        Parameters
            job : backup job information
        Return value
            successful if the ActionResult.code is 0, failed otherwise
    */
    ActionResult AllowRestoreInLocalNode(1:RestoreJob job);

    /** 
        Function description
            check the restore sub job whether can be executed in this node.
        Parameters
            job : backup job information
            subJob : sub job information
        Return value
            successful if the ActionResult.code is 0, failed otherwise
    */
    ActionResult AllowRestoreSubJobInLocalNode(1:RestoreJob job, 2:SubJob subJob);

    /** 
        Function description
            execute restore prerequisite job, asynchronization function and idempotency function
        Parameters
            job : restore job information
        Return value
            successful if the ActionResult.code is 0, failed otherwise
    */
    ActionResult AsyncRestorePrerequisite(1:RestoreJob job);

    /** 
        Function description
            execute restore generate job, asynchronization function and idempotency function, reporting job detail is required
        Parameters
            job : restore job information
            nodeNum : the proxy node number of executing sub job, the number of sub jobs to be generated can refer to this value.
        Return value
            successful if the ActionResult.code is 0, failed otherwise
    */
    ActionResult AsyncRestoreGenerateSubJob(1:RestoreJob job, 2:i32 nodeNum);

    /** 
        Function description
            execute restore job, asynchronization function and idempotency function, reporting job detail with jobId=BackupJob.jobId 
            and subJobId=SubJob.subJobId
        Parameters
            job : restore job information
            subJob: sub job information, it's created in generating step
        Return value
            successful if the ActionResult.code is 0, failed otherwise
    */
    ActionResult AsyncExecuteRestoreSubJob(1:RestoreJob job, 2:SubJob subJob);

    /** 
        Function description
            execute restore post job, asynchronization function and idempotency function
        Parameters
            job : restore job information
            subJob : sub job information
            restoreJobResult: restore job result, according this result, the plugin can execute normal post job or failed post job
        Return value
            successful if the ActionResult.code is 0, failed otherwise
    */
    ActionResult AsyncRestorePostJob(1:RestoreJob job, 2:SubJob subJob, 3:JobResult restoreJobResult);

    /** 
        Function description
            execute livemount generate job, asynchronization function and idempotency function, reporting job detail is required, 
            .eg livemount database cluster, plugin should generate the same number of sub jobs as the number of nodes.
        Parameters
            job : restore job information
            nodeNum : the proxy node number of executing sub job, the number of sub jobs to be generated can refer to this value.
        Return value
            successful if the ActionResult.code is 0, failed otherwise
    */
    ActionResult AsyncLivemountGenerateSubJob(1:LivemountJob job, 2:i32 nodeNum);
    
    /** 
        Function description
            execute livemount job, asynchronization function and idempotency function, reporting job detail with jobId=BackupJob.jobId 
            and subJobId=SubJob.subJobId
        Parameters
            job : restore job information
            subJob : sub job information, it's created in generating step
        Return value
            successful if the ActionResult.code is 0, failed otherwise
    */
    ActionResult AsyncExecuteLivemountSubJob(1:LivemountJob job, 2:SubJob subJob);

    /** 
        Function description
            generate cancel livemount sub job, asynchronization function and idempotency function, reporting job detail is required, 
            .eg cancel livemount database cluster, plugin should generate the same number of sub jobs as the number of nodes.
        Parameters
            job : restore job information
            nodeNum : the proxy node number of executing sub job, the number of sub jobs to be generated can refer to this value.
        Return value
            successful if the ActionResult.code is 0, failed otherwise
    */
    ActionResult AsyncCancelLivemountGenerateSubJob(1:CancelLivemountJob job, 2:i32 nodeNum);
    
    /** 
        Function description
            execute cancel livemount sub job, asynchronization function and idempotency function, reporting job detail with jobId=BackupJob.jobId 
            and subJobId=SubJob.subJobId
        Parameters
            job : restore job information
            subJob : sub job information, it's created in generating step
        Return value
            successful if the ActionResult.code is 0, failed otherwise
    */
    ActionResult AsyncExecuteCancelLivemountSubJob(1:CancelLivemountJob job, 2:SubJob subJob);
    
    /** 
        Function description
            execute instant restore prerequisite job, asynchronization function and idempotency function 
        Parameters
            job : restore job information
        Return value
            successful if the ActionResult.code is 0, failed otherwise
    */
    ActionResult AsyncInstantRestorePrerequisite(1:RestoreJob job);

    /** 
        Function description
            execute instant restore generate job, asynchronization function and idempotency function, reporting job detail is required
        Parameters
            job : restore job information
            nodeNum : the proxy node number of executing sub job, the number of sub jobs to be generated can refer to this value.
        Return value
            successful if the ActionResult.code is 0, failed otherwise
    */
    ActionResult AsyncInstantRestoreGenerateSubJob(1:RestoreJob job, 2:i32 nodeNum);

    /** 
        Function description
            execute instant restore job, asynchronization function and idempotency function, reporting job detail with jobId=BackupJob.jobId 
            and subJobId=SubJob.subJobId, .eg instant restore database cluster, plugin should generate the same number of sub jobs as 
            the number of nodes.
        Parameters
            job : restore job information
            subJob : sub job information, it's created in generating step
        Return value
            successful if the ActionResult.code is 0, failed otherwise
    */
    ActionResult AsyncExecuteInstantRestoreSubJob(1:RestoreJob job, 2:SubJob subJob);

    /** 
        Function description
            execute instant restore post job, asynchronization function and idempotency function
        Parameters
            job : restore job information
            subJob : sub job information
            restoreJobResult : restore job result, according this result, the plugin can execute normal post job or failed post job
        Return value
            successful if the ActionResult.code is 0, failed otherwise
    */
    ActionResult AsyncInstantRestorePostJob(1:RestoreJob job, 2:SubJob subJob, 3:JobResult restoreJobResult);
    
    /** 
        Function description
            execute building index generate job, asynchronization function and idempotency function, reporting job detail is required
        Parameters
            job: build index job information
            nodeNum : the proxy node number of executing sub job, the number of sub jobs to be generated can refer to this value.
        Return value
            successful if the ActionResult.code is 0, failed otherwise
    */
    ActionResult AsyncBuildIndexGenerateSubJob(1:BuildIndexJob job, 2:i32 nodeNum);

    /** 
        Function description
            execute building index job, asynchronization function and idempotency function
        Parameters
            job : build index job information
            subJob : sub job information, it's created in generating step
        Return value
            successful if the ActionResult.code is 0, failed otherwise
    */
    ActionResult AsyncBuildIndexSubJob(1:BuildIndexJob job, 2:SubJob subJob);

    /** 
        Function description
            execute delete copy job, asynchronization function and idempotency function, reporting job detail is required
        Parameters
            job: delete job information
            nodeNum : the proxy node number of executing sub job, the number of sub jobs to be generated can refer to this value.
        Return value
            successful if the ActionResult.code is 0, failed otherwise
    */
    ActionResult AsyncDelCopyGenerateSubJob(1:DelCopyJob job, 2:i32 nodeNum);

    /** 
        Function description
            execute delete copy job, asynchronization function and idempotency function
        Parameters
            job : delete job information
            subJob : sub job information, it's created in generating step
        Return value
            successful if the ActionResult.code is 0, failed otherwise
    */
    ActionResult AsyncDelCopySubJob(1:DelCopyJob job, 2:SubJob subJob);

    /** 
        Function description
            execute checking copy job, asynchronization function and idempotency function, reporting job detail is required
        Parameters
            job: check job information
            nodeNum : the proxy node number of executing sub job, the number of sub jobs to be generated can refer to this value.
        Return value
            successful if the ActionResult.code is 0, failed otherwise
    */
    ActionResult AsyncCheckCopyGenerateSubJob(1:CheckCopyJob job, 2:i32 nodeNum);

    /** 
        Function description
            execute check copy job, asynchronization function and idempotency function
        Parameters
            job : check job information
            subJob : sub job information, it's created in generating step
        Return value
            successful if the ActionResult.code is 0, failed otherwise
    */
    ActionResult AsyncCheckCopySubJob(1:CheckCopyJob job, 2:SubJob subJob);

    /** 
        Function description
            deliver task status to plugin
        Parameters
            status : status of the task delivered to the plugin
            jobId : main job id
            script : script path
        Return value
            successful if the ActionResult.code is 0, failed otherwise
    */
    ActionResult DeliverTaskStatus(1:string status, 2:string jobId, 3:string script);

    /**
        Function description
            check the check copy job whether can be executed in this node.
        Parameters
            job : check copy job information
        Return value
            successful if the ActionResult.code is 0, failed otherwise
    */
    ActionResult AllowCheckCopyInLocalNode(1:CheckCopyJob job);
 
    /**
        Function description
            check the check job sub job whether can be executed in this node.
        Parameters
            job : check job information
            subJob : sub job information
        Return value
            successful if the ActionResult.code is 0, failed otherwise
    */
    ActionResult AllowCheckCopySubJobInLocalNode(1:CheckCopyJob job, 2:SubJob subJob);
}

/** 
 application service, the service name is struct name (ApplicationService) with TMultiplexedProcessor.registerProcessor 
 application protect system -> plugin 
*/
service ApplicationService extends PluginServiceBase {
    /** 
        Function description
            query application list by one apptype in the host, no Authentication, synchronization function, .eg query oracle database
        Parameters
            appType : query application type
        Return value
            application list in the host, list is empty when no application exists
    */
    list<Application> DiscoverApplications(1:string appType);

    /** 
        Function description
            query host cluster information
        Parameters
            appEnv : application environment information which is cluster member
        Return value
            remote host list, list is empty when no application exists
    */
    ApplicationEnvironment DiscoverHostCluster(1:ApplicationEnvironment appEnv) throws(1:AppProtectPluginException e);

    /** 
        Function description
            query application cluster information
        Parameters
            appEnv : application environment information which is cluster member
            application : application information
        Return value
            remote host list, list is empty when no application exists
    */
    ApplicationEnvironment DiscoverAppCluster(1:ApplicationEnvironment appEnv, 2:Application application) throws(1:AppProtectPluginException e);
    
    /** 
        Function description
            check application exist with Authentication, synchronization function
        Parameters
            appEnv : protect application environment
            application : protect application information
        Return value
            successful if the ActionResult.code is 0, failed otherwise
    */
    ActionResult CheckApplication(1:ApplicationEnvironment appEnv, 2:Application application);

    /** 
        Function description
            after finishing the job, clear the data which is not used
        Parameters
            appEnv : protect application environment
            application : check application information
            extendInfo : information needed for future use
        Return value
            successful if the ActionResult.code is 0, failed otherwise
    */
    ActionResult FinalizeClear(1:ApplicationEnvironment appEnv, 2:Application application, 3:map<string, string> extendInfo) throws(1:AppProtectPluginException e);
  
    /** 
        Function description
            list appliation resource, synchronization function, .eg query data file of oracle database
        Parameters
            appEnv : protect application environment
            application : check application information
            parentResource : query child resource of the parentResource
        Return value
            application resource list in the host, list is empty when no application resource exists
    */
    list<ApplicationResource> ListApplicationResource(1:ApplicationEnvironment appEnv, 2:Application application, 3:ApplicationResource parentResource) throws(1:AppProtectPluginException e);

    /** 
        Function description
            list appliation resource by page, synchronization function, .eg query data file of database
        Parameters
            request : list resource request
        Return value
            application resource list in the host with one page, list is empty when no application resource exists
    */
    ResourceResultByPage ListApplicationResourceV2(1:ListResourceRequest request) throws(1:AppProtectPluginException e);

    /** 
        Function description
            list appliation config
        Parameters
            script : Script information.
                     If this parameter is left blank, all configurations are queried.
                     If the value is not empty, the configuration of the corresponding application is queried.
        Return value
            application config list in the host, list is empty when no application config exists
    */
    map<string, string> ListApplicationConfig(1:string script) throws(1:AppProtectPluginException e);

    /** 
        Function description
            For Oracle services, check the archive area of the database.
        Parameters
            appType : query application type
            dbInfoList : oracle db info list
        Return value
            successful if the ActionResult.code is 0
            failed ActionResult.message=dbname1&dbname2,80
    */
    ActionResult OracleCheckArchiveArea(1:string appType, 2:list<OracleDBInfo> dbInfoList);
    /**
        Function description
            When remove resource protect policy, application would do something
        Parameters
            appEnv : protect application environment
            application : check application information
        Return value
            successful if the ActionResult.code is 0, failed otherwise
    */
    ActionResult RemoveProtect(1:ApplicationEnvironment appEnv, 2:Application application) throws(1:AppProtectPluginException e);
}

/** 
 plugin service, the service name is struct name (PluginService) with TMultiplexedProcessor.registerProcessor 
 application protect system -> plugin 
*/
service PluginService extends PluginServiceBase {
    /** 
        Function description
            query the application plugin information
        Parameters
            none
        Return value
            application plugin information, which will throw AppProtectPluginException if query failed
    */
    ApplicationPlugin QueryPlugin() throws(1:AppProtectPluginException e);
}

