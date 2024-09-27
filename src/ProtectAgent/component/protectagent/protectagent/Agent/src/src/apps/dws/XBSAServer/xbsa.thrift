/**-------------------------------------------------------------------------------------
CallResult: the return value type of the function. 
--Attributes:
----response: If the response value is BSA_RC_SUCCESS, data is successfully obtained from the backup service. 
              Otherwise, data fails to be obtained.
--------------------------------------------------------------------------------------------*/
struct CallResult {
    1: required i32 response
}

/**--------------------------------------------------------------------------------
BsaUInt64: custom unsigned 64-bit integer. 
--Attributes:
----left: 0~9223372036854775807.
----right: 9223372036854775808~18446744073709551615.
----------------------------------------------------------------------------------*/
struct BsaUInt64 {
    1: required i64 left,
    2: required i64 right
}

/**---------------------------------------------------------------------------
TM: custom time type.
--Attributes:
----utcTime: Universal Standard Time.
---------------------------------------------------------------------------*/
struct TM {
    1: required string utcTime
}

/**--------------------------------------------------------------------------
BsaObjectOwner: backup object holder type. 
--Attributes:
----bsaObjectOwner: Backup service authenticates, the type is character array, 
                    the maximum length of the array is 64 characters. 
----appObjectOwner: Name defined by the application, the type is character array, the maximum length of the 
                    array is 64 characters.
----------------------------------------------------------------------------*/
struct BsaObjectOwner {
    1: required binary bsaObjectOwner,
    2: required binary appObjectOwner
}

/**--------------------------------------------------------------------------
BsaObjectName: object name type.
--Attributes:
----BsaObjectName: name of the space to which the object belongs, the type is character array, 
                   the maximum length of the array is 1024 characters.
----pathName: Path of the backup object in the space to which the backup object belongs, the type 
              is character array, the maximum length of the array is 1024 characters.
---------------------------------------------------------------------------*/
struct BsaObjectName {
    1: required binary objectSpaceName,
    2: required binary pathName
}

/**---------------------------------------------------------------------------------
BsaDataBlock32: data packet type. 
--Attributes:
----bufferLen: Length of the allocated buffer.
----numBytes: Actual number of bytes read from or written to the buffer, or the minimum number of bytes needed.
----headerBytes: Number of bytes used at start of buffer for header information (offset to data portion of buffer).
----shareId: Value used to identify a shared memory block.
----hareOffset: Specifies the offset of the buffer in the shared memory block.
----bufferPtr: Pointer to the buffer.
--------------------------------------------------------------------------------------*/
struct BsaDataBlock32 {
    1: required i64 bufferLen,
    2: required i64 numBytes,
    3: required i64 headerBytes,
    4: required i64 shareId,
    5: required i64 shareOffset,
    6: required binary bufferPtr
}

