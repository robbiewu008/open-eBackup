/** return action result */
struct ActionResult
{
    /** execute result
        0   - success               : plugin or proxy framework executes successfully
        100 - continue              : plugin or proxy framework executes successfully, client need to perform this operation continue
        101 - busy                  : plugin or proxy framework is busy, client should perform this operation after a period of time
        200 - internal error        : an internal error occurred in the plugin or proxy framework
     */
    1:required i32 code,
    /** business error code, this error code is need to applied in OMRP */
    2:optional i64 bodyErr,
    /** return result message, message maybe contain failed result */
    3:optional string message
    /** business error parameter */
    4:optional list<string> bodyErrParams
}

/** application plugin struct */
struct ApplicationPlugin {
    /** plugin name */
    1:required string name,
    /** plugin rpc service ip address */
    2:required string endPoint,
    /** plugin rpc service ip port */
    3:required i32 port,
    /** plugin running main process */
    4:required string processId
}

/** authentication type */
enum AuthType {
    /** no authentication */
    NO_AUTHENTICATION = 0,
    /** authentication by local os account */
    OS_PASSWORD = 1,
    /** authentication by application account */
    APP_PASSWORD = 2,
    /** authentication by ladp account */
    LADP = 3,
    /** authentication by ak sk */
    AKSK = 4,
    /** authentication by kerberos */
    KERBEROS = 5,
    /** authentication by token */
    TOKEN = 6,
    /** authentication by certification */
    OAUTH2 = 7,
    /** authentication by other method */
    OTHER = 8
}

/** authentication struct 
  * authType is in {OS_PASSWORD|APP_PASSWORD|LADP}
  *     authkey is user name, authPwd is password
  * authType is AKSK
  *     authkey is user name, authPwd is password
  * authType is in {KERBEROS|TOKEN|OAUTH2|OTHER}
  * the authentication information is stored in extendInfo
*/
struct Authentication {
    /** authentication type */
    1:required AuthType authType,
    /** authentication user name */
    2:optional string authkey,
    /** authentication user password */
    3:optional string authPwd,
    /** authentication extend information, when authentication by certification, certification's thumbprint is optional */
    4:optional string extendInfo
}

/** application resource struct, .eg database datafile or vmware virtual machine disk */
struct ApplicationResource {
    /** whether the resource is leaf resource, leaf resource have no sub resource */
    1:required string type,
    /** application type */
    2:optional string subType,
    /** resource id, length:0~256, resource inner id */
    3:required string id,
    /** resource name */
    4:optional string name,
    /** parent resource id */
    5:optional string parentId,
    /** parent resource name */
    6:optional string parentName,
    /** resource extend information */
    7:optional string extendInfo
}

/** application struct, .eg database or VMware virtual machine */
struct Application {
    /** application type */
    1:required string type,
    /** application sub type */
    2:optional string subType,
    /** application id, if application have inner id, it's the inner id */
    3:optional string id,
    /** application name */
    4:required string name,
    /** application parent id */
    5:optional string parentId,
    /** application parent name */
    6:optional string parentName,
    /** application authentication */
    7:optional Authentication auth,
    /** application extend information */
    8:optional string extendInfo
}

/** environment struct, .eg database host or VMware vCenter */
struct ApplicationEnvironment {
    /** environment id */
    1:required string id,
    /** environment name */
    2:required string name,
    /** environment type */
    3:required string type,
    /** sub environment type */
    4:optional string subType,
    /** environment service ip address, only platform environment have this field */
    5:optional string endpoint,
    /** environment service ip port */
    6:optional i32 port,
    /** environment service authentication */
    7:optional Authentication auth,
    /** application environment node list, when the application is local at cluster, nodes will have all nodes */
    8:optional list<ApplicationEnvironment> nodes,
    /** environment extend information */
    9:optional string extendInfo
}

/** subjob type */
enum SubJobType {
    /** prerequisite job */
    PRE_SUB_JOB = 0,
    /** generate job */
    GENERATE_SUB_JOB = 1,
    /** business job */
    BUSINESS_SUB_JOB = 2,
    /** post job */
    POST_SUB_JOB = 3
}

