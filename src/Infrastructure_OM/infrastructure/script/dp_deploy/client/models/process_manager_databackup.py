from typing import List, Literal, TypedDict, Optional
import os
import random
from dataclasses import asdict
import logging as log
from copy import deepcopy
from concurrent.futures import ThreadPoolExecutor

from client_exception import ExpandedNodesAbnormalException
from models.process_manager_base import (
    ProcessManagerBase,
    DeployProcess,
    gen_deploy_process,
)
from subcommands.packages import PackageManager
from subcommands.simbaos import SimbaosManager, get_expected_simbaos_master_cnt, run_postupgrade_paraller
from subcommands.dataprotect import DataProtectManager, get_pm_pe_replicas, UpgradeT
from config import (
    SimbaOSInstallConfig,
    SimbaOSExpandConfig,
    DataBackupConfig,
    CommonNodeConfig,
)

INSTALL_TEMPLATE_FILE = "template/simbaos_databackup/install.yaml.template"
EXPAND_TEMPLATE_FILE = "template/simbaos_databackup/expand.yaml.template"
TemplateType = Literal[
    "template/simbaos_databackup/install.yaml.template",
    "template/simbaos_databackup/expand.yaml.template",
]


class TemplateDicttype(TypedDict):
    install: TemplateType
    expand: TemplateType