/**----------------------------------------------------------------------------------------------
BsaObjectDescriptor: a backup object descriptor. 
--Attributes:
----rsv1: reserved field.
----objectOwner: Owner of the object.
----objectName: Object name.
----createTime: Indicates the time when an object is created.
----copyType: the type of the operation used to create the object
------value:
--------BSA_CopyType_ANY: match any copy type. (For example, Backup or Archive in the Replication Type 
                          field of the structure used to select the query results).
--------BSA_CopyType_ARCHIVE: specifies that the copy type should be "archive".
--------BSA_CopyType_BACKUP: Specifies that the copy type should be "backup".
----copyId: Unique object identifier.
----restoreOrder: Provides hints to the XBSA Application that allow it to optimize the order of object retrieval requests.
----rsv2: reserved field. the type is character array, the maximum length of the array is 31 characters.
----rsv3: reserved field. the type is character array, the maximum length of the array is 31 characters.
----estimatedSize: Estimated object size in bytes, may be up to (2^64 - 1) bits.
----resourceType: for example, UNIX file system, the type is character array, the maximum length of the array is 31 characters.
----objectType: for example, file, directory, database.
------value:
--------BSA_ObjectType_ANY: Used for matching any object type (for example, "file" or directory") value in the 
                            object type field of structures for selecting query results.
--------BSA_ObjectType_FILE: Used by the application to indicate that the type of application object is a "file" or 
                             single object.
--------BSA_ObjectType_DIRECTORY: Used by the application to indicate that the type of application object is a 
                                  "directory" or container of objects.
--------BASBSA_ObjectType_OTHER: Used by the application to indicate that the type of application object is neither a
                                 "file" nor a "directory".
----objectStatus: Most recent / Not most recent
------value: reserved field, 
--------BSA_ObjectStatus_ANY: Provides a wild card function. Can only be used in queries.
--------BSA_ObjectStatus_MOST_RECENT: Indicates that this is the most recent backup copy of an object.
--------BSA_ObjectStatus_NOT_MOST_RECENT: Indicates that this is not the most recent backup copy, or that 
                                          the object itself no longer exists.
----rsv4: reserved field. the type is Character array pointer array, the maximum length of the array is 31.
----objectDescription: Descriptive label for the object, the type is character array, the maximum length of 
                       the array is 100 characters.
----objectInfo: Application-specific information. the type is character array, the maximum length of 
                the array is 256 characters.
----------------------------------------------------------------------------------------------*/
struct BsaObjectDescriptor {
    1: optional i64 rsv1,
    2: required BsaObjectOwner objectOwner,
    3: required BsaObjectName objectName,
    4: required TM createTime,
    5: required i32 copyType,
    6: required BsaUInt64 copyId,
    7: required BsaUInt64 restoreOrder,
    8: optional binary rsv2,
    9: optional binary rsv3,
    10: required BsaUInt64 estimatedSize,
    11: required binary resourceType,
    12: required i32 objectType,
    13: required i32 objectStatus,
    14: optional binary rsv4,
    15: required binary objectDescription,
    16: required binary objectInfo
}

/**---------------------------------------------------------------------------------------------
BsaQueryDescriptor: query result descriptor type. 
--Attributes:
----objectOwner: Owner of the object.
----objectName: Object name.
----rsv1: reserved field.
----rsv2: reserved field.
----rsv3: reserved field.
----rsv4: reserved field.
----copyType: the type of the operation used to create the object
------value:
--------BSA_CopyType_ANY: match any copy type. (For example, Backup or Archive in the Replication Type 
                          field of the structure used to select the query results).
--------BSA_CopyType_ARCHIVE: specifies that the copy type should be "archive".
--------BSA_CopyType_BACKUP: Specifies that the copy type should be "backup".
----rsv5: reserved field. the type is character array, the maximum length of the array is 31 characters.
----rsv6: reserved field. the type is character array, the maximum length of the array is 31 characters.
----rsv7: reserved field. the type is character array, the maximum length of the array is 31 characters.
----objectType: for example, file, directory, database.
------value:
--------BSA_ObjectType_ANY: Used for matching any object type (for example, "file" or directory") value in the 
                            object type field of structures for selecting query results.
--------BSA_ObjectType_FILE: Used by the application to indicate that the type of application object is a "file" or 
                             single object.
--------BSA_ObjectType_DIRECTORY: Used by the application to indicate that the type of application object is a 
                                  "directory" or container of objects.
--------BASBSA_ObjectType_OTHER: Used by the application to indicate that the type of application object is neither a
                                 "file" nor a "directory".
----objectStatus: Most recent / Not most recent
------value: reserved field, 
--------BSA_ObjectStatus_ANY: Provides a wild card function. Can only be used in queries.
--------BSA_ObjectStatus_MOST_RECENT: Indicates that this is the most recent backup copy of an object.
--------BSA_ObjectStatus_NOT_MOST_RECENT: Indicates that this is not the most recent backup copy, or that 
                                          the object itself no longer exists.
----rsv8: reserved field. the type is character array, the maximum length of the array is 100 characters.
------------------------------------------------------------------------------------*/
struct BsaQueryDescriptor {
    1: required BsaObjectOwner objectOwner,
    2: required BsaObjectName objectName,
    3: optional TM rsv1,
    4: optional TM rsv2,
    5: optional TM rsv3,
    6: optional TM rsv4,
    7: required i32 copyType,
    8: optional binary rsv5,
    9: optional binary rsv6,
    10: optional binary rsv7,
    11: required i32 objectType,
    12: required i32 objectStatus,
    13: optional binary rsv8
}

