import os
import logging as log
import hashlib

from client_exception import FileTypeNotSupportException, FilePathNotExistException
from dataprotect_deployment.dp_server_interface import DataProtectDeployClient
from dataprotect_deployment.client_manager import ClientManager
import consts


def calculate_file_sha256(file_path):
    sha256_hash = hashlib.sha256()
    with open(file_path, "rb") as file:
        for byte_block in iter(lambda: file.read(4096), b""):
            sha256_hash.update(byte_block)
    hash_value = sha256_hash.hexdigest()
    return hash_value


def upload_per_client(client, packages):
    for package_path in packages:
        if package_path:
            upload(client, package_path)


def get_package_name_list_from_paths(packages):
    package_names = []
    for path in packages:
        package_names.append(os.path.basename(path))
    return package_names


def delete_per_client(client, package_names):
    for package_name in package_names:
        delete(client, package_name)


class PackageManager:
    def __init__(self, config):
        self.config = config

    @staticmethod
    def upload_packages(nodes, packages):
        for node in nodes:
            client = node.dpclient
            upload_per_client(client, packages)

    def delete_packages(self, nodes, packages):
        nodes = list(nodes)
        packages = list(packages)
        package_names = get_package_name_list_from_paths(packages)
        for node in nodes:
            client = self.config.nodes[node.ip].dpclient
            delete_per_client(client, package_names)


def upload(cli: DataProtectDeployClient, package_path: str):
    log.info(f'Start to upload package {package_path}')

    if not os.path.isfile(package_path):
        return

    package_size = os.path.getsize(package_path)
    hash_value = calculate_file_sha256(package_path)
    package_status = cli.unpack_package(hash_value, package_path, package_size)
    if package_status == "exist":
        log.info(f"Package already exist, {package_path}")
        return
    cli.upload_package(package_size, package_path)
    log.info(f'Successfully uploaded package {package_path}')


def delete(cli: DataProtectDeployClient, package_name: str):
    log.info(f'Start to delete package {package_name}')
    cli.delete_package(package_name)
    log.info(f'Successfully deleted package {package_name}')