class ProcessManagerDataBackup(ProcessManagerBase):
    def __init__(
        self, config: DataBackupConfig, no_rollback: bool, skip_upgrade_simbaos=False, sysadmin="", password=""
    ):
        super().__init__(config, no_rollback)
        self.preliminary_dpclient = self.config.preliminary_node.dpclient
        self.ready_to_handle_nodes: list[CommonNodeConfig] = []
        self.package_paths = None
        template_dict: TemplateDicttype = {
            "install": INSTALL_TEMPLATE_FILE,
            "expand": EXPAND_TEMPLATE_FILE,
        }
        self.simbaos_package_name = os.path.basename(self.config.simbaos_package_path)
        self.image_package_name = os.path.basename(self.config.image_package_path)
        self.chart_package_name = os.path.basename(self.config.chart_package_path)

        self.package_manager = PackageManager(self.config)
        self.simbaos_manager = SimbaosManager(self.config, template_dict)
        self.dataprotect_manager = DataProtectManager(self.config)
        self.dp_nodes: list[CommonNodeConfig] = []
        self.simbaos_nodes: list[SimbaosManager] = []
        temp = self.config.preliminary_node
        self.expand_nodes: list[SimbaosManager] = []
        if self.config.expand_nodes:
            for node in self.config.expand_nodes:
                self.config.preliminary_node = node
                node_config = deepcopy(self.config)
                self.expand_nodes.append(SimbaosManager(node_config, template_dict))

        for node in self.config.nodes.values():
            self.config.preliminary_node = node
            node_config = deepcopy(self.config)
            self.simbaos_nodes.append(SimbaosManager(node_config, template_dict))
            self.dp_nodes.append(node)

        self.skip_upgrade_simbaos = True if config.skip_upgrade_simbaos == "True" else False

        self.config.preliminary_node = temp
        self.already_in_simbaos_cluster = False
        self.upgrade_sysadmin = sysadmin
        self.upgrade_password = password

    def _upload_package(self):
        try:
            now_nodes = self.simbaos_manager.get_status()["nodes"]

            now_nodes_internal_ip: set[Optional[str]] = (
                set([node["internal_ip"] for node in now_nodes]) if now_nodes else set()
            )
            already_in_master_cluster_nodes: set[Optional[str]] = set()
            nodes_have_simbaos: set[Optional[str]] = set()
            nodes_have_dataprotect: set[Optional[str]] = set()
            for node in self.expand_nodes:
                node_internal_ip = node.config.preliminary_node.internal_address
                if node_internal_ip in now_nodes_internal_ip:
                    already_in_master_cluster_nodes.add((node_internal_ip))
                status = node.get_status()["status"]
                if status == "deployed":
                    nodes_have_simbaos.add(node_internal_ip)
                try:
                    dataprotect_status = node.get_dataprotect_status()
                    if dataprotect_status["status"] == "deployed":
                        nodes_have_dataprotect.add(node_internal_ip)
                except Exception as e:
                    log.info("Fail to get node's dataprotect status.")
                    continue
        except Exception as e:
            log.error(f"Fail to get simbaos status, {e}")
            raise e
        nodes_in_other_cluseter = nodes_have_dataprotect - already_in_master_cluster_nodes
        nodes_already_in_mater_cluster = nodes_have_dataprotect & already_in_master_cluster_nodes
        nodes_only_have_simbaos = nodes_have_simbaos - nodes_have_dataprotect - already_in_master_cluster_nodes

        if nodes_already_in_mater_cluster or nodes_in_other_cluseter or nodes_only_have_simbaos:
            raise ExpandedNodesAbnormalException(
                nodes_already_in_mater_cluster,
                nodes_in_other_cluseter,
                nodes_only_have_simbaos,
            )

        self.package_manager.upload_packages(self.ready_to_handle_nodes, self.package_paths)

    def _upgrade_upload_package(self):
        self.package_manager.upload_packages(self.ready_to_handle_nodes, self.package_paths)

    def _delete_package(self):
        self.package_manager.delete_packages(self.ready_to_handle_nodes, self.package_paths)

    def _preinstall_simbaos(self):
        self.simbaos_manager.preisntall(self.simbaos_package_name, self.ready_to_handle_nodes)

    def _rollback_preinstall_simbaos(self):
        pass

    def _install_simbaos(self):
        if self.already_in_simbaos_cluster:
            return
        simbaos_install_config = SimbaOSInstallConfig(
            hosts=[self.config.preliminary_node],
            masters=[],
            workers=[],
            master0=self.config.preliminary_node.node_name,
            control_plane_endpoint=self.config.k8sVIP,
            service_plane_endpoint=self.config.service_plane_endpoint,
            service_plane_endpoint_v6=self.config.service_plane_endpoint_v6,
            kube_pods_cidr=self.config.kube_pods_cidr,
            kube_service_cidr=self.config.kube_service_cidr,
        )

        self.simbaos_manager.install(**asdict(simbaos_install_config))

    def _reset_simbaos(self):
        self.simbaos_manager.reset(self.ready_to_handle_nodes)

    def _preinstall_dataprotect(self):
        self.dataprotect_manager.preinstall(self.ready_to_handle_nodes, self.image_package_name)

    def _reset_preisntall_dataprotect(self):
        pass

    def _install_dataprotect(self):
        self.dataprotect_manager.install(self.chart_package_name)

    def _reset_dataprotect(self):
        self.dataprotect_manager.reset()

    def _expand_node_simbaos(self):
        if self.already_in_simbaos_cluster:
            return
        simbaos_nodes = self.preliminary_dpclient.simbaos_get_status()["nodes"]
        now_master_cnt = len([i["role"] == "master" for i in simbaos_nodes])
        master_cnt = get_expected_simbaos_master_cnt(simbaos_nodes + self.ready_to_handle_nodes)
        add_master_num = master_cnt - now_master_cnt
        master_in_ready_to_handle_node = random.sample(self.ready_to_handle_nodes, add_master_num)

        for node in master_in_ready_to_handle_node:
            node.role = "master"

        simbaos_expand_config = SimbaOSExpandConfig(master=self.config.preliminary_node, hosts=self.config.expand_nodes)

        self.simbaos_manager.expand(**asdict(simbaos_expand_config))

    def _delete_node_simbaos(self):
        self.simbaos_manager.delete_node(self.ready_to_handle_nodes)
        self.simbaos_manager.reset(self.ready_to_handle_nodes)

    def _pre_expand_dataprotect(self):
        self.dataprotect_manager.pre_expand(self.ready_to_handle_nodes)

    def _pre_upgrade_simbaos(self):
        log.info(f"Start to pre upgrade simbaos")
        with ThreadPoolExecutor() as executor:
            result = []
            for node in self.simbaos_nodes:
                r = executor.submit(
                    node.simbaos_pre_upgrade,
                    self.cert_type,
                )
                result.append(r)
            for r in result:
                r.result()
        log.info(f"Successfully pre upgrade simbaos at nodes {self.expand_nodes}")

    def _upgrade_simbaos_sub(self):
        """package_name, node_ip, cert_type, self.address=Data...(make_dps(address))"""
        log.info(f"Start to upgrade simbaos")
        self.preliminary_dpclient.simbaos_upgrade(self.cert_type)
        log.info(f"Successfully upgrade simbaos")

    def _post_upgrade_simbaos(self):
        log.info(f"Start to post upgrade simbaos")
        with ThreadPoolExecutor() as executor:
            result = []
            for node in self.simbaos_nodes:
                r = executor.submit(
                    node.simbaos_post_upgrade,
                )
                result.append(r)
            for r in result:
                r.result()
        log.info(f"Successfully post upgrade simbaos at nodes {self.expand_nodes}")

    def _upgrade_pre_checks(self):
        body_data = UpgradeT(
            ip=self.config.preliminary_node.ip, user_name=self.upgrade_sysadmin, password=self.upgrade_password
        )
        self.dataprotect_manager.perform_upgrade_pre_checks(body_data)

    def _upgrade_backup(self):
        body_data = UpgradeT(
            ip=self.config.preliminary_node.ip, user_name=self.upgrade_sysadmin, password=self.upgrade_password
        )
        self.dataprotect_manager.perform_upgrade_backup(body_data)

    def _upgrade_post_checks(self):
        self.dataprotect_manager.perform_upgrade_post_checks()

    def _upgrade_simbaos(self):
        self._pre_upgrade_simbaos()
        self._upgrade_simbaos_sub()
        self._post_upgrade_simbaos()

    def _upgrade_dataprotect(self):
        master_replicas, worker_replicas = get_pm_pe_replicas(self.preliminary_dpclient)
        log.info(
            "Start to expand dataprotect cluster, " f"pm_replicas={master_replicas}, " f"pe_replicas={worker_replicas}"
        )
        self.dataprotect_manager.preinstall(self.dp_nodes, self.image_package_name)
        dataprotect_upgrade_dict = {
            "chart_package_name": self.chart_package_name[:-4],
            "master_replicas": master_replicas,
            "worker_replicas": worker_replicas,
            "device_type": "e1000",
        }
        self.dataprotect_manager.upgrade_dataprotect(dataprotect_upgrade_dict)

    def _rollback_pre_expand_dataprotect(self):
        pass

    def _expand_dataprotect(self):
        node_num = len(self.preliminary_dpclient.simbaos_get_status()["nodes"])
        param_dict = {
            "chart_package_name": self.chart_package_name,
            "replicas": node_num,
            "device_type": "databackup",
            "node1": "node-0",
            "node2": "node-1",
            "node3": "node-2",
            "float_ip": self.config.float_ip,
            "gateway_ip": self.config.gateway_ip,
        }
        self.dataprotect_manager.expand(**param_dict)

    def _shrink_dataprotect(self):
        pass

    def define_process(self, task_type) -> List[DeployProcess]:
        self.package_paths = [
            self.config.simbaos_package_path,
            self.config.chart_package_path,
            self.config.image_package_path,
        ]
        if task_type == "install":
            self.ready_to_handle_nodes = []
            self.ready_to_handle_nodes.append(self.config.preliminary_node)

            upload_package_process = gen_deploy_process(self._upload_package, self._delete_package)
            preinstall_simbaos_process = gen_deploy_process(self._preinstall_simbaos, self._rollback_preinstall_simbaos)
            install_simbaos_process = gen_deploy_process(self._install_simbaos, self._reset_simbaos)
            preinstall_dp_process = gen_deploy_process(self._preinstall_dataprotect, self._reset_preisntall_dataprotect)
            install_dataprotect_process = gen_deploy_process(self._install_dataprotect, self._reset_dataprotect)

            process_list = [
                upload_package_process,
                preinstall_simbaos_process,
                install_simbaos_process,
                preinstall_dp_process,
                install_dataprotect_process,
            ]

        elif task_type == "expand":
            self.ready_to_handle_nodes = self.config.expand_nodes
            upload_package_process = gen_deploy_process(self._upload_package, self._delete_package)

            preinstall_simbaos_process = gen_deploy_process(self._preinstall_simbaos, self._rollback_preinstall_simbaos)
            expand_simbaos_process = gen_deploy_process(self._expand_node_simbaos, self._delete_node_simbaos)
            pre_expand_dataprotect_process = gen_deploy_process(
                self._pre_expand_dataprotect, self._rollback_pre_expand_dataprotect
            )
            pre_expand_image_load_dataprotect_process = gen_deploy_process(
                self._preinstall_dataprotect, self._reset_preisntall_dataprotect
            )

            expand_dataprotect_process = gen_deploy_process(self._expand_dataprotect, self._shrink_dataprotect)

            process_list = [
                upload_package_process,
                preinstall_simbaos_process,
                expand_simbaos_process,
                pre_expand_dataprotect_process,
                pre_expand_image_load_dataprotect_process,
                expand_dataprotect_process,
            ]

        elif task_type == "pre_upgrade_check":
            self.cert_type = "e1000"
            self.ready_to_handle_nodes = [i for i in self.config.nodes.values()]
            upgrade_dp_precheck_process = gen_deploy_process(
                self._upgrade_pre_checks, self._rollback_preinstall_simbaos
            )
            process_list = [upgrade_dp_precheck_process]

        elif task_type == "backup":
            self.cert_type = "e1000"
            self.ready_to_handle_nodes = [i for i in self.config.nodes.values()]
            upgrade_dp_databackup_process = gen_deploy_process(self._upgrade_backup, self._rollback_preinstall_simbaos)
            process_list = [upgrade_dp_databackup_process]

        elif task_type == "upload_packages":
            self.cert_type = "e1000"
            self.ready_to_handle_nodes = [i for i in self.config.nodes.values()]
            upload_package_process = gen_deploy_process(self._upgrade_upload_package, self._delete_package)
            process_list = [upload_package_process]


        elif task_type == "pre_upgrade_dataprotect":
            self.cert_type = "e1000"
            self.ready_to_handle_nodes = [i for i in self.config.nodes.values()]
            upgrade_dataprotect_process = gen_deploy_process(
                self._preinstall_dataprotect, self._rollback_pre_expand_dataprotect
            )
            process_list = [upgrade_dataprotect_process]

        elif task_type == "upgrade_dataprotect":
            self.cert_type = "e1000"
            self.ready_to_handle_nodes = [i for i in self.config.nodes.values()]
            upgrade_dataprotect_process = gen_deploy_process(
                self._upgrade_dataprotect, self._rollback_preinstall_simbaos
            )
            process_list = [upgrade_dataprotect_process]

        elif task_type == "upgrade_simbaos":
            self.cert_type = "e1000"
            self.ready_to_handle_nodes = [i for i in self.config.nodes.values()]
            upgrade_simbaos_process = gen_deploy_process(self._upgrade_simbaos, self._rollback_preinstall_simbaos)
            process_list = [upgrade_simbaos_process]

        elif task_type == "post_upgrade_check":
            self.cert_type = "e1000"
            self.ready_to_handle_nodes = [i for i in self.config.nodes.values()]
            upgrade_dp_postcheck_process = gen_deploy_process(
                self._upgrade_post_checks, self._rollback_preinstall_simbaos
            )
            process_list = [upgrade_dp_postcheck_process]

        else:
            raise Exception(f"Not support task type:{task_type}")
        return process_list
