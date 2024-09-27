#include "socket.h"

#include "wpp_trace.h"
#include "socket.tmh"



//全局的TCP设备对象
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

函数描述:

    解锁定并释放一块MDL

参数:

    mdl : 要解锁并释放的MDL地址

返回值:

    无

--*/
VOID
DrUnlockAndFreeMdl(
    IN PMDL mdl
    )
{
    MmUnlockPages(mdl);//分页内存需要解锁
    IoFreeMdl(mdl);
}

/*++
函数描述:

    建立PFILE_FULL_EA_INFORMATION结构

参数:

    name_length:扩展属性(EA)名字的长度
    name: EA的名字
    value_length: EA值的长度
    value: EA值
    p_buffer: 构造的EA Buffer指针
    buffer_length: EA Buffer的大小

返回值:

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
    //  得到EA Buffer的长度,1为EaName后的字符串结束标志
    //   
    *buffer_length = FIELD_OFFSET(FILE_FULL_EA_INFORMATION, EaName[0]) +
                       name_length + 1 +
                       value_length;
    //
    //  分配EA Buffer,该内存在DrBuildEaBuffer的调用者中释放
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

函数描述:

    得到PTRANSPORT_ADDRESS结构的实际大小

参数:

    transport_address: 指向PTRANSPORT_ADDRESS结构变量的指针

返回值:

    transport_address的实际大小

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

函数描述:

    关闭传输地址对象

参数:

    address:     指向一个DR_ADDRESS结构体, 代表将要关闭传输地址对象

返回值:

    无

--*/
VOID
DrCloseTransportAddress(IN PDR_ADDRESS address)
{
	if (NULL == address)
	{
		return;
	}
    //
    // 释放通用IRP
    //
    if (address->irp != NULL) 
	{
        IoFreeIrp(address->irp);
        address->irp = NULL;
    }
    //
    // 撤销对地址对象的引用,删除对象
    //
    if (address->file_object != NULL)
	{
        ObDereferenceObject(address->file_object);
        address->file_object = NULL;
    }
    //
    // 关闭地址对象句柄
    //
    if(address->h_address != NULL)
	{
        ZwClose(address->h_address);
        address->h_address = NULL;
    }
}