/** sub job execute policy */
enum ExecutePolicy {
    /** sub job will dispatched in any node */
    ANY_NODE = 0,
    /** whether executing the job in creating job node */
    LOCAL_NODE = 1,
    /** sub job will dispatched in every node one time */
    EVERY_NODE_ONE_TIME = 2,
    /** retry at other node when one node failed */
    RETRY_OTHER_NODE_WHEN_FAILED = 3,
    /** sub job will executed at fixed node */
    FIXED_NODE = 4
}

/** sub job struct, plugin will generate new job with this struct */
struct SubJob {
    /** main job id, new sub job belong to main job */
    1:required string jobId,
    /** sub job id, the id is null when adding new sub job, the id is not null when executing sub job */
    2:optional string subJobId,
    /** sub job type */
    3:required SubJobType jobType,
    /** sub job name, one sub job have unique name in main job scope */
    4:required string jobName,
    /** sub job priority, begin with 1, and sub job will execute by priority, the value is null when executing sub job */
    5:optional i32 jobPriority,
    /** sub job execute policy */
    6:optional ExecutePolicy policy,
    /** whether main job will ignore this job executing result, the value is null when executing sub job */
    7:optional bool ignoreFailed,
    /** sub job executing node uuid, backup agent will check when acquiring job */
    8:optional string execNodeId,
    /** extend job information*/
    9:optional string jobInfo
}

/** storage repository data type */
enum RepositoryDataType {
    /** repository will save the meta data */
    META_REPOSITORY = 0,
    /** repository will save the data */
    DATA_REPOSITORY = 1,
    /** repository will save the cache data */
    CACHE_REPOSITORY = 2,
    /** repository will save the log */
    LOG_REPOSITORY = 3,
    /** repository will save the index data */
    INDEX_REPOSITORY = 4,
    /** repository will save log meta data */
    LOG_META_REPOSITORY = 5
}

/** storage repository protoco type */
enum RepositoryProtocolType {
    /** repository using CIFS protocol */
    CIFS = 0,
    /** repository using NFS protocol */
    NFS = 1,
    /** repository using S3 protocol */
    S3 = 2,
    /** repository using blcok protocol */
    BLOCK = 3,
    /** repository using local directory */
    LOCAL_DIR = 4,
    /** repository using tape */
    TAPE = 5,
}

/** backup copy type */
enum CopyFormatType {
    /** copy using backup storage snapshot */
    INNER_SNAPSHOT = 0,
    /** copy using backup storage directory */
    INNER_DIRECTORY = 1,
    /** copy using extenal data */
    EXTERNAL = 2
}

/** copy data type*/
enum CopyDataType {
    /** full backup copy */
    FULL_COPY = 1,
    /** incremental backup copy*/
    INCREMENT_COPY = 2,
    /** different backup copy*/
    DIFF_COPY = 3,
    /** log backup copy */
    LOG_COPY = 4,
    /** external storage snapshot copy */
    SNAPSHOT_COPY = 5,
    /** permanent incremental backup copy */
    PERMANENT_INCREMENTAL_COPY = 6,
    /** replication copy */
    REPLICATION_COPY = 7,
    /** cloud storage copy */
    CLOUD_STORAGE_COPY = 8,
    /** tape storage copy */
    TAPE_STORAGE_COPY = 9,
    /** clone copy */
    CLONE_COPY = 10
}

/** Host Address */
struct HostAddress {
    /** host address ip .eg NFS server ip address in NFS protocol */
    1:required string ip,
    /** host address port */
    2:required i32 port,
    /** protocol supported by logical ports. eg 1:NFS,2:CIFS,3:NFS+CIFS,1024:Dataturbo*/
    3:optional i32 supportProtocol
}

/** repository role */
enum RepositoryRole {
    /** repository is master */
    REPO_MASTER = 0,
    /** repository is slave */
    REPO_SLAVE = 1
}

