MessageIdTypedef=NTSTATUS

SeverityNames=(Success=0x0:STATUS_SEVERITY_SUCCESS
               Informational=0x1:STATUS_SEVERITY_INFORMATIONAL
               Warning=0x2:STATUS_SEVERITY_WARNING
               Error=0x3:STATUS_SEVERITY_ERROR
              )

FacilityNames=(System=0x0
               RpcRuntime=0x2:FACILITY_RPC_RUNTIME
               RpcStubs=0x3:FACILITY_RPC_STUBS
               Io=0x4:FACILITY_IO_ERROR_CODE
               DRDriver=0x5:FACILITY_DEVICE_ERROR_CODE
              )


MessageId=0x0001 Facility=DRDriver Severity=Warning SymbolicName=LOG_QUERY_STOP_DRIVER_IN_USE
Language=English
Device cannot stop under the current state.
.

MessageId=0x0002 Facility=DRDriver Severity=Warning SymbolicName=LOG_QUERY_STOP_LOWER_IN_USE
Language=English
Device cannot stop, lower devices are in use.
.

MessageId=0x0003 Facility=DRDriver Severity=Informational SymbolicName=LOG_DRIVER_LOADED
Language=English
DRDriver is loaded.
.

MessageId=0x0004 Facility=DRDriver Severity=Warning SymbolicName=LOG_NO_VOL_PROTECTED
Language=English
Volume %2 is not being protected.
.

MessageId=0x0005 Facility=DRDriver Severity=Error SymbolicName=LOG_EMPTY_VOL_QUEUE
Language=English
Volume queue is empty.
.

MessageId=0x0006 Facility=DRDriver Severity=Error SymbolicName=LOG_OPEN_PERIST_FILE_FAIL
Language=English
Failed to open persist file.
.

MessageId=0x0007 Facility=DRDriver Severity=Informational SymbolicName=LOG_SYNC_START_UNDER_STATE
Language=English
Replication started.
.

MessageId=0x0008 Facility=DRDriver Severity=Informational SymbolicName=LOG_QUERY_STOP_SUCCESS
Language=English
DRDriver can be unloaded.
.

MessageId=0x0009 Facility=DRDriver Severity=Error SymbolicName=LOG_CREATE_CDO_FAIL
Language=English
Failed to create control device.
.

MessageId=0x000a Facility=DRDriver Severity=Error SymbolicName=LOG_CREATE_CDO_SYM_LINK_FAIL
Language=English
Failed to create symbolic link name %2 for control device.
.

MessageId=0x000b Facility=DRDriver Severity=Error SymbolicName=LOG_CREATE_CDO_GROUP_INFO_FAIL
Language=English
Failed to create protection information.
.

MessageId=0x000c Facility=DRDriver Severity=Error SymbolicName=LOG_ATTACH_CDO_FAIL
Language=English
Failed to attach control device.
.

MessageId=0x000d Facility=DRDriver Severity=Error SymbolicName=LOG_CDO_START_LOWER_FAIL
Language=English
Control device cannot start because lower devices failed.
.

MessageId=0x000e Facility=DRDriver Severity=Error SymbolicName=LOG_CDO_START_INIT_REG_FAIL
Language=English
Control device cannot start because registry information initialization failed.
.

MessageId=0x000f Facility=DRDriver Severity=Error SymbolicName=LOG_CDO_START_SEND_THREAD_FAIL
Language=English
Failed to create sending thread.
.

MessageId=0x0010 Facility=DRDriver Severity=Error SymbolicName=LOG_CDO_START_FIRST_SHUTDOWN_FAIL
Language=English
Failed to register first time shutdown notification.
.

MessageId=0x0011 Facility=DRDriver Severity=Error SymbolicName=LOG_CDO_START_SECOND_SHUTDOWN_FAIL
Language=English
Failed to register second time shutdown notification.
.

MessageId=0x0012 Facility=DRDriver Severity=Error SymbolicName=LOG_READ_PERSIST_STATE_FAIL
Language=English
Failed to read persist state.
.

MessageId=0x0013 Facility=DRDriver Severity=Error SymbolicName=LOG_READ_REG_SELECTION_FAIL
Language=English
Failed to read registry selection.
.

MessageId=0x0014 Facility=DRDriver Severity=Warning SymbolicName=LOG_PERSIST_STATE_FAIL
Language=English
Persist state failed, DRDriver will start from an Initial Sync or a Resync.
.

