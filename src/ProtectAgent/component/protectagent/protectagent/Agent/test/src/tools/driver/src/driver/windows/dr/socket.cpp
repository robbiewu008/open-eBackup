#include "socket.h"

#include "wpp_trace.h"
#include "socket.tmh"



//ȫ�ֵ�TCP�豸����
PDEVICE_OBJECT g_tcp_device_object = NULL;

PVOID
	DrMalloc(
	IN POOL_TYPE pool_type,
	IN SIZE_T size
	)
{
	return ExAllocatePoolWithTag(pool_type, size, SOCKET_ALLOC_TAG);
}

/*++

��������:

    ���������ͷ�һ��MDL

����:

    mdl : Ҫ�������ͷŵ�MDL��ַ

����ֵ:

    ��

--*/
VOID
DrUnlockAndFreeMdl(
    IN PMDL mdl
    )
{
    MmUnlockPages(mdl);//��ҳ�ڴ���Ҫ����
    IoFreeMdl(mdl);
}

/*++
��������:

    ����PFILE_FULL_EA_INFORMATION�ṹ

����:

    name_length:��չ����(EA)���ֵĳ���
    name: EA������
    value_length: EAֵ�ĳ���
    value: EAֵ
    p_buffer: �����EA Bufferָ��
    buffer_length: EA Buffer�Ĵ�С

����ֵ:

    STATUS_SUCCESS
    STATUS_INSUFFICIENT_RESOURCES

--*/
NTSTATUS
DrBuildEaBuffer(
    IN  ULONG                       name_length,
    IN  PVOID                       name,
    IN  ULONG                       value_length,
    IN  PVOID                       value,
    OUT PFILE_FULL_EA_INFORMATION   *p_buffer,
    OUT PULONG                      buffer_length
    )
{
    PFILE_FULL_EA_INFORMATION   ea_buffer = NULL;
    NTSTATUS                    status = STATUS_SUCCESS;
    //
    //  �õ�EA Buffer�ĳ���,1ΪEaName����ַ���������־
    //   
    *buffer_length = FIELD_OFFSET(FILE_FULL_EA_INFORMATION, EaName[0]) +
                       name_length + 1 +
                       value_length;
    //
    //  ����EA Buffer,���ڴ���DrBuildEaBuffer�ĵ��������ͷ�
    //   
    ea_buffer = (PFILE_FULL_EA_INFORMATION)ExAllocatePoolWithTag(PagedPool, *buffer_length, SOCKET_ALLOC_TAG);
    if( NULL == ea_buffer )
	{
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Alloc EaBuffer failed");
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto out;
    }

    ea_buffer->NextEntryOffset = 0;
    ea_buffer->Flags = 0;
    ea_buffer->EaNameLength = (UCHAR)name_length;
    ea_buffer->EaValueLength = (USHORT)value_length;

    RtlCopyMemory(ea_buffer->EaName, name, ea_buffer->EaNameLength + 1);

    if( value_length && value )
	{
       RtlCopyMemory(&ea_buffer->EaName[name_length + 1], value, value_length);
    }
    *p_buffer = ea_buffer;
out:
    return status;
}

/*++

��������:

    �õ�PTRANSPORT_ADDRESS�ṹ��ʵ�ʴ�С

����:

    transport_address: ָ��PTRANSPORT_ADDRESS�ṹ������ָ��

����ֵ:

    transport_address��ʵ�ʴ�С

--*/
ULONG DrGetTransportAddressLength(
    IN PTRANSPORT_ADDRESS transport_address
    )
{
    LONG        i;
    ULONG       size = 0;
    PTA_ADDRESS address = NULL;

    if(transport_address)
	{
        size = FIELD_OFFSET(TRANSPORT_ADDRESS, Address) +
            FIELD_OFFSET(TA_ADDRESS, Address) * transport_address->TAAddressCount;

        address = (PTA_ADDRESS)(transport_address->Address);
        for (i = 0; i < transport_address->TAAddressCount; i++)
		{
            size += address->AddressLength;
            address = (PTA_ADDRESS) ((PCHAR)address + FIELD_OFFSET(TA_ADDRESS, Address) + address->AddressLength);
        }
    }
    return size;
}

/*++

��������:

    �رմ����ַ����

����:

    address:     ָ��һ��DR_ADDRESS�ṹ��, ����Ҫ�رմ����ַ����

����ֵ:

    ��

--*/
VOID
DrCloseTransportAddress(IN PDR_ADDRESS address)
{
	if (NULL == address)
	{
		return;
	}
    //
    // �ͷ�ͨ��IRP
    //
    if (address->irp != NULL) 
	{
        IoFreeIrp(address->irp);
        address->irp = NULL;
    }
    //
    // �����Ե�ַ���������,ɾ������
    //
    if (address->file_object != NULL)
	{
        ObDereferenceObject(address->file_object);
        address->file_object = NULL;
    }
    //
    // �رյ�ַ������
    //
    if(address->h_address != NULL)
	{
        ZwClose(address->h_address);
        address->h_address = NULL;
    }
}

