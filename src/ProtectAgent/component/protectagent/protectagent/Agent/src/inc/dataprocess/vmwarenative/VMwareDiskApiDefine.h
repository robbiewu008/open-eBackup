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
#ifndef __AGENT_VMWARE_DISK_API_DEFINE_H__
#define __AGENT_VMWARE_DISK_API_DEFINE_H__

#include <string>
#include "common/Types.h"
#define HAVE_STDINT_H
using Bool = char;
#undef HAVE_STDINT_H
#undef LIKELY
#include <chrono>
#define VMWAREDISK_FLAG_OPEN_UNBUFFERED (1 << 0)   // disable host disk caching
#define VMWAREDISK_FLAG_OPEN_SINGLE_LINK (1 << 1)  // don't open parent disk(s)
#define VMWAREDISK_FLAG_OPEN_READ_ONLY (1 << 2)    // open read-only
#define VMWAREDISK_FLAG_OPEN_READ_WRITE (1 << 3)   // open read-write

/*
 * This header file includes function mappings obtained from the VDDK lib
 */
using uint8 = uint8_t;
using uint32 = uint32_t;
using uint64 = uint64_t;

using VixDiskLibSectorType = uint64;
using VixError = uint64;
using VMWARE_DISK_RET_CODE = uint64;
using VixDiskLibGenericLogFunc = void (const char *fmt, va_list args);

enum {
    VIX_OK = 0,

    /* General errors */
    VIX_E_FAIL = 1,
    VIX_E_OUT_OF_MEMORY = 2,
    VIX_E_INVALID_ARG = 3,
    VIX_E_FILE_NOT_FOUND = 4,
    VIX_E_OBJECT_IS_BUSY = 5,
    VIX_E_NOT_SUPPORTED = 6,
    VIX_E_FILE_ERROR = 7,
    VIX_E_DISK_FULL = 8,
    VIX_E_INCORRECT_FILE_TYPE = 9,
    VIX_E_CANCELLED = 10,
    VIX_E_FILE_READ_ONLY = 11,
    VIX_E_FILE_ALREADY_EXISTS = 12,
    VIX_E_FILE_ACCESS_ERROR = 13,
    VIX_E_REQUIRES_LARGE_FILES = 14,
    VIX_E_FILE_ALREADY_LOCKED = 15,
    VIX_E_VMDB = 16,
    VIX_E_NOT_SUPPORTED_ON_REMOTE_OBJECT = 20,
    VIX_E_FILE_TOO_BIG = 21,
    VIX_E_FILE_NAME_INVALID = 22,
    VIX_E_ALREADY_EXISTS = 23,
    VIX_E_BUFFER_TOOSMALL = 24,
    VIX_E_OBJECT_NOT_FOUND = 25,
    VIX_E_HOST_NOT_CONNECTED = 26,
    VIX_E_INVALID_UTF8_STRING = 27,
    VIX_E_OPERATION_ALREADY_IN_PROGRESS = 31,
    VIX_E_UNFINISHED_JOB = 29,
    VIX_E_NEED_KEY = 30,
    VIX_E_LICENSE = 32,
    VIX_E_VM_HOST_DISCONNECTED = 34,
    VIX_E_AUTHENTICATION_FAIL = 35,
    VIX_E_HOST_CONNECTION_LOST = 36,
    VIX_E_DUPLICATE_NAME = 41,
    VIX_E_ARGUMENT_TOO_BIG = 44,

    /* Handle Errors */
    VIX_E_INVALID_HANDLE = 1000,
    VIX_E_NOT_SUPPORTED_ON_HANDLE_TYPE = 1001,
    VIX_E_TOO_MANY_HANDLES = 1002,

    /* XML errors */
    VIX_E_NOT_FOUND = 2000,
    VIX_E_TYPE_MISMATCH = 2001,
    VIX_E_INVALID_XML = 2002,