/*++
函数描述:

    为本地TDI连接打开一个传输地址对象

参数:

    device_name : 这里是L"\\Device\\Tcp"
    transport_address:  指向一个TRANSPORT_ADDRESS 结构体,
                                        代表要打开的本地地址
    address:     指向一个DR_ADDRESS结构体,
                               将会初始化为已打开的传输地址对象

返回值:

    STATUS_SUCCESS  成功
    DrBuildEaBuffer()失败值
    STATUS_INCEFENICENT_RESOURCES
    ZwCreateFile() 失败值
    ObReferenceObjectByHandle() 失败值,包括:
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
    // 为指定的传输地址建立一个EA Buffer
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
    // 打开传输地址对象
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
    //得到TCP 设备对象
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
    // 释放DrBuildEaBuffer中申请的 EA buffer
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

函数描述:

    增加传输地址对象的引用计数,即打开的Endpoint的个数

参数:

    address: 代表传输地址对象

返回值:

    无

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
函数描述:

    减少传输地址对象的引用计数,即打开的Endpoint的个数

参数:

    address: 代表传输地址对象

返回值:

    无
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

函数描述:

    DrMakeSimpleTdiRequest的完成例程

参数:

    device_object: 设备对象
    irp : 提交的IO 请求
    context: IoSetCompletionRoutine提供的完成例程的参数

返回值:

    下一层驱动在irp->IoStatus.Status设置的NTSTATUS值

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

函数描述:

    向TCP 提交请求

参数:

    irp : 要提交的IO 请求

返回值:

    下一层驱动在pIrp->IoStatus.Status设置的NTSTATUS值

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

函数描述:

    建立连接端点对象与传输地址对象的关联

参数:

    address :     代表传输地址对象
    end_point :    代表连接端点对象

返回值:

    STATUS_SUCCESS: 成功
    DrMakeSimpleTdiRequest()失败值

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

函数描述:

    关闭连接端点上下文

参数:

    end_point :    代表要关闭的连接端点

返回值:

    无
--*/
VOID
DrCloseConnectionContext(IN PDR_ENDPOINT	end_point)
{
	if (NULL == end_point)
	{
		return;
	}
    //
    // 撤销对连接端点对象的引用,删除对象
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

函数描述:

    打开连接端点上下文

参数:

    transport_device_name : 这里是L"\\Device\\Tcp"
    end_point :    代表要打开的连接端点
    context : 上下文,最终传入Endpoint的EaValue中

返回值:

    STATUS_SUCCESS: 成功
    DrBuildEaBuffer()失败值
    ZwCreateFile()失败值
    ObReferenceObjectByHandle ()失败值,包括:
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
    // 打开连接端点对象
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
    // 释放DrBuildEaBuffer中申请的 EA buffer
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

函数描述:

   关闭连接端点对象

参数:

    end_point :    代表要关闭的连接端点

返回值:

    无

--*/
VOID
DrCloseConnectionEndpoint(IN PDR_ENDPOINT		end_point)
{
	if (NULL == end_point)
	{
		return;
	}
    //
    // 减少引用计数(仅减少,并不删除)
    //
    DrDerefTransportAddress(end_point->address);
    //
    //  这里不用 disassociate, 直接关闭即可,见
    //  DDK中 "Closing a Connection Endpoint"一节
    //
    DrCloseConnectionContext(end_point);
}

/*++

函数描述:

    打开连接端点对象

参数:

    transport_device_name : 这里是L"\\Device\\Tcp"
    address :     指向一个DR_ADDRESS结构体,    代表
                               连接端点所依赖的传输地址对象
    end_point :    代表要打开的连接端点
    context : 上下文,最终传入Endpoint的EaValue中

返回值:

    STATUS_SUCCESS: 成功
    DrOpenConnectionContext()失败值
    DrAssociateAddress()失败值

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
    // 关联地址对象
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

函数描述:

    设置EventHandler

参数:

    address: 代表传输地址对象
    event_type: EventHandler的类型
    event_handler : 函数指针,含EventType类型事件到来时的处理逻辑
    event_context : 传给pEventHandler函数的参数

返回值:

    STATUS_SUCCESS: 成功
    DrMakeSimpleTdiRequest失败值

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
    // 调用设置EventHandler的API
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
    // 提交SetEventHandler请求
    //
    status = DrMakeSimpleTdiRequest(address->irp);
    if(status != STATUS_SUCCESS)
	{
		TracePrint(TRACE_LEVEL_ERROR, "%!FUNC!::DrMakeSimpleTdiRequest failed, error %!STATUS!", status);
    }

    return status;
}

 /*++

函数描述:

    填充PTDI_CONNECTION_INFORMATION结构

参数:

    transport_address: PTRANSPORT_ADDRESS结构,作为PTDI_CONNECTION_INFORMATION的最后一个域
    req_con_info :要填充PTDI_CONNECTION_INFORMATION结构

返回值:

    无

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

函数描述:

   发起连接

参数:

    transport_address: 远程地址
    end_point: 本地连接端点
返回值:

    STATUS_SUCCESS: 成功
    DrMakeSimpleTdiRequest失败值

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
        &timeout,  //超时值,使用传输层的默认值
        &RequestConnectionInfo,
        NULL
        );
    //
    // 提交Connect请求
    //
    status = DrMakeSimpleTdiRequest(end_point->address->irp);

    return status;
}

/*++

函数描述:

   断开连接

参数:

    end_point: 本地连接端点
    flag : 断开方式

返回值:

    STATUS_SUCCESS: 成功
    DrMakeSimpleTdiRequest失败值

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
        NULL,  //超时值,使用传输层的默认值
        TDI_DISCONNECT_ABORT, //TDI_DISCONNECT_ABORT ,TDI_DISCONNECT_RELEASE
        NULL,
        NULL
        );
    //
    // 提交Disconnect请求
    //
    status = DrMakeSimpleTdiRequest(end_point->address->irp);

    return status;
}

/*++

函数描述:

   DrSendSync, DrReceiveSync的IRP的取消例程

参数:

    device_object: 设备对象
    irp : 提交的IO 请求


返回值:

    下一层驱动在pIrp->IoStatus.Status设置的NTSTATUS值

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

函数描述:

    DrSendSync, DrReceiveSync的完成例程

参数:

    device_object: 设备对象
    irp : 提交的IO 请求
    context: IoSetCompletionRoutine提供的完成例程的参数

返回值:

    下一层驱动在pIrp->IoStatus.Status设置的NTSTATUS值

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
	//用Irp的IoStatus填充用户提供的IoStatus
	//
	if (NULL != req_context->io_status)
	{
		(req_context->io_status)->Status = irp->IoStatus.Status;
		(req_context->io_status)->Information = irp->IoStatus.Information;
	}

	//
	//为同步发送和接收设置事件
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
    //用Irp的IoStatus填充用户提供的IoStatus
    //
    if (NULL != req_context->io_status)
	{
        (req_context->io_status)->Status = irp->IoStatus.Status;
        (req_context->io_status)->Information = irp->IoStatus.Information;
    }

    //
    //为同步发送和接收设置事件
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
    //释放Irp
    //
	if (NULL != irp)
	{
		IoFreeIrp(irp);
		irp = NULL;
	}

    return status;
}


/*++
函数描述:

    执行同步发送

参数:

    end_point : 代表连接端点
    mdl : 要发送的Mdl
    flag : 发送的选项,如TDI_SEND_EXPEDITED, TDI_SEND_PARTIAL等,见DDK

返回值:

    下一层驱动在LocalIoStatus.Status设置的NTSTATUS值

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
		// 得到Mdl的长度，作为发送长度
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

函数描述:

    执行同步接收
参数:

    endpoint : 代表连接端点
    mdl : 用于接收的Mdl
    flag : 接收的选项,如TDI_RECEIVE_EXPEDITED, TDI_RECEIVE_PEEK 等,见DDK
    info_size: 实际接收到的大小

返回值:

    下一层驱动在LocalIoStatus.Status设置的NTSTATUS值

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
		// 得到Mdl的长度，作为接收的长度
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

函数描述:

    执行异步接收

参数:

    endpoint : 代表连接端点
    mdl : 用于接收的Mdl
    flags : 接收的选项,如TDI_SEND_EXPEDITED, TDI_SEND_PARTIAL等,见DDK
    event: 上层提供的Event
    io_status: 上层提供的IoStatus

返回值:

    STATUS_SUCCESS
    IoCallDriver失败值

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
    //分配Irp
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
    // 得到Mdl的长度，作为接收的长度
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