/*++
��������:

    Ϊ����TDI���Ӵ�һ�������ַ����

����:

    device_name : ������L"\\Device\\Tcp"
    transport_address:  ָ��һ��TRANSPORT_ADDRESS �ṹ��,
                                        ����Ҫ�򿪵ı��ص�ַ
    address:     ָ��һ��DR_ADDRESS�ṹ��,
                               �����ʼ��Ϊ�Ѵ򿪵Ĵ����ַ����

����ֵ:

    STATUS_SUCCESS  �ɹ�
    DrBuildEaBuffer()ʧ��ֵ
    STATUS_INCEFENICENT_RESOURCES
    ZwCreateFile() ʧ��ֵ
    ObReferenceObjectByHandle() ʧ��ֵ,����:
        STATUS_OBJECT_TYPE_MISMATCH
        STATUS_ACCESS_DENIED
        STATUS_INVALID_HANDLE

--*/
NTSTATUS
DrOpenTransportAddress(
    IN PWSTR                transport_device_name,
	OUT PDR_ADDRESS         address
)
{
    NTSTATUS                    status = STATUS_SUCCESS;
    ULONG                       address_length = 0;
    UNICODE_STRING              device_name;
    PFILE_FULL_EA_INFORMATION   address_ea = NULL;
    ULONG                       buffer_length = 0;
    OBJECT_ATTRIBUTES           object_attr;
    IO_STATUS_BLOCK             io_status;
	TRANSPORT_ADDRESS* pTransport_address = NULL;

	if (NULL == transport_device_name || NULL == address)
	{
		return STATUS_UNSUCCESSFUL;
	}

	RtlZeroMemory(&io_status, sizeof(IO_STATUS_BLOCK));

	pTransport_address = (TRANSPORT_ADDRESS*)ExAllocatePoolWithTag(PagedPool, sizeof(TRANSPORT_ADDRESS) + sizeof(TDI_ADDRESS_IP), SOCKET_ALLOC_TAG);
	if (pTransport_address == NULL)
	{
		return STATUS_INSUFFICIENT_RESOURCES;
	}

	pTransport_address->TAAddressCount = 1;
	pTransport_address->Address->AddressType = TDI_ADDRESS_TYPE_IP;
	pTransport_address->Address->AddressLength = sizeof(TDI_ADDRESS_IP);
	TDI_ADDRESS_IP *pTdiAddressIp = (TDI_ADDRESS_IP *)&pTransport_address->Address[0].Address;
	RtlZeroMemory(pTdiAddressIp, sizeof(TDI_ADDRESS_IP)); 

    address_length = DrGetTransportAddressLength(pTransport_address);
    RtlInitUnicodeString(&device_name, transport_device_name);
    //
    // Ϊָ���Ĵ����ַ����һ��EA Buffer
    //
    status = DrBuildEaBuffer(
            TDI_TRANSPORT_ADDRESS_LENGTH,
            TdiTransportAddress,
            address_length,
			pTransport_address,
            &address_ea,
            &buffer_length
            );

    if(status != STATUS_SUCCESS) 
	{
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Build EaBuffer failed, error %!STATUS!", status);
        goto out;
    }

    InitializeObjectAttributes(
        &object_attr,
        &device_name,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL
        );
    //
    // �򿪴����ַ����
    //
    status = ZwCreateFile(
                &address->h_address,
                GENERIC_READ | GENERIC_WRITE | SYNCHRONIZE,
                &object_attr,
                &io_status,
                0,
                FILE_ATTRIBUTE_NORMAL,
                FILE_SHARE_READ,
                FILE_OPEN_IF,
                0,
                address_ea,
                buffer_length
                );
    if(!NT_SUCCESS(status))
	{
		if (STATUS_OBJECT_NAME_NOT_FOUND != status)
		{
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Create socket failed, error %!STATUS!", status);
		}
        goto out;
    }
    status = ObReferenceObjectByHandle(
            address->h_address,
            FILE_ANY_ACCESS,
            NULL,
            KernelMode,
            (PVOID *)&address->file_object,
            NULL
            );
    if(status != STATUS_SUCCESS) 
	{
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::ObReferenceObjectByHandle failed, error %!STATUS!", status);
        goto out;
    }

    //
    //�õ�TCP �豸����
    //
    g_tcp_device_object = IoGetRelatedDeviceObject(address->file_object);
    if(NULL == g_tcp_device_object) 
	{
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::IoGetRelatedDeviceObject failed");
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto out;

    }

    ObReferenceObject(g_tcp_device_object);
    address->irp = IoAllocateIrp(g_tcp_device_object->StackSize, FALSE);
    if(NULL == address->irp) 
	{
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::IoAllocateIrp failed");
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto out;
    }

out:
    //
    // �ͷ�DrBuildEaBuffer������� EA buffer
    //
    if(address_ea) 
	{
        ExFreePool((PVOID)address_ea);
        address_ea = NULL;
    }
    if(status != STATUS_SUCCESS) 
	{
        DrCloseTransportAddress(address);
    }

	if (pTransport_address)
	{
		ExFreePool((PVOID)pTransport_address);
		pTransport_address = NULL;
	}

    return status;
}

/*++

��������:

    ���Ӵ����ַ��������ü���,���򿪵�Endpoint�ĸ���

����:

    address: �������ַ����

����ֵ:

    ��

--*/
VOID
DrRefTransportAddress(IN PDR_ADDRESS address)
{
	if (NULL == address)
	{
		return;
	}
    InterlockedIncrement(&address->ref_count);
}

/*++
��������:

    ���ٴ����ַ��������ü���,���򿪵�Endpoint�ĸ���

����:

    address: �������ַ����

����ֵ:

    ��
--*/
VOID
DrDerefTransportAddress(IN PDR_ADDRESS address)
{
	if (NULL == address)
	{
		return;
	}
    if(address->ref_count)
	{
        InterlockedDecrement(&address->ref_count);
    }
}

/*++

��������:

    DrMakeSimpleTdiRequest���������

����:

    device_object: �豸����
    irp : �ύ��IO ����
    context: IoSetCompletionRoutine�ṩ��������̵Ĳ���

����ֵ:

    ��һ��������irp->IoStatus.Status���õ�NTSTATUSֵ

--*/
NTSTATUS
DrSimpleTdiRequestComplete(
    IN PDEVICE_OBJECT   device_object,
    IN PIRP             irp,
    IN PVOID            context
    )
{
	UNREFERENCED_PARAMETER(device_object);

    if (irp->PendingReturned) 
	{
        KeSetEvent((PKEVENT )context, IO_NO_INCREMENT, FALSE);		
    }

    return STATUS_MORE_PROCESSING_REQUIRED;
}