/**------------------------------------------------------------------------------
BsaApiVersion: API version type. 
--Attributes:
----issue: Issue Number of the XBSA Specification.
----version: Version Number of the XBSA Specification.
----level: Implementation-defined version number.
-------------------------------------------------------------------------------*/
struct BsaApiVersion {
    1: i32 issue,
    2: i32 version,
    3: i32 level
}

/**---------------------------------------------------------------------------------
HeartBeatResult: the return type of the heartbeat detection function.
--Attributes:
----strKey: reserved field.
----response: If the response value is BSA_RC_SUCCESS, the connection is normal. Otherwise, 
              the connection is abnormal.
-----------------------------------------------------------------------------------*/
struct HeartBeatResult {
    1: required string strKey,
    2: required i32 response
}

/**----------------------------------------------------------------------------------
CreateObjectResult: the return type of the function for creating a backup object. 
--Attributes:
----objectDescriptor: backup object descriptor.
----dataBlock: data packet.
----storePath: storage path.
----response: If the response value is BSA_RC_SUCCESS, the backup object is successfully created. Otherwise,
              the backup object fails to be created.
----------------------------------------------------------------------------------------*/
struct CreateObjectResult {
    1: required BsaObjectDescriptor objectDescriptor,
    2: required BsaDataBlock32 dataBlock,
    3: required string storePath,
    4: required i32 response 
}

/**-------------------------------------------------------------------------------------
GetDataResult: the return value type of the function that obtains data from the backup service. 
--Attributes:
----dataBlock: data packet.
----response: If the response value is BSA_RC_SUCCESS, data is successfully obtained from the backup service. 
              Otherwise, data fails to be obtained.
--------------------------------------------------------------------------------------------*/
struct GetDataResult {
    1: required BsaDataBlock32 dataBlock,
    2: required i32 response
}

/**---------------------------------------------------------------------------------------------
GetEnvironmentResult: the return value type of the function for obtaining process environment variables. 
--Attributes:
----environmentPtr: Not used currently.
----response: If the response value is BSA_RC_SUCCESS, the process environment variables.
              are obtained successfully. Otherwise, the process environment variables fail to be obtained.
--------------------------------------------------------------------------------------------*/
struct GetEnvironmentResult {
    1: required binary environmentPtr,
    2: required i32 response
}

/**-------------------------------------------------------------------------------------------
GetNextQueryObjectResult: the return value type of the function that obtains the next record 
of the previous query result. 
--Attributes:
----objectDesc: backup object descriptor.
----storePath: storage path.
----response: If the response value is BSA_RC_SUCCESS, the next record is obtained. Otherwise, 
              the next record fails to be obtained.
----getDataType: Backup data type.
----archiveBackupId: Archive backup ID.
----archiveServerIp: IP address of the archive service
----archiveServerPort: Listening port number of the archive service.
----archiveOpenSSL: Whether an SSL certificate is required for archiving.
----fsID: File system ID.
-----------------------------------------------------------------------------------------------*/
struct GetNextQueryObjectResult {
    1: required BsaObjectDescriptor objectDesc,
    2: required string storePath,
    3: required i32 response,
    4: required i32 getDataType,
    5: required string archiveBackupId,
    6: required string archiveServerIp,
    7: required i32 archiveServerPort,
    8: required i32 archiveOpenSSL,
    9: required string fsID
}

/**-----------------------------------------------------------------------------------------
GetObjectResult: the return value type of the packet function for obtaining backup objects. 
--Attributes:
----dataBlock: data packet.
----response: If the response value is BSA_RC_SUCCESS, the data packet is successfully obtained. 
              Otherwise, the data packet fails to be obtained.
----------------------------------------------------------------*/
struct GetObjectResult {
    1: required BsaDataBlock32 dataBlock,
    2: required i32 response
}

/**-------------------------------------------------------------------------------------
BSAnitResult: the return value type of the function for creating an backup session. 
--Attributes:
----handle: backup session handle.
----response: If the response value is BSA_RC_SUCCESS, the backup session is successfully created. Otherwise, 
              the backup session fails to be created.
----------------------------------------------------------------------------------------*/
struct BSAInitResult {
    1: required i64 handle,
    2: required i32 response
}

