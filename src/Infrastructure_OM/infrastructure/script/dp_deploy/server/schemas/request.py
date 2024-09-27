from pydantic import BaseModel, Field
from typing import Union, List


class AuthBase(BaseModel):
    user_name: str
    password: str


class AuthResponse(BaseModel):
    token: str


class CreateNamespaceRequest(BaseModel):
    account_id: str
    namespace_name: str
    pool_id: int


class CreateNamespaceResponse(BaseModel):
    namespace_id: str


class GetNamespaceRequest(BaseModel):
    account_id: str
    namespace_name: str


class GetNamespaceResponse(BaseModel):
    namespace_id: Union[str, None] = None


class DeleteNamespaceRequest(BaseModel):
    account_id: str
    namespace_name: str


class DeleteNamespaceResponse(BaseModel):
    namespace_id: Union[str, None] = None


class CreateNFSShareRequest(BaseModel):
    account_id: str
    namespace_name: str
    namespace_id: str


class CreateNFSShareResponse(BaseModel):
    nfs_id: str


class GetNFSShareRequest(BaseModel):
    account_id: str
    namespace_id: str


class GetNFSShareResponse(BaseModel):
    nfs_id: Union[str, None] = None


class DeleteNFSShareRequest(BaseModel):
    nfs_id: str
    account_id: str


class CreatNFSClientRequest(BaseModel):
    client_name: str
    nfs_id: str
    account_id: str


class CreateNFSClientResponse(BaseModel):
    nfs_client_id: str


class GetNFSClientRequest(BaseModel):
    nfs_id: str
    account_id: str


class GetNFSClientResponse(BaseModel):
    nfs_client_id: Union[str, None] = None


class DeleteNFSClientRequest(BaseModel):
    account_id: str
    client_id: str


class GetInterfaceRequest(BaseModel):
    ip: str


class GetInterfaceResponse(BaseModel):
    interface: str


class PrepareNFSClientRequest(BaseModel):
    user_name: str = Field(None, description="Device Management's user name")
    password: str = Field(None, description="Device Management's user's password", alias='passwd')
    account_id: str = Field(0, description="Resource account's id")
    pool_id: str = Field(0, description="Storage pool's id")
    namespace_name: str
    namespace_id: str
    nfs_id: str
    client_name: str
    client_id: str


class MoveFileRequest(BaseModel):
    origin_file_path: str
    destination_file_path: str


class UnpackPackageRequest(BaseModel):
    hash_value: str
    package_name: str
    package_size: str


class GetPackageStatusResponse(BaseModel):
    status: str


class InstallSmartkubeRequest(BaseModel):
    package_name: str
    node_ip: str
    device_type: str


class InstallSimbaOSRequest(BaseModel):
    base64_encoded_config: str
    device_type: str


class ExpandSimbaOSRequest(BaseModel):
    base64_encoded_config: str


class DeleteSimbaOSNodeRequest(BaseModel):
    node_name: str


class ResetSimbaOSRequest(BaseModel):
    base64_encoded_config: str


class PreUpgradeSimbaOSRequest(BaseModel):
    package_name: str
    node_ip: str
    cert_type: str


class UpgradeSimbaOSRequest(BaseModel):
    cert_type: str


class SimbaOSNodeRole(BaseModel):
    name: str
    role: str


class GetSimbaosStatusResponse(BaseModel):
    status: str
    version: Union[str, None] = None
    nodes: Union[List[SimbaOSNodeRole], None] = None


class PreinstallDataProtectRequest(BaseModel):
    image_package_name: str


class SynchronizeCertRequest(BaseModel):
    host_ip: str
    username: str
    password: str


class InstallDataProtectRequest(BaseModel):
    chart_package_name: str
    pm_replicas: int
    pe_replicas: int
    device_type: str


class UpgradeDataProtectRequest(BaseModel):
    chart_package_name: str
    master_replicas: int
    worker_replicas: int


class InstallDataBackupRequest(BaseModel):
    chart_package_name: str = Field(None, description="chart package's name")
    image_package_name: str = Field(None, description="image_package's name")
    replicas: str = Field(None, description="Number of master nodes")
    gaussdb_pwd: str = Field(None, description="The password of gaussdb", alias='passwd')


class ExpandDataBackupRequest(BaseModel):
    chart_package_name: str
    master_replicas: int
    worker_replicas: int
    device_type: str


class DataBackupExpandDataBackupRequest(BaseModel):
    chart_package_name: str
    device_type: str
    replicas: int
    node1: str
    node2: str
    node3: str


class GetDataProtectStatusResponse(BaseModel):
    status: str
    chart_name: Union[str, None]


class GetHostnameResponse(BaseModel):
    hostname: Union[str, None]


class RequestUpgradeDpserver(BaseModel):
    upgrade_package_name: str


class GetVersion(BaseModel):
    version: str

