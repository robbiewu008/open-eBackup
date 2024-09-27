from dataprotect_deployment.dp_server_interface import DataProtectDeployClient
import logging as log


def namespace_create(
    cli: DataProtectDeployClient,
    namespace_name: str,
    account_id: str,
    pool_id: str
):
    namespace_id = cli.get_namespace(namespace_name, account_id)
    if namespace_id is None:
        namespace_id = cli.create_namespace(
            account_id, namespace_name, pool_id
        )
    log.info(f'Successfully created namespace {namespace_name}, '
             f'namespace_id={namespace_id}')

    nfs_id = cli.get_nfs_share(account_id, namespace_id)
    if nfs_id is None:
        nfs_id = cli.create_nfs_share(account_id, namespace_name, namespace_id)
    log.info(f'Successfully created nfs share, nfs_id={nfs_id}')

    client_id = cli.get_nfs_client(nfs_id, account_id)
    if client_id is None:
        client_id = cli.create_nfs_client('127.0.0.1', nfs_id, account_id)
    log.info(f'Successfully created nfs share client, client id={client_id}')


def namespace_get(
    cli: DataProtectDeployClient,
    namespace_name,
    account_id
):
    return cli.get_namespace(namespace_name, account_id)


def namespace_delete(
    cli: DataProtectDeployClient,
    namespace_name,
    account_id
):
    log.info(f'Start to delete namespace {namespace_name}')

    # delete share path
    namespace_id = cli.get_namespace(namespace_name, account_id)
    if namespace_id is None:
        log.info(f'Namespace {namespace_name} not found, '
                 f'skip delete namespace')
        return

    nfs_id = cli.get_nfs_share(account_id, namespace_id)
    if nfs_id is not None:
        log.info(f'Start to delete nfs share, nfs_id: {nfs_id}')
        cli.delete_nfs_share(nfs_id, account_id)
        log.info('Succesdfully deleted nfs_share')

    cli.delete_namespace(namespace_name, account_id)
    log.info(f'Successfully deleted namespace {namespace_name}')