/**------------------------------------------------------------------------------------------
QueryApiVersionResult: the return value type of the function for querying the API version. 
--Attributes:
----version: API version.
----response: If the response value is BSA_RC_SUCCESS, the API version is successfully queried.
              Otherwise, the API version fails to be queried.
---------------------------------------------------------------------------------------------*/
struct QueryApiVersionResult {
    1: required BsaApiVersion version,
    2: required i32 response
}

/**----------------------------------------------------------------------------------------------
QueryObjectResult: the return value type of the function that obtains the current record of 
the specified query result. 
--Attributes: 
----objectDesc: backup object descriptor.
----storePath: storage path.
----response: If the response value is BSA_RC_SUCCESS, the backup object is successfully obtained. Otherwise, 
              the backup object fails to be obtained.
----getDataType: Backup data type.
----archiveBackupId: Archive backup ID.
----archiveServerIp: IP address of the archive service
----archiveServerPort: Listening port number of the archive service.
----archiveOpenSSL: Whether an SSL certificate is required for archiving.
----fsID: File system ID.
--------------------------------------------------------------------------------------------------*/
struct QueryObjectResult {
    1: required BsaObjectDescriptor objectDesc,
    2: required string  storePath,
    3: required i32 response,
    4: required i32 getDataType,
    5: required string archiveBackupId,
    6: required string archiveServerIp,
    7: required i32 archiveServerPort,
    8: required i32 archiveOpenSSL,
    9: required string fsID
}

/**-------------------------------------------------------------------------------------------------
GetLastErrorResult: the return value type of the function that obtains the last error. 
--Attributes:
----detailError: error description.
----bufferSize: error description string length.
----response: If the response value is BSA_RC_SUCCESS, the previous error is obtained successfully. Otherwise,
              the previous error fails to be obtained.
---------------------------------------------------------------------------------------------------*/
struct GetLastErrorResult {
    1: required string detailError,
    2: required i32 bufferSize,
    3: required i32 response
}

/**-------------------------------------------------------------------------------------------------
QueryServiceProviderResult: the return value type of the function that queries the service provider.
--Attributes: 
----delimiter: Delimiter that identifies the hierarchy string of the backup service provider, the type 
               is array of characters, The maximum length of the character array is determined by the retSize attribute.
----providerPtr: hierarchy description that identifies the backup service provider, the type is array of 
                 characters, The maximum length of the character array is determined by the retSize attribute.
----retSize: a length (signed 32-bit integer) of the hierarchy description that identifies the backup service provider.
----response: If the response value is BSA_RC_SUCCESS, querying the hierarchy description of the backup service 
              provider succeeds; otherwise, querying the hierarchy description of the backup service provider fails.
----------------------------------------------------------------------------------------------------*/
struct QueryServiceProviderResult {
    1: required binary delimiter,
    2: required binary providerPtr,
    3: required i32 retSize,
    4: required i32 response
}


