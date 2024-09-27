#ifndef _SOCKET_H
#define _SOCKET_H

#include "driver.h"
#include "TdiKrnl.h"
#include "tdi.h"
 
#define STATUS_NET_ERROR            ((NTSTATUS)0xe0000001L)

typedef struct _ENDPOINT_REQUEST_CONTEXT
{
	UCHAR               magic[4];
	PRKEVENT            event;
	PIO_STATUS_BLOCK    io_status;
	PMDL                mdl;
	LONG                length;
	PVOID               address;
	BOOLEAN             complete_status;
} ENDPOINT_REQUEST_CONTEXT, *PENDPOINT_REQUEST_CONTEXT;

#define ENDPOINT_REQUEST_CONTEXT_ sizeof(ENDPOINT_REQUEST_CONTEXT)

//
// ���ݽṹ����
//
typedef struct _DR_ADDRESS 
{
	LONG ref_count;// �ö�������ü���
	HANDLE h_address;// ��ַ������
	PFILE_OBJECT file_object;// �����ַ�ļ�����
	PIRP irp;
} DR_ADDRESS, *PDR_ADDRESS;
#define DR_ADDRESS_  sizeof(DR_ADDRESS)

typedef struct _DR_ENDPOINT 
{
	PDR_ADDRESS address;// ��ַ�����ָ��
	HANDLE h_endpoint;// ���Ӷ˵���
	PFILE_OBJECT file_object;// ���Ӷ˵��ļ�����
	IO_STATUS_BLOCK io_status;
	TDI_PROVIDER_INFO provider_info;
	KEVENT event;
} DR_ENDPOINT, *PDR_ENDPOINT;
#define DR_ENDPOINT_  sizeof(DR_ENDPOINT)

NTSTATUS
	DrOpenTransportAddress(
	IN PWSTR                device_name,
	OUT PDR_ADDRESS         address
	);

VOID
	DrCloseTransportAddress(IN PDR_ADDRESS address);

NTSTATUS
	DrOpenConnectionEndpoint(
	IN PWSTR            device_name,
	IN PDR_ADDRESS      address,
	OUT PDR_ENDPOINT    end_point,
	IN PVOID            context
	);

VOID
	DrCloseConnectionEndpoint(IN PDR_ENDPOINT end_point);

NTSTATUS
	DrConnect(
	IN PDR_ENDPOINT end_point,
	uint32_t ip, 
	uint32_t port
	);

NTSTATUS
	DrDisconnect(
	IN PDR_ENDPOINT end_point
	);

NTSTATUS
	DrSendSync(
	IN PDR_ENDPOINT endpoint,
	IN PMDL         mdl,
	IN ULONG        flags
	);

NTSTATUS
	DrReceiveSync(
	IN PDR_ENDPOINT endpoint,
	IN PMDL         mdl,
	IN ULONG        flags,
	OUT PULONG      info_size
	);

NTSTATUS
	DrReceiveAsync(
	IN PDR_ENDPOINT  endpoint,
	IN PMDL          mdl,
	IN ULONG         flags,
	IN PKEVENT       event,
	IN PIO_STATUS_BLOCK io_status
	);

PVOID
	DrMalloc(
	IN POOL_TYPE pool_type,
	IN SIZE_T size
	);

VOID
	DrUnlockAndFreeMdl(
	IN PMDL mdl
	);

#endif