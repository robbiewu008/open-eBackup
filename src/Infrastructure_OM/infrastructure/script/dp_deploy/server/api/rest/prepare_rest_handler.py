import os
from fastapi import APIRouter, HTTPException, status, Header
from server.common.logger.logger import get_logger
from server.common.consts import LOG_PATH
from server.schemas.request import AuthBase, CreateNamespaceRequest, CreateNFSShareRequest, CreatNFSClientRequest, \
    CreateNFSClientResponse, CreateNFSShareResponse, CreateNamespaceResponse, AuthResponse, GetNamespaceResponse, \
    DeleteNFSClientRequest, GetNFSShareRequest, GetNFSShareResponse, GetNFSClientRequest, GetNamespaceRequest, \
    GetNFSClientResponse, DeleteNFSShareRequest, GetHostnameResponse, DeleteNamespaceRequest, GetInterfaceResponse, \
    GetInterfaceRequest
from server.services.underlay.nfs_share import get_token, create_pms_nfs, create_pms_namespace, create_pms_client, \
    get_pms_namespace, delete_pms_namespace, get_pms_nfs, delete_pms_nfs, get_pms_client, delete_pms_client, \
    get_secondary_float_ip, get_hostname, umount, get_interface_from_ip
from server.common.consts import COMMAND_FAILED, COMMAND_WRONG

external_router = APIRouter(
    prefix="/converaged_service",
    tags=["prepare"])
databackup_router = APIRouter(
    prefix="/converaged_service",
    tags=["prepare"]
)


@external_router.post("/test", summary="测试ip和端口")
def request_get_token(req: AuthBase):
    return {"password": req.password}


@external_router.post("/auth", summary="获取token")
def request_get_token(req: AuthBase):
    token, info = get_token(req)
    if not token:
        raise HTTPException(status_code=status.HTTP_401_UNAUTHORIZED, detail=f"unauthorized, info: {info}")
    return AuthResponse(token=token)


@external_router.post("/namespace", summary="创建命名空间")
def request_create_namespace(req: CreateNamespaceRequest, x_auth_token: str = Header(default=None)):
    namespace_id, info = create_pms_namespace(x_auth_token, req)
    if not namespace_id:
        raise HTTPException(status_code=status.HTTP_400_BAD_REQUEST, detail=f"Failed to create namespace,info: {info}")
    return CreateNamespaceResponse(namespace_id=str(namespace_id))


@external_router.get("/namespace", summary="查询命名空间")
def request_get_namespace(req: GetNamespaceRequest, x_auth_token: str = Header(default=None)):
    issue, namespace_id, info = get_pms_namespace(x_auth_token, req)
    if issue == COMMAND_WRONG:
        raise HTTPException(status_code=status.HTTP_400_BAD_REQUEST,
                            detail=f"Request wrong, failed to get namespace, info: {info}")

    return GetNamespaceResponse(namespace_id=namespace_id)


@external_router.delete("/namespace", summary="删除命名空间")
def request_delete_namespace(req: DeleteNamespaceRequest, x_auth_token: str = Header(default=None)):
    has_issue, info = delete_pms_namespace(x_auth_token, req)
    if has_issue:
        raise HTTPException(status_code=status.HTTP_400_BAD_REQUEST, detail=f"Failed to delete namespace, info: {info}")


@external_router.post("/nfs_share", summary="创建NFS共享")
def request_create_nfs_share(pre: CreateNFSShareRequest, x_auth_token: str = Header(default=None)):
    nfs_id, info = create_pms_nfs(x_auth_token, pre)
    if not nfs_id:
        raise HTTPException(status_code=status.HTTP_400_BAD_REQUEST, detail=f"Failed to create nfs share, info: {info}")
    return CreateNFSShareResponse(nfs_id=str(nfs_id))


@external_router.get("/nfs_share", summary="查询NFS共享")
def request_get_nfs_share(pre: GetNFSShareRequest, x_auth_token: str = Header(default=None)):
    issue, nfs_id, info = get_pms_nfs(x_auth_token, pre)
    if issue == COMMAND_WRONG:
        raise HTTPException(status_code=status.HTTP_400_BAD_REQUEST, detail=f"Failed to get nfs share, info: {info}")

    return GetNFSShareResponse(nfs_id=nfs_id)


@external_router.delete("/nfs_share", summary="删除NFS共享")
def request_delete_nfs_share(req: DeleteNFSShareRequest, x_auth_token: str = Header(default=None)):
    has_issue, info = delete_pms_nfs(x_auth_token, req)
    if has_issue:
        raise HTTPException(status_code=status.HTTP_400_BAD_REQUEST, detail=f"Failed to delete nfs share, info: {info}")


@external_router.post("/client", summary="创建客户端")
def request_create_client(pre: CreatNFSClientRequest, x_auth_token: str = Header(default=None)):
    client_id, info = create_pms_client(x_auth_token, pre)
    if not client_id:
        raise HTTPException(status_code=status.HTTP_400_BAD_REQUEST,
                            detail=f"Failed to create nfs client, info: {info}")
    return CreateNFSClientResponse(nfs_client_id=str(client_id))


@external_router.get("/client", summary="查询客户端")
def request_get_client(pre: GetNFSClientRequest, x_auth_token: str = Header(default=None)):
    issue, nfs_client_id, info = get_pms_client(x_auth_token, pre)
    if issue == COMMAND_WRONG:
        raise HTTPException(status_code=status.HTTP_400_BAD_REQUEST, detail=f"Failed to get nfs client, info: {info}")
    return GetNFSClientResponse(nfs_client_id=nfs_client_id)


@external_router.delete("/client", summary="删除客户端")
def request_delete_client(pre: DeleteNFSClientRequest, x_auth_token: str = Header(default=None)):
    has_issue, info = delete_pms_client(x_auth_token, pre)
    if has_issue:
        raise HTTPException(status_code=status.HTTP_400_BAD_REQUEST,
                            detail=f"Failed to delete nfs client, info: {info}")


@external_router.get("/secondary_float_ip", summary="查询副浮动ip")
def request_get_secondary_float_ip(x_auth_token: str = Header(default=None)):
    nfs_client_id, info = get_secondary_float_ip(x_auth_token)
    if not nfs_client_id:
        raise HTTPException(status_code=status.HTTP_400_BAD_REQUEST,
                            detail=f"Failed to get secondary_float_ip, info: {info}")
    return GetNFSClientResponse(nfs_client_id=str(nfs_client_id))


@external_router.get("/hostname", summary="查询主机名称")
@databackup_router.get("/hostname", summary="查询主机名称")
def request_get_hostname():
    hostname = get_hostname()
    return GetHostnameResponse(hostname=str(hostname))


@databackup_router.post("/interface", summary="查询ip对应的网卡")
def request_get_interface_from_ip(req: GetInterfaceRequest):
    has_issue, info = get_interface_from_ip(req.ip)
    if has_issue:
        raise HTTPException(status_code=status.HTTP_400_BAD_REQUEST,
                            detail=f"Failed to get interface from ip: {req.ip},  info: {info}")
    return GetInterfaceResponse(interface=str(info))


@external_router.post("/umount", summary="卸除挂载点")
def request_umount():
    has_issue, info = umount()
    if has_issue:
        raise HTTPException(status_code=status.HTTP_400_BAD_REQUEST,
                            detail=f"Failed to umount, info: {info}")