MessageId=0x0015 Facility=DRDriver Severity=Error SymbolicName=LOG_DESTROY_SEND_THREAD_FAIL
Language=English
Failed to destroy sending thread.
.

MessageId=0x0016 Facility=DRDriver Severity=Informational SymbolicName=LOG_CDO_REMOVED
Language=English
Control device was removed.
.

MessageId=0x0017 Facility=DRDriver Severity=Informational SymbolicName=LOG_PERSIST_DATA_ABORT_STATE
Language=English
DRDriver will not persist data under the current state.
.

MessageId=0x0018 Facility=DRDriver Severity=Error SymbolicName=LOG_PERSIST_DATASET_ID_FAIL
Language=English
Failed to save dataset Id.
.

MessageId=0x0019 Facility=DRDriver Severity=Informational SymbolicName=LOG_CDO_STOPPED
Language=English
Control device was stopped.
.

MessageId=0x001a Facility=DRDriver Severity=Error SymbolicName=LOG_PERSIST_GET_SYS_VOL_FAIL
Language=English
Failed to get system volume.
.

MessageId=0x001b Facility=DRDriver Severity=Error SymbolicName=LOG_PERSIST_GET_NTFS_VOL_DATA_FAIL
Language=English
Failed to get NTFS volume information.
.

MessageId=0x001c Facility=DRDriver Severity=Error SymbolicName=LOG_PERSIST_WRITE_PERSIST_STATE_FAIL
Language=English
Failed to get write persist state.
.

MessageId=0x001d Facility=DRDriver Severity=Error SymbolicName=LOG_PERSIST_CREATE_PERSIST_FILE_FAIL
Language=English
Failed to create persist file.
.

MessageId=0x001e Facility=DRDriver Severity=Error SymbolicName=LOG_PERSIST_REMOVE_COMPRESSION_FAIL
Language=English
Failed to remove compression attribute from persist file.
.

MessageId=0x001f Facility=DRDriver Severity=Error SymbolicName=LOG_PERSIST_SET_PERSIST_FILE_SIZE_FAIL
Language=English
Failed to set persist file size.
.

MessageId=0x0020 Facility=DRDriver Severity=Error SymbolicName=LOG_PERSIST_CREATE_FILE_EXTENT_FAIL
Language=English
Failed to create persist file extents.
.

MessageId=0x0021 Facility=DRDriver Severity=Informational SymbolicName=LOG_PERSIST_SHUTDOWN_PREPARE_FINISH
Language=English
Succeeded to preare for shutdown.
.

MessageId=0x0022 Facility=DRDriver Severity=Error SymbolicName=LOG_PERSIST_NO_VOL_BITMAP
Language=English
Failed to find persist bitmap for disk %2.
.

MessageId=0x0023 Facility=DRDriver Severity=Informational SymbolicName=LOG_PERSIST_STATE_SUCCEED
Language=English
Persist state is healthy.
.

MessageId=0x0024 Facility=DRDriver Severity=Informational SymbolicName=LOG_PERSIST_STATE_ABANDON
Language=English
Persist state and persist data is ignored under the current state.
.

MessageId=0x0025 Facility=DRDriver Severity=Error SymbolicName=LOG_PERSIST_READ_VOL_BITMAP_FAIL
Language=English
Failed to read persist bitmap for disk %2.
.

MessageId=0x0026 Facility=DRDriver Severity=Informational SymbolicName=LOG_PERSIST_READ_VOL_BITMAP_FINISH
Language=English
Succeeded to get persist bitmap for disk %2.
.

MessageId=0x0027 Facility=DRDriver Severity=Error SymbolicName=LOG_PERSIST_WRONG_HEADER_MAGIC
Language=English
Wrong magic code of persist header.
.

MessageId=0x0028 Facility=DRDriver Severity=Error SymbolicName=LOG_PERSIST_WRONG_HEADER_START_SAFE
Language=English
Persist header does not indicate a complete shutdown.
.

MessageId=0x0029 Facility=DRDriver Severity=Error SymbolicName=LOG_PERSIST_WRONG_HEADER_SELECTION
Language=English
Registry selection has changed.
.

MessageId=0x002a Facility=DRDriver Severity=Error SymbolicName=LOG_PERSIST_OPEN_PERSIST_FILE_FAIL
Language=English
Failed to open persist file.
.

MessageId=0x002b Facility=DRDriver Severity=Informational SymbolicName=LOG_PERSIST_OPEN_PERSIST_FINISHED
Language=English
Succeeded to open persist file.
.
