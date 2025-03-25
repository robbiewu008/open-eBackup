/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 *
 * @file xbsa.h
 * @brief  Contains function declarations xbsa
 * @version 1.0.0
 * @date 2020-08-01
 * @author wangguitao 00510599
 */
/* xbsa.h
 *
 * This is a sample C header file describing the XBSA.
 *
 * This appendix is not a normative part of the
 * specification and is provided for illustrative
 * purposes only.
 *
 * Implementations must ensure that the sizes of integer
 * datatypes match their names, not necessarily the typedefs
 * presented in this example.
 *
 */
#ifndef XBSASTRUCT
#define XBSASTRUCT

#include <time.h>

#define XBSA_EXPORT_API __attribute__ ((visibility("default")))
#ifdef __cplusplus
extern "C" {
#endif

/* BSA_Int16
 */
typedef short BSA_Int16;

/* BSA_Int32
 */
typedef int BSA_Int32;

/* BSA_Int64
 */
typedef struct { /* defined as two 32-bit integers */
    BSA_Int32   left;
    BSA_Int32   right;
} BSA_Int64;

/* BSA_UInt16
 */
typedef unsigned short BSA_UInt16;

/* BSA_UInt32
 */
typedef unsigned int BSA_UInt32;

/* BSA_UInt64
 */
typedef struct {  /*  defined as two unsigned 32-bit integers */
    BSA_UInt32  left;
    BSA_UInt32  right;
} BSA_UInt64;

/* BSA_ShareId, Shared Memory Buffer reference
 */
typedef short BSA_ShareId;  /*  operating system dependent */

/* Constants used
 *
 * Maximum string lengths (lower bound), including trailing null
 */
#define BSA_MAX_APPOBJECT_OWNER     64 // Max end-user object owner length
#define BSA_MAX_BSAOBJECT_OWNER     64 // Max BSA object owner length
#define BSA_MAX_DESCRIPTION        100 // Description field
#define BSA_MAX_OBJECTSPACENAME   1024 // Max ObjectSpace name length
#define BSA_MAX_OBJECTINFO         256 // Max object info size
#define BSA_MAX_PATHNAME          1024 // Max path name length
#define BSA_MAX_RESOURCETYPE        31 // Max resource mgr name length
#define BSA_MAX_TOKEN_SIZE          64 // Max size of a security token

/* Other constants */

#define BSA_ANY                     1 // General-purpose enumeration wild-card value

/*
 * Return Codes Used
 */
#define BSA_RC_ABORT_SYSTEM_ERROR         0x03
#define BSA_RC_ACCESS_FAILURE             0x4D
#define BSA_RC_AUTHENTICATION_FAILURE     0x04
#define BSA_RC_BUFFER_TOO_SMALL           0x4E
#define BSA_RC_INVALID_CALL_SEQUENCE      0x05
#define BSA_RC_INVALID_COPYID             0x4F
#define BSA_RC_INVALID_DATABLOCK          0x34
#define BSA_RC_INVALID_ENV                0x50
#define BSA_RC_INVALID_HANDLE             0x06
#define BSA_RC_INVALID_OBJECTDESCRIPTOR   0x51
#define BSA_RC_INVALID_QUERYDESCRIPTOR    0x53
#define BSA_RC_INVALID_VOTE               0x0B
#define BSA_RC_NO_MATCH                   0x11
#define BSA_RC_NO_MORE_DATA               0x12
#define BSA_RC_NULL_ARGUMENT              0x55
#define BSA_RC_OBJECT_NOT_FOUND           0x1A
#define BSA_RC_SUCCESS                    0x00
#define BSA_RC_TRANSACTION_ABORTED        0x20
#define BSA_RC_VERSION_NOT_SUPPORTED      0x4B

// describes the type of the operation used to create the object
typedef enum {
    BSA_CopyType_ANY = 1, // Used for matching any copy type (for example, "backup" or "archive" in the copy type
                        // field of structures for selecting query results).
    BSA_CopyType_ARCHIVE = 2, // Specifies that the copy type should be "archive".
    BSA_CopyType_BACKUP = 3   // Specifies that the copy type should be "backup".
} BSA_CopyType;

// describes the current status of the object
typedef enum {
    BSA_ObjectStatus_ANY = 1,         // Provides a wild card function. Can only be used in queries.
    BSA_ObjectStatus_MOST_RECENT = 2, // Indicates that this is the most recent backup copy of an object.
    BSA_ObjectStatus_NOT_MOST_RECENT = 3, // Indicates that this is not the most recent backup copy, or that the
                                        // object itself no longer exists.
    BSA_OBJECTSTATUS_APPEND_WRITE = 4,
    BSA_OBJECTSTATUS_OVERWRITE_WRITE = 5
} BSA_ObjectStatus;

// describes the original data type of the object
typedef enum {
    BSA_ObjectType_ANY = 1, // Used for matching any object type (for example, "file" or directory") value in the object
                            // type field of structures for selecting query results.
    BSA_ObjectType_FILE = 2, // Used by the application to indicate that the type of application object is a "file" or
                            // single object.
    BSA_ObjectType_DIRECTORY = 3, // Used by the application to indicate that the type of application object is a
                                // "directory" or container of objects.
    BASBSA_ObjectType_OTHER = 4 // Used by the application to indicate that the type of application object is neither a
                                // "file" nor a "directory".
} BSA_ObjectType;

// describes whether or not the transaction is to be committed
typedef enum {
    BSA_Vote_COMMIT = 1, // The transaction is to be committed.
    BSA_Vote_ABORT  = 2 // The transaction is to be aborted.
} BSA_Vote;

// describes xbsa request instance id position
typedef enum {
    START_POSITION = 0, // The transaction is to be committed.
    END_POSITION  = 6 // The transaction is to be aborted.
} XBSA_INSTANCE_INFO;

// describes the version of the API that is implemented
// eg,XBSA 1.1.x: issue=1,version=1,level=x
typedef struct {
    BSA_UInt16      issue; // Issue Number of the XBSA Specification
    BSA_UInt16      version; // Version Number of the XBSA Specification
    BSA_UInt16      level; // Implementation-defined version number
} BSA_ApiVersion;

// used to pass data between an XBSA Application and the Backup Service
// The values assigned to the various structure fields would always obey the following relationships:
//  bufferLen    >= headerBytes + numBytes
//  trailerBytes == (bufferLen - numBytes - headerBytes)
//
// The header and trailer portions of the buffer are reserved for the use of the Backup Service,
// and should not be modified by the XBSA Application. The XBSA Application should only write to the data portion of
// the buffer,which is the only portion used for transferring application data.
// The sizes for the header and trailer portions of the buffer that are required by the Backup Service are
// obtained by calling BSACreateObject() or BSAGetObject().
typedef struct {
    BSA_UInt32  bufferLen; // Length of the allocated buffer
    BSA_UInt32 numBytes;    // Actual number of bytes read from or written to the buffer, or the minimum number of bytes
                            // needed
    BSA_UInt32 headerBytes; // Number of bytes used at start of buffer for header information (offset to data portion of
                            // buffer)
    BSA_ShareId shareId; // Value used to identify a shared memory block.
    BSA_UInt32  shareOffset; // Specifies the offset of the buffer in the shared memory block.
    void       *bufferPtr; // Pointer to the buffer
} BSA_DataBlock32;


typedef struct {
    BSA_UInt16  bufferLen; // Length of the allocated buffer
    BSA_UInt16 numBytes;    // Actual number of bytes read from or written to the buffer, or the minimum number of bytes
                            // needed
    void       *bufferPtr; // Pointer to the buffer
} BsaDataBlock16Iif;


typedef struct {
    BSA_UInt32  bufferLen; // Length of the allocated buffer
    BSA_UInt32 numBytes;    // Actual number of bytes read from or written to the buffer, or the minimum number of bytes
                            // needed
    void       *bufferPtr; // Pointer to the buffer
} BsaDataBlock32Iif;

// the name assigned by an XBSA Application to an XBSA Object.
//
// An objectSpaceName is an optionally defined, fixed-length character string. It identifies a logical space,
// called an Object space, in which the object belongs. For example, an Object space may be used to identify
// a storage volume (for example, a disk partition, or a floppy disk), or a database in the XBSA Application's domain.
//
// The concept of an Object space is used to provide a primary grouping of XBSA Objects, which may be used for object
// search by a user and/or for object management by the Backup Service. Additional groupings are provided by Filespec
// and by object attributes.
// Examples of an objectSpaceName are C: Drive and VolumeLabel=XYZ.
//
// A pathName is a hierarchical character string that identifies an XBSA Object within an ObjectSpace.
// An example of a pathName for the backup copy of a UNIX file may be its original path name and file name,
// for example, /documents/opengroup/backup.proposal.
//
// The value of the delimiter used to separate name components can be obtained by calling BSAGetEnvironment().
typedef struct {
    char  objectSpaceName[BSA_MAX_OBJECTSPACENAME]; // Highest-level name qualifier
    char  pathName[BSA_MAX_PATHNAME]; // Object name within objectspace
} BSA_ObjectName;

// the name of the owner of an object.
typedef struct {
    char  bsa_ObjectOwner[BSA_MAX_BSAOBJECT_OWNER]; // this is the name that the Backup Service authenticates
    char  app_ObjectOwner[BSA_MAX_APPOBJECT_OWNER]; // this is the name defined by the application
} BSA_ObjectOwner;

// used to describe an object.
// Some of the fields in this structure are supplied by the XBSA Application (Direction = in),
// and some by the Backup Service (Direction = out). Some fields are optional.
//
// All values in a BSA_ObjectDescriptor must be valid before the BSA_ObjectDescriptor as a whole is valid.
// For enumerations valid values exclude the enumeration "ANY". For strings valid values are null-terminated.
// The optional string value is the empty string. The optional restoreOrder value is zero.
// The optional objectInfo value is all zeros (that is, a zero-filled field).
// The mandatory objectName must have a non-empty string in the pathName field. The mandatory createTime must
// be a valid time in UTC.
// The mandatory copyId must be non-zero. The mandatory resourceType must have a non-empty string value.
typedef struct {
    BSA_UInt32          rsv1;
    BSA_ObjectOwner     objectOwner; // Owner of the object //client,o
    BSA_ObjectName      objectName; // Object name //client,m
    struct tm           createTime; // Create time //service,m
    BSA_CopyType        copyType; // Copy type: archive or backup //client,m
    BSA_UInt64          copyId; // Unique object identifier //service,m
    BSA_UInt64 restoreOrder; // Provides hints to the XBSA Application that allow it to optimize the order of object
                            // retrieval requests // service,o
    char                rsv2[31];
    char                rsv3[31];
    BSA_UInt64          estimatedSize; // Estimated object size in bytes, may be up to (2^64 - 1) bits //client,m
    char                resourceType[BSA_MAX_RESOURCETYPE]; // for example, UNIX file system //client,m
    BSA_ObjectType      objectType; // for example, file, directory, database //client,m
    BSA_ObjectStatus    objectStatus; // Most recent / Not most recent //service,m
    char               *rsv4[31];
    char                objectDescription[BSA_MAX_DESCRIPTION]; // Descriptive label for the object //client,o
    unsigned char       objectInfo[BSA_MAX_OBJECTINFO]; // Application-specific information //client,o
} BSA_ObjectDescriptor;

// used to query the repository in order to locate objects.
typedef struct {
    BSA_ObjectOwner     objectOwner;
    BSA_ObjectName      objectName;
    struct tm           rsv1;
    struct tm           rsv2;
    struct tm           rsv3;
    struct tm           rsv4;
    BSA_CopyType        copyType;
    char                rsv5[31];
    char                rsv6[31];
    char                rsv7[31];
    BSA_ObjectType      objectType;
    BSA_ObjectStatus    objectStatus;
    char                rsv8[100];
} BSA_QueryDescriptor;

// contains an application-specific security token
typedef  char BSA_SecurityToken;

typedef enum {
    BSA_DWS = 1,
    BSA_INFORMIX  = 2,
    BSA_HCS = 3,
    BSA_TPOPS = 4,
    BSA_GBASE_8S = 5,
    BSA_UNKNOWN = 255
} BSA_AppType;

#ifdef __cplusplus
}
#endif  /* __cplusplus */

#endif