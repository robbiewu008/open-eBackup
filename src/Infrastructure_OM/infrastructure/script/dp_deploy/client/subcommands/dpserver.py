import tarfile
import os
import yaml
import logging as log
import time
import consts
from tabulate import tabulate
from dataprotect_deployment.client_manager import ClientManager
from dataprotect_deployment.client_manager import run_in_parallel
from dataprotect_deployment.dp_server_interface import DataProtectDeployClient
from config import PacificNode
from utils import retry


def get_versions(
    client_mgr: ClientManager,
    nodes: [PacificNode]
) -> [(str, str)]:
    versions = run_in_parallel(
        client_mgr,
        nodes,
        DataProtectDeployClient.dpserevr_get_version
    )
    return versions


def extract_version_from_package(package_path: str) -> str:
    with tarfile.open(package_path, 'r:gz') as tar:
        file_in_tar = tar.extractfile('./manifest.yml')
        if file_in_tar:
            content = file_in_tar.read()
            manifest = yaml.safe_load(content)
            return manifest['Version']


def format_check_dpserver_version_err_msg(hostname_versions):
    data = [['Node', 'VERSION']]
    for (hostname, version) in hostname_versions:
        item = [hostname]
        item.append(version if version is not None else 'FAILED')
        data.append(item)
    return tabulate(data, headers='firstrow')


@retry(tries=6, delay=30)
def check_dpserver_versions(
    client_mgr: ClientManager,
    nodes: [PacificNode]
):
    hostname_versions = get_versions(client_mgr, nodes)
    versions = [v[1] for v in hostname_versions]
    if all(v == versions[0] for v in versions):
        return
    # dpserver's versions are not all the same
    err_msg = format_check_dpserver_version_err_msg(hostname_versions)
    raise Exception(
        f"The versions of dpserver are not all the same:\n"
        f"{err_msg}"
    )


def umount(client_mgr: ClientManager, nodes: [PacificNode]):
    node_names = [n.name for n in nodes]
    log.info(f'Start to umount nfs at {node_names}')
    run_in_parallel(
        client_mgr,
        nodes,
        DataProtectDeployClient.umount_nfs_share
    )
    log.info(f'Successfully umount nfs at {node_names}')


def upgrade(
    client_mgr: ClientManager,
    package_path: str,
    upgrade_nodes: [PacificNode]
):
    upgrade_version = extract_version_from_package(package_path)
    if upgrade_version is None:
        raise Exception("Invalid upgrade package")

    versions = get_versions(client_mgr, upgrade_nodes)
    log.info(f"Before upgrade, versions is {versions}")
    failed_nodes = [node for node, version in versions if version is None]
    if len(failed_nodes) > 0:
        raise Exception(f"Failed to get version info from {failed_nodes}")
    # check if all versions are lower or equal than expected
    if not all([version <= upgrade_version for _, version in versions]):
        failed_nodes = [(n, v) for n, v in versions if v > upgrade_version]
        err_msg = [
            f'{node} dpserver-{version} > dpserver-{upgrade_version}; '
            for (node, version) in failed_nodes
        ]
        raise Exception("Unable to upgrade dpserver. " + ''.join(err_msg))

    package_name = os.path.basename(package_path)
    run_in_parallel(
        client_mgr,
        upgrade_nodes,
        DataProtectDeployClient.dpserver_upgrade,
        package_name,
    )

    # check version pre 10s, up to 2mins
    upgrade_succeed_nodes = []
    for i in range(int(consts.DPSERVER_UPGRADE_TIMEOUT / consts.DPSERVER_UPGRADE_WAIT_INTERNAL)):
        versions = get_versions(client_mgr, upgrade_nodes)
        for (n, version) in versions:
            if n not in upgrade_succeed_nodes and version == upgrade_version:
                log.info(f'Successfully upgraded dpserver at {n}')
                upgrade_succeed_nodes.append(n)
        if len(upgrade_succeed_nodes) == len(upgrade_nodes):
            log.info('Successfully upgraded dpserver at all nodes')
            return
        time.sleep(consts.DPSERVER_UPGRADE_WAIT_INTERNAL)

    upgrade_failed_nodes = [
        n.name for n in upgrade_nodes
        if n not in upgrade_succeed_nodes
    ]
    raise Exception(f"Failed to upgrade dpserver at {upgrade_failed_nodes}, "
                    f"Try again")