/*++

��������:

    ��TCP �ύ����

����:

    irp : Ҫ�ύ��IO ����

����ֵ:

    ��һ��������pIrp->IoStatus.Status���õ�NTSTATUSֵ

--*/
NTSTATUS 
DrMakeSimpleTdiRequest(
    IN PIRP irp
)
{
    NTSTATUS status = STATUS_SUCCESS;
    KEVENT   event;

	if (NULL == irp)
	{
		return STATUS_UNSUCCESSFUL;
	}

    KeInitializeEvent(&event, NotificationEvent, FALSE);

    IoSetCompletionRoutine(
           irp,
           DrSimpleTdiRequestComplete,
           &event,
           TRUE,
           TRUE,
           TRUE
           );
    status = IoCallDriver( g_tcp_device_object, irp );
    if (STATUS_PENDING == status)
    {
        KeWaitForSingleObject(
                   &event,
                   Executive,
                   KernelMode,
                   FALSE,
                   NULL
                   );
        status = irp->IoStatus.Status;
    }
    return status;
}

/*++

��������:

    �������Ӷ˵�����봫���ַ����Ĺ���

����:

    address :     �������ַ����
    end_point :    �������Ӷ˵����

����ֵ:

    STATUS_SUCCESS: �ɹ�
    DrMakeSimpleTdiRequest()ʧ��ֵ

--*/
NTSTATUS
DrAssociateAddress(
	IN PDR_ADDRESS address,
	OUT PDR_ENDPOINT end_point
	)
{
    NTSTATUS status = STATUS_SUCCESS;
	if (NULL == address || NULL == end_point)
	{
		return STATUS_UNSUCCESSFUL;	
	}

#pragma warning(disable : 4127)
    TdiBuildAssociateAddress(
       address->irp,
       g_tcp_device_object,
       end_point->file_object,
       NULL,
       NULL,
       address->h_address
       );

    status = DrMakeSimpleTdiRequest(address->irp);
    if (status != STATUS_SUCCESS)
	{
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::DrMakeSimpleTdiRequest failed, error %!STATUS!", status);
		return status;
    }
	end_point->address = address;
	DrRefTransportAddress(end_point->address);
    return status;
}

/*++

��������:

    �ر����Ӷ˵�������

����:

    end_point :    ����Ҫ�رյ����Ӷ˵�

����ֵ:

    ��
--*/
VOID
DrCloseConnectionContext(IN PDR_ENDPOINT	end_point)
{
	if (NULL == end_point)
	{
		return;
	}
    //
    // ���������Ӷ˵���������,ɾ������
    //
    if (end_point->file_object != NULL)
	{
        ObDereferenceObject(end_point->file_object);
        end_point->file_object = NULL;
    }

    if (end_point->h_endpoint != NULL)
	{
        ZwClose(end_point->h_endpoint);
        end_point->h_endpoint = NULL;
    }
}

/*++

��������:

    �����Ӷ˵�������

����:

    transport_device_name : ������L"\\Device\\Tcp"
    end_point :    ����Ҫ�򿪵����Ӷ˵�
    context : ������,���մ���Endpoint��EaValue��

����ֵ:

    STATUS_SUCCESS: �ɹ�
    DrBuildEaBuffer()ʧ��ֵ
    ZwCreateFile()ʧ��ֵ
    ObReferenceObjectByHandle ()ʧ��ֵ,����:
        STATUS_OBJECT_TYPE_MISMATCH
        STATUS_ACCESS_DENIED
        STATUS_INVALID_HANDLE  

--*/

NTSTATUS
DrOpenConnectionContext(
    IN PWSTR            transport_device_name,
	IN PDR_ENDPOINT		end_point,
    IN PVOID            context
    )
{
    NTSTATUS                    status = STATUS_SUCCESS;
    UNICODE_STRING              device_name;
    PFILE_FULL_EA_INFORMATION   context_ea = NULL;
    ULONG                       buffer_length = 0;
    OBJECT_ATTRIBUTES           object_attr;
    IO_STATUS_BLOCK             io_status;

	if (NULL == transport_device_name || NULL == end_point)
	{
		return STATUS_UNSUCCESSFUL;
	}

    RtlZeroMemory(&io_status, sizeof(IO_STATUS_BLOCK));
    
    RtlInitUnicodeString( &device_name, transport_device_name);
    
    status = DrBuildEaBuffer(
            TDI_CONNECTION_CONTEXT_LENGTH,
            TdiConnectionContext,
            sizeof( PVOID ),
            &context,
            &context_ea,
            &buffer_length
            );
    if(status != STATUS_SUCCESS) 
	{
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Build EaBuffer failed, error %!STATUS!", status);
        goto out;
    }

    InitializeObjectAttributes(
            &object_attr,
            &device_name,
            OBJ_CASE_INSENSITIVE,
            NULL,
            NULL
            );

    //
    // �����Ӷ˵����
    // 
    status = ZwCreateFile(
            &end_point->h_endpoint,
            GENERIC_READ | GENERIC_WRITE | SYNCHRONIZE,
            &object_attr,
            &io_status,
            0,
            FILE_ATTRIBUTE_NORMAL,
            FILE_SHARE_READ,
            FILE_OPEN_IF,
            0,
            context_ea,
            buffer_length
            );

    if(status != STATUS_SUCCESS) 
	{
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::ZwCreateFile failed, error %!STATUS!, IoStatusBlock.Status %!STATUS!", status, io_status.Status);
        goto out;
    }

    status = ObReferenceObjectByHandle(
            end_point->h_endpoint,
            FILE_ANY_ACCESS,
            NULL,
            KernelMode,
            (PVOID *)&end_point->file_object,
            NULL
            );

    if(status != STATUS_SUCCESS) 
	{
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::ObReferenceObjectByHandle failed, error %!STATUS!", status);
        goto out;
    }

out:
    //
    // �ͷ�DrBuildEaBuffer������� EA buffer
    //
    if(context_ea) 
	{
        ExFreePool(context_ea );
        context_ea = NULL;
    }
    if(status != STATUS_SUCCESS) 
	{
        DrCloseConnectionContext(end_point);
    }

    return status;
}

