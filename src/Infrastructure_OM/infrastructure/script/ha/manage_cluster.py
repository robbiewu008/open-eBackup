# coding: utf-8
import json
import os
import ssl
from urllib import request

from common.logger.logger import get_logger
from kmc.common import kmc_decrypt, initialize_kmc

node_name = os.getenv('NODE_NAME', "node-0")
pod_name = os.getenv('POD_NAME', "")
log_file = os.path.join(os.sep, 'opt', 'OceanProtect', 'logs', node_name,
                        'infrastructure', 'ha', 'scriptlog', 'ha.log')
logger = get_logger(log_file)

STORAGE_CONFIGMAP_URL = "https://infrastructure.dpa.svc.cluster.local:8088/v1/infra/configmap/setup?nameSpace=dpa" \
                        "&configMap=cluster-conf"
DATABACKUP_CONFIGMAP_URL = "https://infrastructure.dpa.svc.cluster.local:8088/v1/infra/configmap/setup?nameSpace=dpa" \
                           "&configMap=multicluster-conf"
SERVICE_URL = "https://infrastructure.dpa.svc.cluster.local:8088/v1/infra/pod/add/label"

HTTP_SUCCESS_STATUS = 200
SUCCESS_CODE = 0
FAIL_CODE = 1

CA_FILE = "/opt/OceanProtect/infrastructure/cert/internal/ca/ca.crt.pem"
CERT_FILE = "/opt/OceanProtect/infrastructure/cert/internal/internal.crt.pem"
KEY_FILE = "/opt/OceanProtect/infrastructure/cert/internal/internal.pem"
INFRA_CERT = "/opt/OceanProtect/infrastructure/cert/internal/internal_cert"
EXTER_CERT = "/opt/OceanProtect/protectmanager/cert/internal/OpenAPI/OpenAPI.cnf"


def change_cluster_role(role):
    if os.environ.get("DEPLOY_TYPE") == 'd8':
        if role == "PRIMARY":
            url = f'{DATABACKUP_CONFIGMAP_URL}&configKey=HA_PRIMARY&configValue={node_name}'
            url_service = f'{SERVICE_URL}?podName={pod_name}&type=role&tag=primary'
        else:
            url = f'{DATABACKUP_CONFIGMAP_URL}&configKey=HA_STANDBY&configValue={node_name}'
            url_service = f'{SERVICE_URL}?podName={pod_name}&type=role&tag=standby'
        config_res = change_cluster_role_databackup(role, url, "POST")
        label_res = change_cluster_role_databackup(role, url_service, "GET")
        res = config_res and label_res
    else:
        url = f'{STORAGE_CONFIGMAP_URL}&configKey=CLUSTER_ROLE&configValue={role}'
        res = change_cluster_role_databackup(role, url, "POST")
    return res


def change_cluster_role_databackup(role, url, method):
    """
    通过infrastructure接口修改集群角色
    :param role: 集群角色，主集群Primary，备集群Standby
    :param url: 集群设置标记接口
    :param method: 集群接口类型
    :return: True
    """
    context = ssl.SSLContext(ssl.PROTOCOL_TLSv1_2)
    context.check_hostname = False
    context.load_cert_chain(
        certfile=CERT_FILE,
        keyfile=KEY_FILE,
        password=kmc_decrypt(INFRA_CERT, logger)
    )
    context.load_verify_locations(cafile=CA_FILE)
    context.verify_mode = ssl.CERT_REQUIRED
    headers = {"Content-Type": "application/json"}
    try:
        req = request.Request(url, method=method, headers=headers)
        response = request.urlopen(req, context=context)
        res_body = json.loads(response.read().decode('utf-8'))
        data = res_body.get("data")
    except Exception:
        logger.error(f'get data from url response failed.')
        return FAIL_CODE
    if response.status == HTTP_SUCCESS_STATUS:
        if data == 'success':
            logger.info(f'change cluster role to {role} success.')
            return SUCCESS_CODE
        else:
            err = res_body.get("error")
            logger.error(f'change cluster role to {role} failed, code:{err.get("errId")}, desc:{err.get("errMsg")}.')
            return FAIL_CODE
    else:
        logger.error(f'change cluster role to {role} failed, status:{response.status}.')
        return FAIL_CODE


def get_cert_password(type):
    """
    获取证书私钥密码
    :param:
    :return: string
    """
    external_cert_type = "external"
    internal_cert_type = "internal"
    if type == external_cert_type:
        return kmc_decrypt(EXTER_CERT, logger)
    elif type == internal_cert_type:
        return kmc_decrypt(INFRA_CERT, logger)
    else:
        return ''
