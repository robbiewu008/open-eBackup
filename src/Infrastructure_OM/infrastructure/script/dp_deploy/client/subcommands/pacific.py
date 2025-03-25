import logging as log

from config import StoragePoolConfig
from dataprotect_deployment.pacific_interface import PacificClient, PacificNode
from client_exception import EDSFailException, StoragePoolExpandFailException

def create_storage_pool(
    pacific_cli: PacificClient,
    storage_pool_config: StoragePoolConfig,
    pool_name: str
) -> int:
    log.info(f"Start to create storage pool {pool_name}")
    pool_id = pacific_cli.query_storage_pool(pool_name)
    if pool_id is not None:
        log.info(f'Storage pool already exist, pool_id: {pool_id}')
        return pool_id

    servers = get_servers(pacific_cli)
    task_id = pacific_cli.create_storage_pool(
        pool_name, storage_pool_config, servers)
    log.info(f'Create storage pool task id {task_id}')

    log.info('Start to wait for create storage pool task')
    task_status = pacific_cli.wait_for_task(task_id)
    if task_status == 'success':
        log.info('Successfully created storage pool')
        pool_id = pacific_cli.query_storage_pool(pool_name)
        return pool_id

    raise Exception('Failed to create storage pool')


def get_storage_pool_id(pacific_cli: PacificClient, pool_name: str) -> int:
    log.info(f'Start to get storage pool id of {pool_name}')
    pool_id = pacific_cli.query_storage_pool(pool_name)
    return pool_id


def delete_storage_pool(pacific_cli: PacificClient, pool_id: int):
    log.info(f"Start to delete storage pool {pool_id}")
    task_id = pacific_cli.delete_storage_pool(pool_id)
    log.info(f'Delete storage pool task id {task_id}')

    log.info('Start to wait for delete storage pool task')
    task_status = pacific_cli.wait_for_task(task_id)
    if task_status == 'success':
        log.info('Successfully deleted storage pool')
        return

    raise Exception('Failed to delete storage pool')


def check_converged_eds_file_service(
    pacific_cli: PacificClient,
    pool_id: int,
    ip_list: [str]
) -> [str]:
    """检查并返回没有开启文件服务的节点"""
    service_status = pacific_cli.query_converged_eds_file_service(pool_id)
    service_of_pool = service_status.get('service_of_pools')[0]

    result_nodes = []
    for services_of_node in service_of_pool.get('services_on_pool_nodes'):
        for node_ip in services_of_node:
            if node_ip in ip_list and \
               services_of_node[node_ip].get('file_service') != 0:
                result_nodes.append(node_ip)
    return result_nodes


def create_converged_eds_file_service(
    pacific_cli: PacificClient,
    pool_id: int
):

    log.info('Start to create converged EDS file service')

    service_status = pacific_cli.query_converged_eds_file_service(pool_id)
    service_of_pool = service_status.get('service_of_pools')[0]

    is_file_service_exist = True
    for services_of_node in service_of_pool.get('services_on_pool_nodes'):
        for node_ip in services_of_node:
            if services_of_node[node_ip].get('file_service') != 0:
                is_file_service_exist = False
                break

    if is_file_service_exist:
        log.info("Converged EDS file service already exsit")
        return

    task_id = pacific_cli.create_converged_eds_file_service(pool_id)

    log.info('Start to wait for create creating converged EDS service task')
    task_status = pacific_cli.wait_for_task(task_id)
    if task_status == 'success':
        log.info('Successfully created converged EDS service')
        return

    raise EDSFailException


def delete_converged_eds_file_service(pacific_cli: PacificClient, pool_id):
    log.info('Start to delete converged EDS file service')
    task_id = pacific_cli.delete_converged_eds_file_service(pool_id)
    log.info(f'Delete converged EDS file service task id {task_id}')

    log.info('Start to wait for delete converged EDS file service task')
    task_status = pacific_cli.wait_for_task(task_id)
    if task_status == 'success':
        log.info('Successfully deleted converged EDS file service')
        return

    raise Exception('Failed to delete storage pool')


def expand_converged_eds_file_service(cli: PacificClient, pool_id, ip_list):
    nodes_disabled = check_converged_eds_file_service(cli, pool_id, ip_list)

    ip_list = [ip for ip in ip_list if ip in nodes_disabled]
    if len(ip_list) == 0:
        log.warning('The converged eds file service already enabled')
        return
    log.info(f'Start to expand converged EDS file service at {ip_list}')
    task_id = cli.expand_converged_eds_file_service(pool_id, ip_list)
    log.info(f'Expand converged EDS file service task id {task_id}')

    log.info('Start to wait for expand converged EDS file service task')
    task_status = cli.wait_for_task(task_id)
    if task_status == 'success':
        log.info('Successfully expand converged EDS file service')
        return
    raise StoragePoolExpandFailException


def scale_down_converged_eds_file_service(cli: PacificClient, pool_id, ip_list):
    nodes_disabled = check_converged_eds_file_service(cli, pool_id, ip_list)
    for ip in ip_list:
        if ip in nodes_disabled:
            continue
        # 只支持一次缩容一个节点
        log.info(f'Start to delete converged EDS file service at {ip}')
        task_id = cli.scale_down_converged_eds_file_service(pool_id, [ip])
        log.info(f'Scale down converged EDS file service task id {task_id}')

        log.info('Start to wait for scale donw converged EDS file service task')
        task_status = cli.wait_for_task(task_id)
        if task_status == 'success':
            log.info(f'Successfully delete converged EDS file service at {ip}')
            return
        raise Exception('Failed to scale down converged eds file service')
    log.info(f"Successfully scale down converged eds file service at {ip_list}")


def create_account(pacific_cli: PacificClient, account_name) -> str:
    log.info('Start to create storage account')
    account_id = pacific_cli.query_account(account_name)
    if account_id is not None:
        log.info(f"Account already exists, account_id={account_id}")
        return account_id
    (account_name, account_id) = pacific_cli.create_account(account_name, 0)
    log.info('Successfully created storage account')
    return str(account_id)


def get_servers(pacific_cli: PacificClient) -> [PacificNode]:
    servers = pacific_cli.get_servers()
    return [s for s in servers if s.is_management() or s.is_storage()]