/*++

��������:

   �ر����Ӷ˵����

����:

    end_point :    ����Ҫ�رյ����Ӷ˵�

����ֵ:

    ��

--*/
VOID
DrCloseConnectionEndpoint(IN PDR_ENDPOINT		end_point)
{
	if (NULL == end_point)
	{
		return;
	}
    //
    // �������ü���(������,����ɾ��)
    //
    DrDerefTransportAddress(end_point->address);
    //
    //  ���ﲻ�� disassociate, ֱ�ӹرռ���,��
    //  DDK�� "Closing a Connection Endpoint"һ��
    //
    DrCloseConnectionContext(end_point);
}

/*++

��������:

    �����Ӷ˵����

����:

    transport_device_name : ������L"\\Device\\Tcp"
    address :     ָ��һ��DR_ADDRESS�ṹ��,    ����
                               ���Ӷ˵��������Ĵ����ַ����
    end_point :    ����Ҫ�򿪵����Ӷ˵�
    context : ������,���մ���Endpoint��EaValue��

����ֵ:

    STATUS_SUCCESS: �ɹ�
    DrOpenConnectionContext()ʧ��ֵ
    DrAssociateAddress()ʧ��ֵ

--*/
NTSTATUS
	DrOpenConnectionEndpoint(
	IN PWSTR            transport_device_name,
	IN PDR_ADDRESS		address,
	IN PDR_ENDPOINT		end_point,
	IN PVOID            context
	)
{
    NTSTATUS status = STATUS_SUCCESS;

	if (NULL == transport_device_name 
		|| NULL == address
		|| NULL == end_point)
	{
		return STATUS_UNSUCCESSFUL;
	}

    status = DrOpenConnectionContext(
        transport_device_name,
		end_point,
        context
        );

    if (status != STATUS_SUCCESS)
	{
        goto out;
    }
    //
    // ������ַ����
    //    
    status = DrAssociateAddress(address, end_point);
    if( status != STATUS_SUCCESS )
	{
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::DrAssociateAddress failed, error %!STATUS!", status);
        DrCloseConnectionContext(end_point);
        goto out;
    }
// 	status = DrQueryInformation(end_point);
// 	if (status != STATUS_SUCCESS)
// 	{
// 		DrCloseConnectionContext(end_point);
// 		goto out;
// 	}

out:
    return status;
}

/*++

��������:

    ����EventHandler

����:

    address: �������ַ����
    event_type: EventHandler������
    event_handler : ����ָ��,��EventType�����¼�����ʱ�Ĵ����߼�
    event_context : ����pEventHandler�����Ĳ���

����ֵ:

    STATUS_SUCCESS: �ɹ�
    DrMakeSimpleTdiRequestʧ��ֵ

--*/
NTSTATUS
DrSetEventHandler(
	IN PDR_ADDRESS		address,
    LONG				event_type,
    PVOID				event_handler,
    PVOID				event_context
    )
{
    NTSTATUS status = STATUS_SUCCESS;
	if (NULL == address 
		|| NULL == event_handler
		|| NULL == event_context)
	{
		return STATUS_UNSUCCESSFUL;
	}
    //
    // ��������EventHandler��API
    //
#pragma warning(disable : 4127)
    TdiBuildSetEventHandler(
               address->irp,
               g_tcp_device_object,
               address->file_object,
               NULL,
               NULL,
               event_type,
               event_handler,
               event_context
               );

    //
    // �ύSetEventHandler����
    //
    status = DrMakeSimpleTdiRequest(address->irp);
    if(status != STATUS_SUCCESS)
	{
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::DrMakeSimpleTdiRequest failed, error %!STATUS!", status);
    }

    return status;
}

 /*++

��������:

    ���PTDI_CONNECTION_INFORMATION�ṹ

����:

    transport_address: PTRANSPORT_ADDRESS�ṹ,��ΪPTDI_CONNECTION_INFORMATION�����һ����
    req_con_info :Ҫ���PTDI_CONNECTION_INFORMATION�ṹ

����ֵ:

    ��

--*/
VOID
DrSetupRequestConnectionInfo(
    IN PTRANSPORT_ADDRESS               transport_address,
    IN OUT PTDI_CONNECTION_INFORMATION  req_con_info
    )
{
	if (NULL == transport_address 
		|| NULL == req_con_info)
	{
		return;
	}
    req_con_info->UserDataLength = 0;
    req_con_info->UserData = NULL;
    req_con_info->OptionsLength = 0;
    req_con_info->Options = NULL;
    req_con_info->RemoteAddressLength = sizeof(TA_IP_ADDRESS);
    req_con_info->RemoteAddress = transport_address;

}