    /* VM Control Errors */
    VIX_E_TIMEOUT_WAITING_FOR_TOOLS = 3000,
    VIX_E_UNRECOGNIZED_COMMAND = 3001,
    VIX_E_OP_NOT_SUPPORTED_ON_GUEST = 3003,
    VIX_E_PROGRAM_NOT_STARTED = 3004,
    VIX_E_CANNOT_START_READ_ONLY_VM = 3005,
    VIX_E_VM_NOT_RUNNING = 3006,
    VIX_E_VM_IS_RUNNING = 3007,
    VIX_E_CANNOT_CONNECT_TO_VM = 3008,
    VIX_E_POWEROP_SCRIPTS_NOT_AVAILABLE = 3009,
    VIX_E_NO_GUEST_OS_INSTALLED = 3010,
    VIX_E_VM_INSUFFICIENT_HOST_MEMORY = 3011,
    VIX_E_SUSPEND_ERROR = 3012,
    VIX_E_VM_NOT_ENOUGH_CPUS = 3013,
    VIX_E_HOST_USER_PERMISSIONS = 3014,
    VIX_E_GUEST_USER_PERMISSIONS = 3015,
    VIX_E_TOOLS_NOT_RUNNING = 3016,
    VIX_E_GUEST_OPERATIONS_PROHIBITED = 3017,
    VIX_E_ANON_GUEST_OPERATIONS_PROHIBITED = 3018,
    VIX_E_ROOT_GUEST_OPERATIONS_PROHIBITED = 3019,
    VIX_E_MISSING_ANON_GUEST_ACCOUNT = 3023,
    VIX_E_CANNOT_AUTHENTICATE_WITH_GUEST = 3024,
    VIX_E_UNRECOGNIZED_COMMAND_IN_GUEST = 3025,
    VIX_E_CONSOLE_GUEST_OPERATIONS_PROHIBITED = 3026,
    VIX_E_MUST_BE_CONSOLE_USER = 3027,
    VIX_E_VMX_MSG_DIALOG_AND_NO_UI = 3028,
    /* VIX_E_NOT_ALLOWED_DURING_VM_RECORDING        = 3029, Removed in version 1.11 */
    /* VIX_E_NOT_ALLOWED_DURING_VM_REPLAY           = 3030, Removed in version 1.11 */
    VIX_E_OPERATION_NOT_ALLOWED_FOR_LOGIN_TYPE = 3031,
    VIX_E_LOGIN_TYPE_NOT_SUPPORTED = 3032,
    VIX_E_EMPTY_PASSWORD_NOT_ALLOWED_IN_GUEST = 3033,
    VIX_E_INTERACTIVE_SESSION_NOT_PRESENT = 3034,
    VIX_E_INTERACTIVE_SESSION_USER_MISMATCH = 3035,
    /* VIX_E_UNABLE_TO_REPLAY_VM                    = 3039, Removed in version 1.11 */
    VIX_E_CANNOT_POWER_ON_VM = 3041,
    VIX_E_NO_DISPLAY_SERVER = 3043,
    /* VIX_E_VM_NOT_RECORDING                       = 3044, Removed in version 1.11 */
    /* VIX_E_VM_NOT_REPLAYING                       = 3045, Removed in version 1.11 */
    VIX_E_TOO_MANY_LOGONS = 3046,
    VIX_E_INVALID_AUTHENTICATION_SESSION = 3047,

    /* VM Errors */
    VIX_E_VM_NOT_FOUND = 4000,
    VIX_E_NOT_SUPPORTED_FOR_VM_VERSION = 4001,
    VIX_E_CANNOT_READ_VM_CONFIG = 4002,
    VIX_E_TEMPLATE_VM = 4003,
    VIX_E_VM_ALREADY_LOADED = 4004,
    VIX_E_VM_ALREADY_UP_TO_DATE = 4006,
    VIX_E_VM_UNSUPPORTED_GUEST = 4011,

    /* Property Errors */
    VIX_E_UNRECOGNIZED_PROPERTY = 6000,
    VIX_E_INVALID_PROPERTY_VALUE = 6001,
    VIX_E_READ_ONLY_PROPERTY = 6002,
    VIX_E_MISSING_REQUIRED_PROPERTY = 6003,
    VIX_E_INVALID_SERIALIZED_DATA = 6004,
    VIX_E_PROPERTY_TYPE_MISMATCH = 6005,

