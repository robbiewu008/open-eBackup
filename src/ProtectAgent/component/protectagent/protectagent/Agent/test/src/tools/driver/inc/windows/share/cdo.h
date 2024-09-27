#pragma once
#include "driver.h"
#include "group_mem.h"




_Dispatch_type_(IRP_MJ_SHUTDOWN)
DRIVER_DISPATCH CdoShutdown;
_Dispatch_type_(IRP_MJ_CREATE)
DRIVER_DISPATCH CdoCreate;
_Dispatch_type_(IRP_MJ_CLOSE)
DRIVER_DISPATCH CdoClose;
_Dispatch_type_(IRP_MJ_CLEANUP)
DRIVER_DISPATCH CdoCleanUp;
_Dispatch_type_(IRP_MJ_DEVICE_CONTROL)
DRIVER_DISPATCH CdoDeviceCtl;
_Dispatch_type_(IRP_MJ_PNP)
DRIVER_DISPATCH CdoDispatchPnp;

_Dispatch_type_(IRP_MJ_POWER)
DRIVER_DISPATCH CdoDispatchPower;

DRIVER_DISPATCH CdoDispatchAny;
DRIVER_ADD_DEVICE CdoAddDevice;

BOOLEAN IsControlDev(IN PDEVICE_OBJECT DeviceObject);

BOOLEAN IsControlPdo(IN PDEVICE_OBJECT DeviceObject);
GroupInfo* GetGroupInfoByCdo();
VOID HoldCdoRemoveLock();
VOID ReleaseCdoRemoveLock();
PVOID GetCdoMemInfo();
PDRIVER_OBJECT GetCdoDriverObject();