/*++

��������:

   ��������

����:

    transport_address: Զ�̵�ַ
    end_point: �������Ӷ˵�
����ֵ:

    STATUS_SUCCESS: �ɹ�
    DrMakeSimpleTdiRequestʧ��ֵ

--*/
NTSTATUS
DrConnect(
    IN PDR_ENDPOINT end_point,
	uint32_t ip, 
	uint32_t port
    )
{
    NTSTATUS status = STATUS_SUCCESS;
	LARGE_INTEGER timeout = RtlConvertLongToLargeInteger(CONNECT_TIMOUT);

	if (NULL == end_point)
	{
		return STATUS_UNSUCCESSFUL;
	}

	TRANSPORT_ADDRESS transport_address;
	transport_address.TAAddressCount = 1;
	transport_address.Address->AddressType = TDI_ADDRESS_TYPE_IP;
	transport_address.Address->AddressLength = sizeof(TDI_ADDRESS_IP);
	TDI_ADDRESS_IP *address_ip = (TDI_ADDRESS_IP *)&transport_address.Address[0].Address;
	address_ip->sin_port = HTONS(port);
	address_ip->in_addr = ip;

    TDI_CONNECTION_INFORMATION RequestConnectionInfo;

    RtlZeroMemory(&RequestConnectionInfo, sizeof(TDI_CONNECTION_INFORMATION));

    DrSetupRequestConnectionInfo(
            &transport_address,
            &RequestConnectionInfo);

    TdiBuildConnect(
        end_point->address->irp,
        g_tcp_device_object,
        end_point->file_object,
        NULL,
        NULL,
        &timeout,  //��ʱֵ,ʹ�ô�����Ĭ��ֵ
        &RequestConnectionInfo,
        NULL
        );
    //
    // �ύConnect����
    //
    status = DrMakeSimpleTdiRequest(end_point->address->irp);

    return status;
}

/*++

��������:

   �Ͽ�����

����:

    end_point: �������Ӷ˵�
    flag : �Ͽ���ʽ

����ֵ:

    STATUS_SUCCESS: �ɹ�
    DrMakeSimpleTdiRequestʧ��ֵ

--*/
NTSTATUS
DrDisconnect(
	IN PDR_ENDPOINT end_point
    )
{
    NTSTATUS status = STATUS_SUCCESS;
	if (NULL == end_point)
	{
		return STATUS_UNSUCCESSFUL;
	}
    TdiBuildDisconnect(
        end_point->address->irp,
        g_tcp_device_object,
		end_point->file_object,
        NULL,
        NULL,
        NULL,  //��ʱֵ,ʹ�ô�����Ĭ��ֵ
        TDI_DISCONNECT_ABORT, //TDI_DISCONNECT_ABORT ,TDI_DISCONNECT_RELEASE
        NULL,
        NULL
        );
    //
    // �ύDisconnect����
    //
    status = DrMakeSimpleTdiRequest(end_point->address->irp);

    return status;
}

/*++

��������:

   DrSendSync, DrReceiveSync��IRP��ȡ������

����:

    device_object: �豸����
    irp : �ύ��IO ����


����ֵ:

    ��һ��������pIrp->IoStatus.Status���õ�NTSTATUSֵ

--*/
VOID
DrCancelIrp(
    IN PDEVICE_OBJECT  device_object,
    IN PIRP            irp

)
{
	UNREFERENCED_PARAMETER(device_object);

    if (NULL == irp)
	{
		return;
	}
    
	_Releases_lock_(irp->CancelIrql)
    IoReleaseCancelSpinLock(irp->CancelIrql);
    irp->IoStatus.Status = STATUS_CANCELLED;
    irp->IoStatus.Information = 0;
    IoCompleteRequest(irp , IO_NO_INCREMENT);
}

/*++

��������:

    DrSendSync, DrReceiveSync���������

����:

    device_object: �豸����
    irp : �ύ��IO ����
    context: IoSetCompletionRoutine�ṩ��������̵Ĳ���

����ֵ:

    ��һ��������pIrp->IoStatus.Status���õ�NTSTATUSֵ

--*/
NTSTATUS
DrEndpointRequestCompleteSync(
	IN PDEVICE_OBJECT       device_object,
	IN PIRP                 irp,
	IN PVOID                context
)
{
	UNREFERENCED_PARAMETER(device_object);

	PENDPOINT_REQUEST_CONTEXT   req_context = (PENDPOINT_REQUEST_CONTEXT)context;


	if (NULL == req_context)
	{
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Invalid context");
		return STATUS_UNSUCCESSFUL;
	}

	if (NULL == irp)
	{
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Invalid irp");
		return STATUS_UNSUCCESSFUL;
	}

	if (!(!strcmp((const char*)req_context->magic, "CSS")
		|| !strcmp((const char*)req_context->magic, "CSA")
		|| !strcmp((const char*)req_context->magic, "CRS")
		|| !strcmp((const char*)req_context->magic, "CRA"))
		)
	{
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Irp req_context error");
		return  STATUS_UNSUCCESSFUL;
	}

	//
	//��Irp��IoStatus����û��ṩ��IoStatus
	//
	if (NULL != req_context->io_status)
	{
		(req_context->io_status)->Status = irp->IoStatus.Status;
		(req_context->io_status)->Information = irp->IoStatus.Information;
	}

	//
	//Ϊͬ�����ͺͽ��������¼�
	//
	if (irp->PendingReturned)
	{
		if (req_context->event != NULL)
		{
			KeSetEvent(req_context->event, IO_NO_INCREMENT, FALSE);
		}
	}

	return STATUS_MORE_PROCESSING_REQUIRED;
}