    /* Completion Errors */
    VIX_E_BAD_VM_INDEX = 8000,

    /* Message errors */
    VIX_E_INVALID_MESSAGE_HEADER = 10000,
    VIX_E_INVALID_MESSAGE_BODY = 10001,

    /* Snapshot errors */
    VIX_E_SNAPSHOT_INVAL = 13000,
    VIX_E_SNAPSHOT_DUMPER = 13001,
    VIX_E_SNAPSHOT_DISKLIB = 13002,
    VIX_E_SNAPSHOT_NOTFOUND = 13003,
    VIX_E_SNAPSHOT_EXISTS = 13004,
    VIX_E_SNAPSHOT_VERSION = 13005,
    VIX_E_SNAPSHOT_NOPERM = 13006,
    VIX_E_SNAPSHOT_CONFIG = 13007,
    VIX_E_SNAPSHOT_NOCHANGE = 13008,
    VIX_E_SNAPSHOT_CHECKPOINT = 13009,
    VIX_E_SNAPSHOT_LOCKED = 13010,
    VIX_E_SNAPSHOT_INCONSISTENT = 13011,
    VIX_E_SNAPSHOT_NAMETOOLONG = 13012,
    VIX_E_SNAPSHOT_VIXFILE = 13013,
    VIX_E_SNAPSHOT_DISKLOCKED = 13014,
    VIX_E_SNAPSHOT_DUPLICATEDDISK = 13015,
    VIX_E_SNAPSHOT_INDEPENDENTDISK = 13016,
    VIX_E_SNAPSHOT_NONUNIQUE_NAME = 13017,
    VIX_E_SNAPSHOT_MEMORY_ON_INDEPENDENT_DISK = 13018,
    VIX_E_SNAPSHOT_MAXSNAPSHOTS = 13019,
    VIX_E_SNAPSHOT_MIN_FREE_SPACE = 13020,
    VIX_E_SNAPSHOT_HIERARCHY_TOODEEP = 13021,
    // DEPRECRATED VIX_E_SNAPSHOT_RRSUSPEND                     = 13022,
    VIX_E_SNAPSHOT_NOT_REVERTABLE = 13024,

    /* Host Errors */
    VIX_E_HOST_DISK_INVALID_VALUE = 14003,
    VIX_E_HOST_DISK_SECTORSIZE = 14004,
    VIX_E_HOST_FILE_ERROR_EOF = 14005,
    VIX_E_HOST_NETBLKDEV_HANDSHAKE = 14006,
    VIX_E_HOST_SOCKET_CREATION_ERROR = 14007,
    VIX_E_HOST_SERVER_NOT_FOUND = 14008,
    VIX_E_HOST_NETWORK_CONN_REFUSED = 14009,
    VIX_E_HOST_TCP_SOCKET_ERROR = 14010,
    VIX_E_HOST_TCP_CONN_LOST = 14011,
    VIX_E_HOST_NBD_HASHFILE_VOLUME = 14012,
    VIX_E_HOST_NBD_HASHFILE_INIT = 14013,

