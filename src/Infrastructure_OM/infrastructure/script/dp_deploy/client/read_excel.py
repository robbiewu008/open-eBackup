from typing import TypedDict, Optional
import pandas as pd

from client_exception import (
    IncorrectFormatException,
    TooManyMasterNodeException,
    NodeTypeNotSupportException,
)


class Node_Config(TypedDict):
    ip: str
    simbaos_ip: str
    internal_address: str
    user: str
    passwd: str
    kadmin: str
    kadmin_passwd: str


class E1000_Config(TypedDict):
    preliminary_node: Node_Config
    expand_nodes: list[Node_Config]
    k8sVIP: str
    k8sServiceVIP: str
    kube_pods_cidr: str
    kube_service_cidr: str
    float_ip: Optional[str]
    gateway_ip: Optional[str]
    service_plane_endpoint: Optional[str]
    service_plane_endpoint_v6: Optional[str]


INFO_DICT = {
    "*节点外部IP": ["ip", "simbaos_ip"],
    "*节点内部IP": ["internal_address"],
    "*SSH用户名": ["user"],
    "*SSH用户密码": ["passwd"],
    "*K8S用户名": ["kadmin"],
    "*K8S用户密码": ["kadmin_passwd"],
}

TRANSLATE_DICT = {
    "*节点类型": ["*节点类型", "*Node Type"],
    "*节点外部IP": ["*节点外部IP", "*External IP Address"],
    "*节点内部IP": ["*节点内部IP", "*Internal IP Address"],
    "*SSH用户名": ["*SSH用户名", "*SSH Username"],
    "*SSH用户密码": ["*SSH用户密码", "*SSH User Password"],
    "*K8S用户名": ["*K8S用户名", "*K8s Username"],
    "*K8S用户密码": ["*K8S用户密码", "*K8s User Password"],
    "K8S高可用IP": ["K8S高可用IP", "K8s HA IP Address"],
    "GaussDB高可用IP": ["GaussDB高可用IP", "GaussDB HA IP Address"],
    "仲裁网关": ["仲裁网关", "Quorum Gateway"],
    "主节点": ["主节点", "Active node"],
    "待扩节点": ["待扩节点", "Node to be added"],
    "集群浮动IP": ["集群浮动IP", "Cluster Float IP"],
    "集群浮动IPv6": ["集群浮动IPv6", "Cluster Float IPv6"],
}

SUPPORT_LANGUAGE = {"CN": 0, "EN": 1}

lang = 0


def trans(info):
    return TRANSLATE_DICT[info][lang]


def read_xlsx(path):
    global lang
    df = pd.read_excel(path, skiprows=1)
    if df.columns[0] == "*节点类型":
        language = 'CN'
    elif df.columns[0] == "*Node Type":
        language = "EN"
    else:
        raise IncorrectFormatException

    lang = SUPPORT_LANGUAGE[language]

    config: E1000_Config = {}

    master_exist = False
    rows, columns = df.shape
    for row in range(rows):
        node_config: Node_Config = {}
        for info in INFO_DICT.keys():
            element = df.iloc[row][trans(info)]
            single_config = {i: element for i in INFO_DICT[info]}
            node_config.update(single_config)

        node_type = df.iloc[row][trans("*节点类型")]
        if node_type == trans("主节点") and master_exist:
            raise TooManyMasterNodeException
        elif node_type == trans("主节点"):
            config["preliminary_node"] = node_config
            master_exist = True
        elif node_type == trans("待扩节点"):
            config.setdefault("expand_nodes", []).append(node_config)
        else:
            raise NodeTypeNotSupportException

    config["k8sVIP"] = config["k8sServiceVIP"] = df.iloc[0][trans("K8S高可用IP")]
    config["kube_pods_cidr"] = df.iloc[0]["kube_pods_cidr"]
    config["kube_service_cidr"] = df.iloc[0]["kube_service_cidr"]
    config["float_ip"] = df.iloc[0][trans("GaussDB高可用IP")]
    config["float_ip"] = None if pd.isna(config["float_ip"]) else config["float_ip"]
    config["gateway_ip"] = df.iloc[0][trans("仲裁网关")]
    config["gateway_ip"] = (
        None if pd.isna(config["gateway_ip"]) else config["gateway_ip"]
    )

    config["service_plane_endpoint"] = df.iloc[0][trans("集群浮动IP")]
    config["service_plane_endpoint"] = (
        None
        if pd.isna(config["service_plane_endpoint"])
        else config["service_plane_endpoint"]
    )

    config["service_plane_endpoint_v6"] = df.iloc[0].get(trans("集群浮动IPv6"))
    config["service_plane_endpoint_v6"] = (
        None
        if pd.isna(config["service_plane_endpoint_v6"])
        else config["service_plane_endpoint_v6"]
    )
    return config