NTSTATUS
DrEndpointRequestCompleteAsync(
    IN PDEVICE_OBJECT       device_object,
    IN PIRP                 irp,
    IN PVOID                context
    )
{
	UNREFERENCED_PARAMETER(device_object);

	NTSTATUS status = STATUS_SUCCESS;

    PENDPOINT_REQUEST_CONTEXT   req_context = (PENDPOINT_REQUEST_CONTEXT)context ;

	
	if (NULL == req_context)
	{
		status = STATUS_UNSUCCESSFUL;
		goto out;
	}

	if (NULL != req_context->mdl)
	{
		DrUnlockAndFreeMdl(req_context->mdl);
		//IoFreeMdl(mdl);
		req_context->mdl = NULL;
	}

	if (NULL == irp)
	{
		status = STATUS_UNSUCCESSFUL;
		goto out;
	}

    if( !(!strcmp((const char*)req_context->magic, "CSS")
        || !strcmp((const char*)req_context->magic, "CSA")
        || !strcmp((const char*)req_context->magic, "CRS")
        || !strcmp((const char*)req_context->magic, "CRA"))
	  )
	{
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::Irp req_context error");
		status = STATUS_UNSUCCESSFUL;
		goto out;
	}

    //
    //��Irp��IoStatus����û��ṩ��IoStatus
    //
    if (NULL != req_context->io_status)
	{
        (req_context->io_status)->Status = irp->IoStatus.Status;
        (req_context->io_status)->Information = irp->IoStatus.Information;
    }

    //
    //Ϊͬ�����ͺͽ��������¼�
    //
    if (irp->PendingReturned) 
	{
        if(req_context->event != NULL)
		{
            KeSetEvent(req_context->event, IO_NO_INCREMENT, FALSE);
        }
    }

	status = STATUS_MORE_PROCESSING_REQUIRED;

out:
	if (NULL != req_context)
	{
		req_context->io_status = NULL;
		req_context->event = NULL;

		ExFreePool(req_context);
		req_context = NULL;
	}

    //
    //�ͷ�Irp
    //
	if (NULL != irp)
	{
		IoFreeIrp(irp);
		irp = NULL;
	}

    return status;
}


/*++
��������:

    ִ��ͬ������

����:

    end_point : �������Ӷ˵�
    mdl : Ҫ���͵�Mdl
    flag : ���͵�ѡ��,��TDI_SEND_EXPEDITED, TDI_SEND_PARTIAL��,��DDK

����ֵ:

    ��һ��������LocalIoStatus.Status���õ�NTSTATUSֵ

--*/
BOOLEAN BuildSendIrp(PDR_ENDPOINT end_point, PMDL mdl, ULONG flags, PIRP* ret_irp, PENDPOINT_REQUEST_CONTEXT* ret_context, PIO_STATUS_BLOCK*	ret_io_status, PKEVENT* ret_event)
{
	BOOLEAN ret = FALSE;

	BOOLEAN flag = FALSE;

	NTSTATUS   status = STATUS_SUCCESS;
	PIRP   irp = NULL;
	PENDPOINT_REQUEST_CONTEXT   req_context = NULL;
	PKEVENT event = NULL;
	PIO_STATUS_BLOCK	 io_status = NULL;
	PMDL  next_mdl = NULL;
	ULONG length = 0;
	do
	{
		irp = IoAllocateIrp(g_tcp_device_object->StackSize, FALSE);
		if (NULL == irp)
		{
			status = STATUS_INSUFFICIENT_RESOURCES;
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::IoAllocateIrp failed, error %!STATUS!", status);
			break;
		}

		req_context = (PENDPOINT_REQUEST_CONTEXT)DrMalloc(NonPagedPool, sizeof(ENDPOINT_REQUEST_CONTEXT));
		if (NULL == req_context)
		{
			status = STATUS_INSUFFICIENT_RESOURCES;
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::DrMalloc for context failed, error %!STATUS!", status);
			break;
		}
		RtlZeroMemory(req_context, ENDPOINT_REQUEST_CONTEXT_);

		event = (PKEVENT)DrMalloc(NonPagedPool, sizeof(KEVENT));
		if (NULL == event)
		{
			status = STATUS_INSUFFICIENT_RESOURCES;
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::DrMalloc for pLocalEvent failed, error %!STATUS!", status);
			break;
		}
		KeInitializeEvent(event, SynchronizationEvent, FALSE);

		io_status = (PIO_STATUS_BLOCK)DrMalloc(NonPagedPool, sizeof(IO_STATUS_BLOCK));
		if (NULL == io_status)
		{
			status = STATUS_INSUFFICIENT_RESOURCES;
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::DrMalloc for pIoStatus failed, error %!STATUS!", status);
			break;
		}
		RtlFillMemory(io_status, sizeof(IO_STATUS_BLOCK), 0xC3);

		strcpy_s((char *)req_context->magic, sizeof(req_context->magic), "CSS");
		req_context->event = event;
		req_context->io_status = io_status;
		req_context->mdl = mdl;
		req_context->length = -1;
		req_context->address = NULL;
		//
		// �õ�Mdl�ĳ��ȣ���Ϊ���ͳ���
		//
		next_mdl = mdl;
		while (next_mdl)
		{
			length += MmGetMdlByteCount(next_mdl);
			next_mdl = next_mdl->Next;
		}

		TdiBuildSend(
			irp,
			g_tcp_device_object,
			end_point->file_object,
			DrEndpointRequestCompleteSync,
			req_context,
			mdl,
			flags,
			length
		);

		ret = TRUE;
	} while (flag);

	if (!NT_SUCCESS(status))
	{
		if (irp)
		{
			IoFreeIrp(irp);
			irp = NULL;
		}

		if (req_context)
		{
			ExFreePool(req_context);
			req_context = NULL;
		}

		if (io_status)
		{
			ExFreePool(io_status);
			io_status = NULL;
		}

		if (event)
		{
			ExFreePool(event);
			event = NULL;
		}
	}

	*ret_irp = irp;
	*ret_context = req_context;
	*ret_io_status = io_status;
	*ret_event = event;

	return ret;
}