    /* Disklib errors */
    VIX_E_DISK_INVAL = 16000,
    VIX_E_DISK_NOINIT = 16001,
    VIX_E_DISK_NOIO = 16002,
    VIX_E_DISK_PARTIALCHAIN = 16003,
    VIX_E_DISK_NEEDSREPAIR = 16006,
    VIX_E_DISK_OUTOFRANGE = 16007,
    VIX_E_DISK_CID_MISMATCH = 16008,
    VIX_E_DISK_CANTSHRINK = 16009,
    VIX_E_DISK_PARTMISMATCH = 16010,
    VIX_E_DISK_UNSUPPORTEDDISKVERSION = 16011,
    VIX_E_DISK_OPENPARENT = 16012,
    VIX_E_DISK_NOTSUPPORTED = 16013,
    VIX_E_DISK_NEEDKEY = 16014,
    VIX_E_DISK_NOKEYOVERRIDE = 16015,
    VIX_E_DISK_NOTENCRYPTED = 16016,
    VIX_E_DISK_NOKEY = 16017,
    VIX_E_DISK_INVALIDPARTITIONTABLE = 16018,
    VIX_E_DISK_NOTNORMAL = 16019,
    VIX_E_DISK_NOTENCDESC = 16020,
    VIX_E_DISK_NEEDVMFS = 16022,
    VIX_E_DISK_RAWTOOBIG = 16024,
    VIX_E_DISK_TOOMANYOPENFILES = 16027,
    VIX_E_DISK_TOOMANYREDO = 16028,
    VIX_E_DISK_RAWTOOSMALL = 16029,
    VIX_E_DISK_INVALIDCHAIN = 16030,
    VIX_E_DISK_KEY_NOTFOUND = 16052, // metadata key is not found
    VIX_E_DISK_SUBSYSTEM_INIT_FAIL = 16053,
    VIX_E_DISK_INVALID_CONNECTION = 16054,
    VIX_E_DISK_ENCODING = 16061,
    VIX_E_DISK_CANTREPAIR = 16062,
    VIX_E_DISK_INVALIDDISK = 16063,
    VIX_E_DISK_NOLICENSE = 16064,
    VIX_E_DISK_NODEVICE = 16065,
    VIX_E_DISK_UNSUPPORTEDDEVICE = 16066,
    VIX_E_DISK_CAPACITY_MISMATCH = 16067,
    VIX_E_DISK_PARENT_NOTALLOWED = 16068,
    VIX_E_DISK_ATTACH_ROOTLINK = 16069,

    /* Crypto Library Errors */
    VIX_E_CRYPTO_UNKNOWN_ALGORITHM = 17000,
    VIX_E_CRYPTO_BAD_BUFFER_SIZE = 17001,
    VIX_E_CRYPTO_INVALID_OPERATION = 17002,
    VIX_E_CRYPTO_RANDOM_DEVICE = 17003,
    VIX_E_CRYPTO_NEED_PASSWORD = 17004,
    VIX_E_CRYPTO_BAD_PASSWORD = 17005,
    VIX_E_CRYPTO_NOT_IN_DICTIONARY = 17006,
    VIX_E_CRYPTO_NO_CRYPTO = 17007,
    VIX_E_CRYPTO_ERROR = 17008,
    VIX_E_CRYPTO_BAD_FORMAT = 17009,
    VIX_E_CRYPTO_LOCKED = 17010,
    VIX_E_CRYPTO_EMPTY = 17011,
    VIX_E_CRYPTO_KEYSAFE_LOCATOR = 17012,

    /* Remoting Errors. */
    VIX_E_CANNOT_CONNECT_TO_HOST = 18000,
    VIX_E_NOT_FOR_REMOTE_HOST = 18001,
    VIX_E_INVALID_HOSTNAME_SPECIFICATION = 18002,

    /* Screen Capture Errors. */
    VIX_E_SCREEN_CAPTURE_ERROR = 19000,
    VIX_E_SCREEN_CAPTURE_BAD_FORMAT = 19001,
    VIX_E_SCREEN_CAPTURE_COMPRESSION_FAIL = 19002,
    VIX_E_SCREEN_CAPTURE_LARGE_DATA = 19003,

    /* Guest Errors */
    VIX_E_GUEST_VOLUMES_NOT_FROZEN = 20000,
    VIX_E_NOT_A_FILE = 20001,
    VIX_E_NOT_A_DIRECTORY = 20002,
    VIX_E_NO_SUCH_PROCESS = 20003,
    VIX_E_FILE_NAME_TOO_LONG = 20004,
    VIX_E_OPERATION_DISABLED = 20005,