/** backup storage repository */
struct StorageRepository {
    /** backup repository id */
    1:required string id,
    /** backup repository type */
    2:required RepositoryDataType repositoryType,
    /** repository role if task have several repositories */
    3:optional RepositoryRole role,
    /** backup repository path is local A8000 path*/
    4:required bool isLocal,
    /** backup repository local mount path */
    5:optional list<string> path,
    /** backup respository protocol type */
    6:required RepositoryProtocolType protocol,
    /** backup repository authentication information */
    7:optional Authentication auth,
    /** backup respository manage endpoint */
    8:optional HostAddress endpoint,
    /** backup respository data access path */
    9:optional string remotePath,
    /** backup respository data access name, .eg cifs protocol share name */
    10:optional string remoteName,
    /** backup respository data access endpoint */
    11:optional list<HostAddress> remoteHost,
    /** extend authentication information of repository, 
        .eg backup to NFS, the extendAuth is the authentication informatio of nfs server 
      */
    12:optional Authentication extendAuth,
    /** backup repository cifs authentication information */
    13:optional Authentication cifsAuth,
    /** respository extend information
      * if protocol is S3, the extendInfo will contain restore server information, extend information is as follows:
      * {
      *     "enableSSL": [False|True],
      *     "serviceInfo": {
      *         [
      *             "ip": "server1 address",
      *             "port": "server port"
      *         ],
      *         [
      *             "ip": "server2 address",
      *             "port": "server port"
      *         ]
      *     }
      * }
      */
    14:optional string extendInfo
}

/** storage snapshot information */
struct Snapshot {
    /** storage snapshot id */
    1:required string id,
    /** storage snapshot parent name, nas file system name or logical unit name */
    2:required string parentName
}

/** backup copy information */
struct Copy {
    /** backup copy data format type */
    1:optional CopyFormatType formatType,
    /** backup copy data type */
    2:optional CopyDataType dataType,
    /** backup copy id */
    3:required string id,
    /** backup copy name */
    4:optional string name,
    /** backup copy protect application timestamp, it's requried  when application need support log backup */
    5:optional i64 timestamp,
    /** backup copy application transaction No., it's requried when application need support log backup and application have inner transaction No. */
    6:optional i64 transactionNo,
    /** protect environment, .eg protect vCenter or protect host */
    7:optional ApplicationEnvironment protectEnv,
    /** protect application, .eg protect virtual machine or database */
    8:optional Application protectObject,
    /** protect resource about application, .eg protect vm disk or database datafile */
    9:optional list<ApplicationResource> protectSubObjects,
    /** when image is inner type, path is respository type in backup storage, if the application use multiple filesystem, this field need list */
    10:optional list<StorageRepository> repositories,
    /** additional storage snapshot list information */
    11:optional list<Snapshot> snapshots,
    /** copy extend information */
    12:optional string extendInfo
}

/** backup job type */
enum BackupJobType {
    /** full backup */
    FULL_BACKUP = 1,
    /** incremental backup, backup different data base on lastest backup */
    INCREMENT_BACKUP = 2,
    /** different backup, backup different data base on lastest full backup */
    DIFF_BACKUP = 3,
    /** log backup */
    LOG_BAKCUP = 4,
    /** external storage backup */
    SNAPSHOT = 5,
    /** permanent incremental backup, after incremental backup, plugin need synthetic full backup copy */
    PERMANENT_INCREMENTAL_BACKUP = 6
}

/** query data by page */
struct QueryByPage {
    /** page number */
    1:required i32 pageNo,
    /** one page size */
    2:required i32 pageSize,
    /** data order field */
    3:optional list<string> orders,
    /** filter condition */
    4:optional string conditions,
}

/** job permission */
struct JobPermission {
    /** user id */
    1:required string user,
    /** group id */
    2:required string group,
    /** file mode */
    3:optional string fileMode,
    /** whether mount file system before executing jobs */
    4:optional bool isMount,
    /** job permission extend information */
    5:optional string extendInfo
}