BOOLEAN CheckCallStatus(PKEVENT event, PIO_STATUS_BLOCK io_status, NTSTATUS call_status, PLARGE_INTEGER wait_time, PNTSTATUS result_status, PULONG result_information)
{
	BOOLEAN irp_complete = FALSE;
	NTSTATUS status = call_status;
	ULONG  information = 0;

	if (STATUS_PENDING == status)
	{
		status = KeWaitForSingleObject(
			event,
			Executive,
			KernelMode,
			FALSE,
			wait_time
		);

		if (STATUS_SUCCESS == status)
		{
			if (io_status->Status != STATUS_SUCCESS)
			{
				TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::pIoStatus.Status indicates failure: 0x%08x.(%u)", io_status->Status, (ULONG)io_status->Information);
			}
			status = io_status->Status;
			information = (ULONG)io_status->Information;
			irp_complete = TRUE;
		}
		else if (STATUS_TIMEOUT == status)
		{
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::STATUS_TIMEOUT, error %!STATUS!", status);
			information = 0;
			irp_complete = FALSE;
		}
		else
		{
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::KeWaitForSingleObject failed, error %!STATUS!", status);
			information = 0;
			irp_complete = FALSE;
		}
	}
	else if (STATUS_SUCCESS == status)
	{
		status = io_status->Status;
		information = (ULONG)io_status->Information;
		irp_complete = TRUE;
	}
	else
	{
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::IoCallDriver failed, error %!STATUS!", status);
		information = 0;
		irp_complete = TRUE;
	}

	if (result_status)
	{
		*result_status = status;
	}

	if (result_information)
	{
		*result_information = information;
	}

	return irp_complete;
}

NTSTATUS
DrSendSync(
	IN PDR_ENDPOINT     end_point,
    IN PMDL             mdl,
    IN ULONG            flags
    )
{
    NTSTATUS                            status = STATUS_SUCCESS;
    PIRP                                irp = NULL;
    BOOLEAN                             irp_complete = FALSE;
    BOOLEAN                             cancel_irp = FALSE;
	PENDPOINT_REQUEST_CONTEXT   req_context = NULL;
	PIO_STATUS_BLOCK	 io_status = NULL;
	PKEVENT event = NULL;

	if (NULL == end_point || NULL == mdl)
	{
		status = STATUS_UNSUCCESSFUL;
		goto out;
	}

	if (!BuildSendIrp(end_point, mdl, flags, &irp, &req_context, &io_status, &event))
	{
		status = STATUS_UNSUCCESSFUL;
		goto out;
	}

    //IoSetCancelRoutine(irp, DrCancelIrp);

    status = IoCallDriver( g_tcp_device_object, irp );
	irp_complete = CheckCallStatus(event, io_status, status, NULL, &status, NULL);

    if(!irp_complete && !KeReadStateEvent(event))
	{
        cancel_irp = IoCancelIrp(irp);
        KeWaitForSingleObject(
                event,
                Executive,
                KernelMode,
                FALSE,
                NULL
                );
    }

out:
	if (req_context)
	{
		ExFreePool(req_context);
		req_context = NULL;
	}

	if(event)
	{
		ExFreePool(event);
		event = NULL;
	}

	if(io_status)
	{
		ExFreePool(io_status);
		io_status = NULL;
	}

	if (irp)
	{
		IoFreeIrp(irp);
		irp = NULL;
	}

    return status ;
}

/*++

��������:

    ִ��ͬ������
����:

    endpoint : �������Ӷ˵�
    mdl : ���ڽ��յ�Mdl
    flag : ���յ�ѡ��,��TDI_RECEIVE_EXPEDITED, TDI_RECEIVE_PEEK ��,��DDK
    info_size: ʵ�ʽ��յ��Ĵ�С

����ֵ:

    ��һ��������LocalIoStatus.Status���õ�NTSTATUSֵ

--*/
BOOLEAN BuildRecvIrp(PDR_ENDPOINT endpoint, PMDL mdl, ULONG flags, PIRP* ret_irp, PENDPOINT_REQUEST_CONTEXT* ret_context, PIO_STATUS_BLOCK* ret_io_status, PKEVENT* ret_event)
{
	BOOLEAN ret = FALSE;

	BOOLEAN flag = FALSE;

	NTSTATUS   status = STATUS_SUCCESS;
	PIRP   irp = NULL;
	PENDPOINT_REQUEST_CONTEXT   req_context = NULL;
	PKEVENT event = NULL;
	PIO_STATUS_BLOCK	 io_status = NULL;
	PMDL  next_mdl = NULL;
	ULONG length = 0;
	do
	{
		irp = IoAllocateIrp(g_tcp_device_object->StackSize, FALSE);
		if (NULL == irp)
		{
			status = STATUS_INSUFFICIENT_RESOURCES;
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::IoAllocateIrp failed, error %!STATUS!", status);
			break;
		}

		req_context = (PENDPOINT_REQUEST_CONTEXT)DrMalloc(NonPagedPool, sizeof(ENDPOINT_REQUEST_CONTEXT));
		if (NULL == req_context)
		{
			status = STATUS_INSUFFICIENT_RESOURCES;
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::DrMalloc for pRequestContext failed, error %!STATUS!", status);
			break;
		}
		RtlZeroMemory(req_context, ENDPOINT_REQUEST_CONTEXT_);

		event = (PKEVENT)DrMalloc(NonPagedPool, sizeof(KEVENT));
		if (NULL == event)
		{
			status = STATUS_INSUFFICIENT_RESOURCES;
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::DrMalloc for pLocalEvent failed, error %!STATUS!", status);
			break;
		}
		KeInitializeEvent(event, SynchronizationEvent, FALSE);

		io_status = (PIO_STATUS_BLOCK)DrMalloc(NonPagedPool, sizeof(IO_STATUS_BLOCK));
		if (NULL == io_status)
		{
			status = STATUS_INSUFFICIENT_RESOURCES;
			TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::DrMalloc for pIoStatus failed, error %!STATUS!", status);
			break;
		}
		RtlFillMemory(io_status, sizeof(IO_STATUS_BLOCK), 0xC3);

		strcpy_s((char *)req_context->magic, sizeof(req_context->magic), "CRS");
		req_context->event = event;
		req_context->io_status = io_status;
		req_context->mdl = mdl;
		req_context->length = -1;
		req_context->address = NULL;
		//
		// �õ�Mdl�ĳ��ȣ���Ϊ���յĳ���
		//    
		next_mdl = mdl;
		while (next_mdl)
		{
			length += MmGetMdlByteCount(next_mdl);
			next_mdl = next_mdl->Next;
		}

		TdiBuildReceive(
			irp,
			g_tcp_device_object,
			endpoint->file_object,
			DrEndpointRequestCompleteSync,
			req_context,
			mdl,
			flags,
			length
		);

		ret = TRUE;
	} while (flag);

	if (!NT_SUCCESS(status))
	{
		if (irp)
		{
			IoFreeIrp(irp);
			irp = NULL;
		}

		if (req_context)
		{
			ExFreePool(req_context);
			req_context = NULL;
		}

		if (io_status)
		{
			ExFreePool(io_status);
			io_status = NULL;
		}

		if (event)
		{
			ExFreePool(event);
			event = NULL;
		}
	}

	*ret_irp = irp;
	*ret_context = req_context;
	*ret_io_status = io_status;
	*ret_event = event;

	return ret;
}