    /* Tools install errors */
    VIX_E_TOOLS_INSTALL_NO_IMAGE = 21000,
    VIX_E_TOOLS_INSTALL_IMAGE_INACCESIBLE = 21001,
    VIX_E_TOOLS_INSTALL_NO_DEVICE = 21002,
    VIX_E_TOOLS_INSTALL_DEVICE_NOT_CONNECTED = 21003,
    VIX_E_TOOLS_INSTALL_CANCELLED = 21004,
    VIX_E_TOOLS_INSTALL_INIT_FAILED = 21005,
    VIX_E_TOOLS_INSTALL_AUTO_NOT_SUPPORTED = 21006,
    VIX_E_TOOLS_INSTALL_GUEST_NOT_READY = 21007,
    VIX_E_TOOLS_INSTALL_SIG_CHECK_FAILED = 21008,
    VIX_E_TOOLS_INSTALL_ERROR = 21009,
    VIX_E_TOOLS_INSTALL_ALREADY_UP_TO_DATE = 21010,
    VIX_E_TOOLS_INSTALL_IN_PROGRESS = 21011,
    VIX_E_TOOLS_INSTALL_IMAGE_COPY_FAILED = 21012,

    /* Wrapper Errors */
    VIX_E_WRAPPER_WORKSTATION_NOT_INSTALLED = 22001,
    VIX_E_WRAPPER_VERSION_NOT_FOUND = 22002,
    VIX_E_WRAPPER_SERVICEPROVIDER_NOT_FOUND = 22003,
    VIX_E_WRAPPER_PLAYER_NOT_INSTALLED = 22004,
    VIX_E_WRAPPER_RUNTIME_NOT_INSTALLED = 22005,
    VIX_E_WRAPPER_MULTIPLE_SERVICEPROVIDERS = 22006,

    /* FuseMnt errors */
    VIX_E_MNTAPI_MOUNTPT_NOT_FOUND = 24000,
    VIX_E_MNTAPI_MOUNTPT_IN_USE = 24001,
    VIX_E_MNTAPI_DISK_NOT_FOUND = 24002,
    VIX_E_MNTAPI_DISK_NOT_MOUNTED = 24003,
    VIX_E_MNTAPI_DISK_IS_MOUNTED = 24004,
    VIX_E_MNTAPI_DISK_NOT_SAFE = 24005,
    VIX_E_MNTAPI_DISK_CANT_OPEN = 24006,
    VIX_E_MNTAPI_CANT_READ_PARTS = 24007,
    VIX_E_MNTAPI_UMOUNT_APP_NOT_FOUND = 24008,
    VIX_E_MNTAPI_UMOUNT = 24009,
    VIX_E_MNTAPI_NO_MOUNTABLE_PARTITONS = 24010,
    VIX_E_MNTAPI_PARTITION_RANGE = 24011,
    VIX_E_MNTAPI_PERM = 24012,
    VIX_E_MNTAPI_DICT = 24013,
    VIX_E_MNTAPI_DICT_LOCKED = 24014,
    VIX_E_MNTAPI_OPEN_HANDLES = 24015,
    VIX_E_MNTAPI_CANT_MAKE_VAR_DIR = 24016,
    VIX_E_MNTAPI_NO_ROOT = 24017,
    VIX_E_MNTAPI_LOOP_FAILED = 24018,
    VIX_E_MNTAPI_DAEMON = 24019,
    VIX_E_MNTAPI_INTERNAL = 24020,
    VIX_E_MNTAPI_SYSTEM = 24021,
    VIX_E_MNTAPI_NO_CONNECTION_DETAILS = 24022,
    /* FuseMnt errors: Do not exceed 24299 */