/**------------------------------------------------------------
--------------------------------------------------------------
function name: HeartBeat.
function：connection status detection.
--------------------------------------------------------------
--------------------------------------------------------------
function name: BSABeginTxn.
function: begin an API transaction.
farameter: 
    handle: session handle, its type is signed long integer type.
return: If BSA_RC_SUCCESS is returned, the execution is successful.  Otherwise, the execution fails.
--------------------------------------------------------------
--------------------------------------------------------------
function name: BSACreateObject.
function: create a BSA object (either a backup or an archive copy)
parameter: 
    handle: Session handle, its type is signed long integer type.
--------------------------------------------------------------
--------------------------------------------------------------
function name: BSADeleteObject
function: delete a BSA object.
parameter: 
    handle: Session handle, its type is signed long integer type.
return: If BSA_RC_SUCCESS is returned, the execution is successful. Otherwise, the execution fails.
--------------------------------------------------------------
--------------------------------------------------------------
function name: BSAEndData
function: notified of the transaction that the backup object has completed sending or reading data,
end a BSAGetData() or BSASendData() sequence.
parameter: 
    handle: Session handle, its type is signed long integer type.
    estimatedSize: XBSA object's estimated size. its type is unsigned long long.
    size: XBSA total IO size in a period. its type is signed long integer type.
return: If BSA_RC_SUCCESS is returned, the execution is successful. Otherwise, the execution fails.
--------------------------------------------------------------
--------------------------------------------------------------
function name: BSAEndTxn
function: end a transaction
parameter: 
    handle: Session handle, its type signed is long integer type.
return: If BSA_RC_SUCCESS is returned, the execution is successful. Otherwise, the execution fails.
--------------------------------------------------------------
--------------------------------------------------------------
function name: BSAGetData
function: get a byte stream of data using buffers.
parameter: 
    handle: Session handle, its type signed is long integer type.
--------------------------------------------------------------
--------------------------------------------------------------
function name: BSAGetEnvironment
function: retrieve the current environment for the session.
parameter: 
    handle: Session handle, its type signed is long integer type.
--------------------------------------------------------------
--------------------------------------------------------------
function name: BSAGetLastError
function: retrieve the error code for the last system error.
parameter: 
    bufferSize: Error description character array length, its type is 64-bit long integer.
--------------------------------------------------------------
--------------------------------------------------------------
function name: BSAGetNextQueryObject
function: get the next object relating to a previous query.
parameter: 
    handle: Session handle, its type is signed long integer type.
--------------------------------------------------------------
--------------------------------------------------------------
function name: BSAGetObject
function: get an object.
parameter: 
    handle: Session handle, its type is signed long integer type.
--------------------------------------------------------------
--------------------------------------------------------------
function name: BSAInit
function: initialize the environment and set up a session.
parameter: 
    envPtr: Process environment, its type is byte array.
--------------------------------------------------------------
--------------------------------------------------------------
function name: BSAQueryApiVersion
function: query for the current version of the API.
--------------------------------------------------------------
--------------------------------------------------------------
function name: BSAQueryObject
function: query about object copies.
parameter: 
    handle: Session handle, its type is signed long integer type.
--------------------------------------------------------------
--------------------------------------------------------------
function name: BSAQueryServiceProvider
function: query the name of the Backup Service implementation.
parameter: 
    retSize: Character array length describing the backup service provider, its type is 64-bit long integer.
--------------------------------------------------------------
--------------------------------------------------------------
function name: BSASendData
function: send a byte stream of data in a buffer.
parameter: 
    handle: Session handle, its type is signed long integer type.
return: If BSA_RC_SUCCESS is returned, the execution is successful. Otherwise, the execution fails.
--------------------------------------------------------------
--------------------------------------------------------------
function name: BSATerminate
function: terminate a session.
parameter: 
    handle: Session handle, its type is signed long integer type.
return: If BSA_RC_SUCCESS is returned, the execution is successful. Otherwise, the execution fails.
--------------------------------------------------------------
--------------------------------------------------------------*/

service BSAService {
    HeartBeatResult HeartBeat(),
    CallResult BSABeginTxn(1: i64 handle),
    CreateObjectResult BSACreateObject(1: i64 handle, 2: BsaObjectDescriptor objectDescriptor),
    CallResult  BSADeleteObject(1:i64 handle, 2: BsaUInt64 copyId),
    CallResult  BSAEndData(1:i64 handle, 2: BsaUInt64 estimatedSize, 3: i64 size),
    CallResult  BSAEndTxn(1:i64 handle, 2: i32 vote),
    GetDataResult  BSAGetData(1:i64 handle, 2: BsaDataBlock32 dataBlock),
    GetEnvironmentResult BSAGetEnvironment(1: i64 handle, 2: BsaObjectOwner objectOwner),
    GetLastErrorResult BSAGetLastError(1: i64 bufferSize),
    GetNextQueryObjectResult BSAGetNextQueryObject(1: i64 handle),
    GetObjectResult BSAGetObject(1: i64 handle, 2: BsaObjectDescriptor objectDesc),
    BSAInitResult BSAInit(1:BsaObjectOwner objectOwner, 2: binary envPtr, 3: i32 appType),
    QueryApiVersionResult BSAQueryApiVersion(),
    QueryObjectResult BSAQueryObject(1: i64 handle, 2: BsaQueryDescriptor queryDesc),
    QueryServiceProviderResult BSAQueryServiceProvider(1: i64 retSize),
    CallResult BSASendData(1: i64 handle, 2: BsaDataBlock32 dataBlock),
    CallResult BSATerminate(1: i64 handle)
}
