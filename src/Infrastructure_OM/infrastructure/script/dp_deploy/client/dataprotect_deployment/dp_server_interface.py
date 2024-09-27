import requests
import os
from requests_toolbelt import MultipartEncoder
from utils import make_dpserver_address, check_result


class DataProtectDeployClient:
    def __init__(self, address: str, token: str = None, user: str = None, passwd: str = None):
        self.address = make_dpserver_address(address)

        self.session = requests.Session()
        self.session.headers.update({'X-AUTH-TOKEN': token})
        self.session.headers.update({'SSH-USER': user})
        self.session.headers.update({"SSH-PASSWD": passwd})
        self.session.verify = False

    def gethostname(self) -> str:
        url = f"https://{self.address}/v1/dp_deploy/converaged_service/hostname"
        r = self.session.get(url)
        check_result(r, 'Failed to get hostname')
        return r.json().get('hostname')

    def get_interface_from_ip(self, ip):
        url = f"https://{self.address}/v1/dp_deploy/converaged_service/interface"
        r = self.session.post(url, json={
            "ip": ip
        })
        check_result(r, 'Failed to get interface info')
        return r.json().get('interface')

    def create_namespace(self, account_id: str,
                         namespace_name: str, pool_id: str) -> str:
        url = f"https://{self.address}/v1/dp_deploy/converaged_service/namespace"
        r = self.session.post(url, json={
            'account_id': account_id,
            'namespace_name': namespace_name,
            'pool_id': pool_id
        })

        check_result(r, 'Failed to create namespace')
        return r.json().get('namespace_id')

    def get_namespace(self, namespace_name: str, account_id: str) -> str:
        url = f"https://{self.address}/v1/dp_deploy/converaged_service/namespace"
        r = self.session.get(url, json={
            'namespace_name': namespace_name,
            'account_id': account_id
        })
        check_result(r, 'Failed to get namespace')
        return r.json().get('namespace_id')

    def delete_namespace(self, namespace_name: str, account_id: str):
        url = f"https://{self.address}/v1/dp_deploy/converaged_service/namespace"
        r = self.session.delete(url, json={
            'namespace_name': namespace_name,
            'account_id': account_id
        })
        check_result(r, 'Failed to delete namespace')

    def create_nfs_share(self, account_id: str, namespace_name: str,
                         namespace_id: str) -> str:
        url = f"https://{self.address}/v1/dp_deploy/converaged_service/nfs_share"
        r = self.session.post(url, json={
            'account_id': account_id,
            'namespace_name': namespace_name,
            'namespace_id': namespace_id
        })
        check_result(r, f"Failed to create nfs_share"
                        f"namespace_name: {namespace_name}, "
                        f"namespace_id: {namespace_id}, "
                        f"account_id:{account_id}")
        return r.json().get('nfs_id')

    def get_nfs_share(self, account_id: str, namespace_id: str) -> str:
        url = f"https://{self.address}/v1/dp_deploy/converaged_service/nfs_share"
        r = self.session.get(url, json={
            'namespace_id': namespace_id,
            'account_id': account_id
        })
        check_result(r, f'Failed to get nfs share, '
                        f'account_id: {account_id}, '
                        f'namespace_id:{namespace_id}')
        return r.json().get('nfs_id')

    def delete_nfs_share(self, nfs_id: str, account_id: str):
        url = f"https://{self.address}/v1/dp_deploy/converaged_service/nfs_share"
        r = self.session.delete(url, json={
            'nfs_id': nfs_id,
            'account_id': account_id
        })
        check_result(r, f'Failed to delete nfs share, nfs_id: {nfs_id}')

    def umount_nfs_share(self):
        url = f"https://{self.address}/v1/dp_deploy/converaged_service/umount"
        r = self.session.post(url)
        check_result(r, f'Failed to umount nfs at {self.address}')

    def create_nfs_client(self, client_name: str,
                          nfs_id: str, account_id: str) -> str:
        url = f"https://{self.address}/v1/dp_deploy/converaged_service/client"
        r = self.session.post(url, json={
            'client_name': client_name,
            'nfs_id': nfs_id,
            'account_id': account_id,
        })
        check_result(r, f'Failed to create nfs client, nfs_id: {nfs_id}')
        return r.json().get('nfs_client_id')

    def get_nfs_client(self, nfs_id: str, account_id: str):
        url = f"https://{self.address}/v1/dp_deploy/converaged_service/client"
        r = self.session.get(url, json={
            'nfs_id': nfs_id,
            'account_id': account_id
        })
        check_result(r, f'Failed to get nfs client: nfs_id: {nfs_id}, '
                        f'account_id: {account_id}')
        return r.json().get('nfs_client_id')

    def delete_nfs_client(self, account_id: str, client_id: str):
        url = f"https://{self.address}/v1/dp_deploy/converaged_service/client"
        r = self.session.get(url, json={
            'account_id': account_id,
            'client_id': client_id
        })
        check_result(r, f'Failed to delete nfs client failed: '
                        f'account_id: {account_id}, client_id: {client_id}')

    def unpack_package(self, hash_value, package_path, package_size):
        url = f"https://{self.address}/v1/dp_deploy/package/unpack_package"
        package_name = os.path.basename(package_path)
        r = self.session.post(url, json={
            "hash_value": hash_value,
            "package_name": package_name,
            "package_size": str(package_size)
        })
        check_result(r, f"Failed to unpack package: {package_name}")
        return r.json().get("status")

    def upload_package(self, package_size, package_path: str):
        url = f"https://{self.address}/v1/dp_deploy/package/upload"
        filename = os.path.basename(package_path)
        with open(package_path, 'rb') as package_file:
            m = MultipartEncoder(fields={
                'package_size': str(package_size),
                'file': (filename, package_file)
            })
            r = self.session.post(url, data=m,
                                  headers={'Content-Type': m.content_type})
            check_result(r, f'Failed to upload package: {package_path}')

    def delete_package(self, package_name: str):
        url = f"https://{self.address}/v1/dp_deploy/package/delete/{package_name}"
        r = self.session.delete(url)
        check_result(r, f'Failed to delete package: {package_name}')

    def simbaos_get_status(self):
        url = f'https://{self.address}/v1/dp_deploy/app/simbaos/status'
        r = self.session.get(url)
        check_result(r, 'Failed to get simbaos status')
        return r.json()

    def simbaos_preinstall(self, package_name: str, node_ip: str, device_type: str):
        url = f"https://{self.address}/v1/dp_deploy/app/simbaos/preinstall"
        r = self.session.post(url, json={
            'package_name': package_name,
            'node_ip': node_ip,
            'device_type': device_type
        })
        check_result(r, f'Failed to preinstall simbaos at {self.address}, '
                        f'package_name: {package_name}')

    def simbaos_install(self, base64_encoded_config: str, device_type: str):
        url = f"https://{self.address}/v1/dp_deploy/app/simbaos/install"
        r = self.session.post(url, json={
            'base64_encoded_config': base64_encoded_config,
            'device_type': device_type
        })
        check_result(r, 'Failed to install simbaos')

    def simbaos_preupgrade(self, package_name: str, node_ip: str, cert_type: str):
        url = f"https://{self.address}/v1/dp_deploy/app/simbaos/pre_upgrade"
        r = self.session.post(url, json={
            'package_name': package_name,
            'node_ip': node_ip,
            'cert_type': cert_type
        })
        check_result(r, f'Failed to pre upgrade simbaos at {self.address}, '
                        f'package_name: {package_name}')

    def simbaos_upgrade(self, cert_type: str):
        url = f"https://{self.address}/v1/dp_deploy/app/simbaos/upgrade"
        r = self.session.post(url, json={
            'cert_type': cert_type
        })
        check_result(r, f'Failed to upgrade simbaos')

    def simbaos_postupgrade(self):
        url = f"https://{self.address}/v1/dp_deploy/app/simbaos/post_upgrade"
        r = self.session.post(url)
        check_result(r, f'Failed to upgrade simbaos')

    def simbaos_expand(self, base64_encoded_config: str):
        url = f"https://{self.address}/v1/dp_deploy/app/simbaos/expand"
        r = self.session.post(url, json={
            'base64_encoded_config': base64_encoded_config
        })
        check_result(r, 'Failed to expand simbaos')

    def simbaos_reset(self):
        url = f"https://{self.address}/v1/dp_deploy/app/simbaos/reset"
        r = self.session.post(url)
        check_result(r, 'Failed to reset simbaos')

    def simbaos_delete_node(self, node_name):
        url = f"https://{self.address}/v1/dp_deploy/app/simbaos/delete_node"
        r = self.session.post(url, json={
            'node_name': node_name
        })
        check_result(r, 'Failed to delete simbaos node')

    def dataprotect_status(self):
        url = f"https://{self.address}/v1/dp_deploy/app/dataprotect/status"
        r = self.session.get(url)
        check_result(r, 'Failed to get dataprotect status')
        return r.json()

    def dataprotect_preinstall(self, image_package_name: str):
        url = f"https://{self.address}/v1/dp_deploy/app/dataprotect/preinstall"
        r = self.session.post(url, json={
            'image_package_name': image_package_name
        })
        check_result(r, 'Failed to preinstall databackup')

    def dataprotect_install(
            self,
            chart_package_name: str,
            pm_replicas: int,
            pe_replicas: int,
            device_type: str
    ):
        url = f"https://{self.address}/v1/dp_deploy/app/dataprotect/install"
        r = self.session.post(url, json={
            'chart_package_name': chart_package_name,
            'pm_replicas': pm_replicas,
            'pe_replicas': pe_replicas,
            'device_type': device_type
        })
        check_result(r, 'Failed to install dataprotect')

    def dataprotect_reset(self):
        url = f"https://{self.address}/v1/dp_deploy/app/dataprotect/reset"
        r = self.session.delete(url)
        check_result(r, 'Failed to reset dataprotect')

    def dataprotect_expand(self, **kwargs):
        url = f"https://{self.address}/v1/dp_deploy/app/dataprotect/expand"
        r = self.session.post(url, json=kwargs)
        check_result(r, 'Failed to expand dataprotect')

    def dpserver_upgrade(self, upgrade_package_name: str):
        url = f"https://{self.address}/v1/dp_deploy/dpserver/upgrade"
        r = self.session.post(url, json={
            'upgrade_package_name': upgrade_package_name
        })
        check_result(r, 'Failed to upgrade dpserver')

    def dpserevr_get_version(self):
        url = f"https://{self.address}/v1/dp_deploy/dpserver/version"
        try:
            r = self.session.get(url)
            if r.status_code == requests.codes.ok:
                return r.json().get('version')
        except Exception:
            return None

    def sync_cert(
            self,
            host_ip,
            username,
            password):
        url = f"https://{self.address}/v1/dp_deploy/app/dataprotect/syn_cert"
        r = self.session.post(url, json={
            'host_ip': host_ip,
            'username': username,
            'password': password
        })
        check_result(r, 'Failed to sync cert')