    /* VixMntapi errors */
    VIX_E_MNTAPI_INCOMPATIBLE_VERSION = 24300,
    VIX_E_MNTAPI_OS_ERROR = 24301,
    VIX_E_MNTAPI_DRIVE_LETTER_IN_USE = 24302,
    VIX_E_MNTAPI_DRIVE_LETTER_ALREADY_ASSIGNED = 24303,
    VIX_E_MNTAPI_VOLUME_NOT_MOUNTED = 24304,
    VIX_E_MNTAPI_VOLUME_ALREADY_MOUNTED = 24305,
    VIX_E_MNTAPI_FORMAT_FAILURE = 24306,
    VIX_E_MNTAPI_NO_DRIVER = 24307,
    VIX_E_MNTAPI_ALREADY_OPENED = 24308,
    VIX_E_MNTAPI_ITEM_NOT_FOUND = 24309,
    VIX_E_MNTAPI_UNSUPPROTED_BOOT_LOADER = 24310,
    VIX_E_MNTAPI_UNSUPPROTED_OS = 24311,
    VIX_E_MNTAPI_CODECONVERSION = 24312,
    VIX_E_MNTAPI_REGWRITE_ERROR = 24313,
    VIX_E_MNTAPI_UNSUPPORTED_FT_VOLUME = 24314,
    VIX_E_MNTAPI_PARTITION_NOT_FOUND = 24315,
    VIX_E_MNTAPI_PUTFILE_ERROR = 24316,
    VIX_E_MNTAPI_GETFILE_ERROR = 24317,
    VIX_E_MNTAPI_REG_NOT_OPENED = 24318,
    VIX_E_MNTAPI_REGDELKEY_ERROR = 24319,
    VIX_E_MNTAPI_CREATE_PARTITIONTABLE_ERROR = 24320,
    VIX_E_MNTAPI_OPEN_FAILURE = 24321,
    VIX_E_MNTAPI_VOLUME_NOT_WRITABLE = 24322,

    /* Success on operation that completes asynchronously */
    VIX_ASYNC = 25000,

    /* Async errors */
    VIX_E_ASYNC_MIXEDMODE_UNSUPPORTED = 26000,

    /* Network Errors */
    VIX_E_NET_HTTP_UNSUPPORTED_PROTOCOL = 30001,
    VIX_E_NET_HTTP_URL_MALFORMAT = 30003,
    VIX_E_NET_HTTP_COULDNT_RESOLVE_PROXY = 30005,
    VIX_E_NET_HTTP_COULDNT_RESOLVE_HOST = 30006,
    VIX_E_NET_HTTP_COULDNT_CONNECT = 30007,
    VIX_E_NET_HTTP_HTTP_RETURNED_ERROR = 30022,
    VIX_E_NET_HTTP_OPERATION_TIMEDOUT = 30028,
    VIX_E_NET_HTTP_SSL_CONNECT_ERROR = 30035,
    VIX_E_NET_HTTP_TOO_MANY_REDIRECTS = 30047,
    VIX_E_NET_HTTP_TRANSFER = 30200,
    VIX_E_NET_HTTP_SSL_SECURITY = 30201,
    VIX_E_NET_HTTP_GENERIC = 30202,
};
typedef enum {
    VIXDISKLIB_ADAPTER_IDE = 1,
    VIXDISKLIB_ADAPTER_SCSI_BUSLOGIC = 2,
    VIXDISKLIB_ADAPTER_SCSI_LSILOGIC = 3,
    VIXDISKLIB_ADAPTER_UNKNOWN = 256
} VixDiskLibAdapterType;
typedef enum {
    VIXDISKLIB_CRED_UID = 1,       // use userid password
    VIXDISKLIB_CRED_SESSIONID = 2, // http session id
    VIXDISKLIB_CRED_TICKETID = 3,  // vim ticket id
    VIXDISKLIB_CRED_SSPI = 4,      // Windows only - use current thread credentials.
    VIXDISKLIB_CRED_UNKNOWN = 256
} VixDiskLibCredType;
typedef struct {
    char *vmxSpec;    // URL like spec of the VM.
    char *serverName; // Name or IP address of VC / ESX.
    char *thumbPrint; // SSL Certificate thumb print.
    long privateUse;  // This value is ignored.
    VixDiskLibCredType credType;

    union VixDiskLibCreds {
        struct VixDiskLibUidPasswdCreds {
            char *userName; // User id and password on the
            char *password; // VC/ESX host.
        } uid;
        struct VixDiskLibSessionIdCreds { // Not supported in 1.0
            char *cookie;
            char *userName;
            char *key;
        } sessionId;
        struct VixDiskLibTicketIdCreds *ticketId; // Internal use only.
    } creds;

    uint32_t port;        // port to use for authenticating with VC/ESXi host
    uint32_t nfcHostPort; // port to use for establishing NFC connection to ESXi host
    char *vimApiVer;      // VIM API version to use, private
} VixDiskLibConnectParams;

