#pragma once
#include "driver.h"

extern "C"
{
	DRIVER_INITIALIZE DriverEntry;
	DRIVER_ADD_DEVICE AddDevice;
	DRIVER_UNLOAD DriverUnload;

	_Dispatch_type_(IRP_MJ_POWER)
	DRIVER_DISPATCH DispatchPower;
	_Dispatch_type_(IRP_MJ_PNP)
	DRIVER_DISPATCH DispatchPnp;
	_Dispatch_type_(IRP_MJ_WRITE)
	DRIVER_DISPATCH DispatchWrite;
	_Dispatch_type_(IRP_MJ_DEVICE_CONTROL)
	DRIVER_DISPATCH DispatchDeviceIOControl;
	_Dispatch_type_(IRP_MJ_SHUTDOWN)
	DRIVER_DISPATCH DispatchShutdown;
	_Dispatch_type_(IRP_MJ_CREATE)
	DRIVER_DISPATCH DispatchCreate;
	_Dispatch_type_(IRP_MJ_CLOSE)
	DRIVER_DISPATCH DispatchClose;
	_Dispatch_type_(IRP_MJ_CLEANUP)
	DRIVER_DISPATCH DispatchCleanUp;

	DRIVER_DISPATCH DispatchAny;
	DRIVER_DISPATCH StartDevice;
	DRIVER_DISPATCH RemoveDevice;

	DRIVER_DISPATCH ForwardIrpSynchronous;
	DRIVER_DISPATCH SendToNextDriver;
	DRIVER_DISPATCH SendToNextDriverAndPend;
	IO_COMPLETION_ROUTINE IoCompletion;
	IO_COMPLETION_ROUTINE IrpCompletion;

	ULONG GetDeviceTypeToUse(PDEVICE_OBJECT pdo);
	NTSTATUS CompleteRequest(IN PIRP Irp, IN NTSTATUS status, IN ULONG_PTR info);
	NTSTATUS DirectIoControl(PDEVICE_OBJECT DevObj, ULONG CtlCode, PVOID InputBuffer, ULONG InputSize, PVOID OutputBuffer, ULONG OutputSize);
	NTSTATUS DirectGetPdo(PDEVICE_OBJECT DevObj, PDEVICE_OBJECT* pdo);
}
