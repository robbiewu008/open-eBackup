#pragma once
#include "driver.h"

extern "C"
{
	DRIVER_INITIALIZE DriverEntry;
	DRIVER_ADD_DEVICE AddDevice;
	DRIVER_UNLOAD DriverUnload;

	_Dispatch_type_(IRP_MJ_PNP)
		DRIVER_DISPATCH DispatchPnp;
	_Dispatch_type_(IRP_MJ_WRITE)
		DRIVER_DISPATCH DispatchWrite;

	DRIVER_DISPATCH StartDevice;
	DRIVER_DISPATCH RemoveDevice;

	IO_COMPLETION_ROUTINE IoCompletion;

	NTSTATUS DirectIoControl(PDEVICE_OBJECT DevObj, ULONG CtlCode, PVOID Buffer, ULONG Size);
}