typedef struct {
    uint32 cylinders;
    uint32 heads;
    uint32 sectors;
} VixDiskLibGeometry;
typedef struct {
    VixDiskLibGeometry biosGeo;        // BIOS geometry for booting and partitioning
    VixDiskLibGeometry physGeo;        // physical geometry
    VixDiskLibSectorType capacity;     // total capacity in sectors
    VixDiskLibAdapterType adapterType; // adapter type
    int numLinks;                      // number of links (i.e. base disk + redo logs)
    char *parentFileNameHint;          // parent file for a redo log
    char *uuid;                        // disk UUID
} VixDiskLibInfo;

typedef struct {
    uint64_t offset;
    uint64_t length;
} VixDiskLibBlock;

typedef struct {
    uint32_t numBlocks;
    VixDiskLibBlock blocks[1];
} VixDiskLibBlockList;

using VixDiskLibConnection = struct VixDiskLibConnectParam *;
typedef struct VixDiskLibHandleStruct VixDiskLibHandleStruct;
typedef VixDiskLibHandleStruct *VixDiskLibHandle;

typedef VixError (*VixDiskLibInitFunc)(
    uint32, uint32, VixDiskLibGenericLogFunc, VixDiskLibGenericLogFunc, VixDiskLibGenericLogFunc, const char *);
typedef VixError (*VixDiskLibInitExFunc)(uint32, uint32, VixDiskLibGenericLogFunc, VixDiskLibGenericLogFunc,
    VixDiskLibGenericLogFunc, const char *, const char *);
typedef void (*VixDiskLibExitFunc)();
typedef VixError (*VixDiskLibConnectFunc)(const VixDiskLibConnectParams *, VixDiskLibConnection *);
typedef VixError (*VixDiskLibConnectExFunc)(
    const VixDiskLibConnectParams *, Bool, const char *, const char *, VixDiskLibConnection *);
typedef VixError (*VixDiskLibDisconnectFunc)(VixDiskLibConnection);
typedef VixError (*VixDiskLibPrepareForAccessFunc)(const VixDiskLibConnectParams *, const char *);
typedef VixError (*VixDiskLibEndAccessFunc)(const VixDiskLibConnectParams *, const char *);
typedef VixError (*VixDiskLibOpenFunc)(const VixDiskLibConnection, const char *, uint32, VixDiskLibHandle *);
typedef VixError (*VixDiskLibCloseFunc)(VixDiskLibHandle);
typedef VixError (*VixDiskLibGetInfoFunc)(VixDiskLibHandle, VixDiskLibInfo **);
typedef void (*VixDiskLibFreeInfoFunc)(VixDiskLibInfo *);
typedef VixError (*VixDiskLibReadFunc)(VixDiskLibHandle, VixDiskLibSectorType, VixDiskLibSectorType, uint8 *);
typedef VixError (*VixDiskLibWriteFunc)(VixDiskLibHandle, VixDiskLibSectorType, VixDiskLibSectorType, const uint8 *);
typedef char *(*VixDiskLibGetErrorTextFunc)(VixError, const char *);
typedef void (*VixDiskLibFreeErrorTextFunc)(char *);
typedef const char *(*VixDiskLibListTransportModesFunc)();
typedef VixError (*VixDiskLibCleanupFunc)(const VixDiskLibConnectParams *, uint32 *, uint32 *);
typedef const char *(*VixDiskLibGetTransportModeFunc)(VixDiskLibHandle);
typedef VixError (*vixDiskLibQueryAllocateBlocksFun)(VixDiskLibHandle, VixDiskLibSectorType,
    VixDiskLibSectorType, VixDiskLibSectorType, VixDiskLibBlockList **);