NTSTATUS
DrReceiveSync(
    IN PDR_ENDPOINT  endpoint,
    IN PMDL          mdl,
    IN ULONG         flags,
    OUT PULONG       info_size
    )
{
    NTSTATUS                    status = STATUS_SUCCESS;
    PIRP                        irp = NULL;
    ULONG                       information = 0;
    PENDPOINT_REQUEST_CONTEXT   req_context = NULL;
	PKEVENT                     event = NULL;
	PIO_STATUS_BLOCK            io_status = NULL;
    BOOLEAN                     irp_complete = FALSE;
    BOOLEAN                     cancel_irp = FALSE;
	LARGE_INTEGER				wait_time;

	if (NULL == mdl || NULL == endpoint)
	{
		status = STATUS_UNSUCCESSFUL;
		goto out;
	}
 
	if (!BuildRecvIrp(endpoint, mdl, flags, &irp, &req_context, &io_status, &event))
	{
		status = STATUS_UNSUCCESSFUL;
		goto out;
	}

    //IoSetCancelRoutine(irp, DrCancelIrp);

	wait_time = RtlConvertLongToLargeInteger(SOCKET_WAIT_TIME);
    status = IoCallDriver(g_tcp_device_object, irp);
	irp_complete = CheckCallStatus(event, io_status, status, &wait_time, &status, &information);

    *info_size = information;

    if (!irp_complete && !KeReadStateEvent(event))
	{
        cancel_irp = IoCancelIrp(irp);
        KeWaitForSingleObject(
                event,
                Executive,
                KernelMode,
                FALSE,
                NULL
                );
    }

out:
	if (req_context)
	{
		ExFreePool(req_context);
		req_context = NULL;
	}

	if (event)
	{
		ExFreePool(event);
		event = NULL;
	}

	if (io_status)
	{
		ExFreePool(io_status);
		io_status = NULL;
	}

	if (irp)
	{
		IoFreeIrp(irp);
		irp = NULL;
	}

    return status;
}

 /*++

��������:

    ִ���첽����

����:

    endpoint : �������Ӷ˵�
    mdl : ���ڽ��յ�Mdl
    flags : ���յ�ѡ��,��TDI_SEND_EXPEDITED, TDI_SEND_PARTIAL��,��DDK
    event: �ϲ��ṩ��Event
    io_status: �ϲ��ṩ��IoStatus

����ֵ:

    STATUS_SUCCESS
    IoCallDriverʧ��ֵ

--*/
NTSTATUS
DrReceiveAsync(
	IN PDR_ENDPOINT  endpoint,
	IN PMDL          mdl,
	IN ULONG         flags,
	IN PKEVENT       event,
	IN PIO_STATUS_BLOCK io_status
	)
{
	PIRP                        irp = NULL;
    PMDL                        next_mdl = NULL;
    ULONG                       length = 0;
    NTSTATUS                    status = STATUS_SUCCESS;
    PENDPOINT_REQUEST_CONTEXT   req_context = NULL;
    //
    //����Irp
    //  
    irp = IoAllocateIrp(g_tcp_device_object->StackSize, FALSE);
    if (NULL == irp)
	{
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto out;
    }

    req_context = (PENDPOINT_REQUEST_CONTEXT)DrMalloc(NonPagedPool, ENDPOINT_REQUEST_CONTEXT_);
    if (NULL == req_context)
	{
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto out;
    }
    //
    // �õ�Mdl�ĳ��ȣ���Ϊ���յĳ���
    //
    next_mdl = mdl;
    while (next_mdl)
	{
        length += MmGetMdlByteCount(next_mdl);
        next_mdl = next_mdl->Next;
    }

	strcpy_s((char *)req_context->magic, sizeof(req_context->magic), "CRS");
	req_context->event = event;
	req_context->io_status = io_status;
	req_context->mdl = NULL;
	req_context->length = length;
	req_context->address = NULL;

    TdiBuildReceive (
             irp,
             g_tcp_device_object,
             endpoint->file_object,
             DrEndpointRequestCompleteAsync,
             req_context,
             mdl,
             flags,
             length
             );

    status = IoCallDriver(g_tcp_device_object, irp);
    if (STATUS_PENDING == status || STATUS_SUCCESS == status) 
	{
        status = STATUS_SUCCESS;
    }
	else
	{
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::IoCallDriver failed, error %!STATUS!", status);
    }

	return status;

out:
	if (mdl)
	{
		DrUnlockAndFreeMdl(mdl);
		mdl = NULL;
	}

	if (irp)
	{
		IoFreeIrp(irp);
		irp = NULL;
	}

	if (req_context)
	{
		ExFreePool(req_context);
		req_context = NULL;
	}

    return status ;
}