typedef VixError (*vixDiskLib_FreeBlockListFun) (VixDiskLibBlockList *block_list);

struct VMwareDiskOperations {
    VMwareDiskOperations()
    {
        vixDiskLibInit = NULL;
        vixDiskLibInitEx = NULL;
        vixDiskLibExit = NULL;
        vixDiskLibConnect = NULL;
        vixDiskLibConnectEx = NULL;
        vixDiskLibDisconnect = NULL;

        vixDiskLibPrepareForAccess = NULL;
        vixDiskLibEndAccess = NULL;

        vixDiskLibOpen = NULL;
        vixDiskLibClose = NULL;

        vixDiskLibGetInfo = NULL;
        vixDiskLibFreeInfo = NULL;

        vixDiskLibRead = NULL;
        vixDiskLibWrite = NULL;

        vixDiskLibGetErrorText = NULL;
        vixDiskLibFreeErrorText = NULL;

        vixDiskLibListTransportModes = NULL;
        vixDiskLibCleanup = NULL;
        vixDiskLibGetTransportMode = NULL;
        vixDiskLibQueryAllocateBlocks = NULL;
        vixDiskLib_FreeBlockList = NULL;
    }

    ~VMwareDiskOperations()
    {}

    VixDiskLibInitFunc vixDiskLibInit;
    VixDiskLibInitExFunc vixDiskLibInitEx;
    VixDiskLibExitFunc vixDiskLibExit;
    VixDiskLibConnectFunc vixDiskLibConnect;
    VixDiskLibConnectExFunc vixDiskLibConnectEx;
    VixDiskLibDisconnectFunc vixDiskLibDisconnect;
    VixDiskLibPrepareForAccessFunc vixDiskLibPrepareForAccess;
    VixDiskLibEndAccessFunc vixDiskLibEndAccess;
    VixDiskLibOpenFunc vixDiskLibOpen;
    VixDiskLibCloseFunc vixDiskLibClose;
    VixDiskLibGetInfoFunc vixDiskLibGetInfo;
    VixDiskLibFreeInfoFunc vixDiskLibFreeInfo;
    VixDiskLibReadFunc vixDiskLibRead;
    VixDiskLibWriteFunc vixDiskLibWrite;
    VixDiskLibGetErrorTextFunc vixDiskLibGetErrorText;
    VixDiskLibFreeErrorTextFunc vixDiskLibFreeErrorText;
    VixDiskLibListTransportModesFunc vixDiskLibListTransportModes;
    VixDiskLibCleanupFunc vixDiskLibCleanup;
    VixDiskLibGetTransportModeFunc vixDiskLibGetTransportMode;
    vixDiskLibQueryAllocateBlocksFun vixDiskLibQueryAllocateBlocks;
    vixDiskLib_FreeBlockListFun vixDiskLib_FreeBlockList;
};

struct VddkConnectParams {
    std::string vmSpec;      // URL like spec of the VM.
    std::string serverName;  // Name or IP address of VC / ESX.
    std::string thumbPrint;  // SSL Certificate thumb print.d.
    std::string userName;    // User id and password on the
    std::string password;    // VC/ESX host.
    std::string key;
    mp_uint64 port;
    std::string vmMoRef; // vm moref
    std::string vmSnapshotRef; // vm snapshot moref
    bool openMode; // true:read, false: read&write
    bool bSupportSAN; // default value is true for backup operation
    int protectType; // 0-backup, 1-recovery
    int hostagentSystemVirt; // 0-physical, 1-virtual
    std::string transportMode; // only can be nbd or nbdssl
    std::string diskType; // disktype
    VddkConnectParams() : port(0), openMode(true), bSupportSAN(true)
    {}

    ~VddkConnectParams()
    {
        for (std::string::iterator it = password.begin(); it != password.end(); ++it) {
            (*it) = (std::string::value_type)'\0';
        }
    }
};

#endif
