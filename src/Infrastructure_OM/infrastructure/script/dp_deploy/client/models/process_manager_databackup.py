from typing import List
import os
import random
from dataclasses import asdict
import logging as log

from models.process_manager_base import ProcessManagerBase, DeployProcess, gen_deploy_process
from subcommands.packages import PackageManager
from subcommands.simbaos import SimbaosManager, get_expected_simbaos_master_cnt
from subcommands.dataprotect import DataProtectManager
from config import SimbaOSInstallConfig, SimbaOSExpandConfig

INSTALL_TEMPLATE_FILE = 'template/simbaos_databackup/install.yaml.template'
EXPAND_TEMPLATE_FILE = 'template/simbaos_databackup/expand.yaml.template'


class ProcessManagerDataBackup(ProcessManagerBase):
    def __init__(self, config, no_rollback: bool):
        super().__init__(config, no_rollback)
        self.preliminary_dpclient = self.config.preliminary_node.dpclient
        self.ready_to_handle_nodes = []
        self.package_paths = None
        template_dict = {"install": INSTALL_TEMPLATE_FILE, "expand": EXPAND_TEMPLATE_FILE}
        self.simbaos_package_name = os.path.basename(self.config.simbaos_package_path)
        self.image_package_name = os.path.basename(self.config.image_package_path)
        self.chart_package_name = os.path.basename(self.config.chart_package_path)

        self.package_manager = PackageManager(self.config)
        self.simbaos_manager = SimbaosManager(self.config, template_dict)
        self.dataprotect_manager = DataProtectManager(self.config)

        self.already_in_simbaos_cluster = False

    def _upload_package(self):
        self.package_manager.upload_packages(self.ready_to_handle_nodes, self.package_paths)

    def _delete_package(self):
        self.package_manager.delete_packages(self.ready_to_handle_nodes, self.package_paths)

    def _preinstall_simbaos(self):
        try:
            now_nodes = self.simbaos_manager.get_status()["nodes"]
            now_nodes_names = []
            if now_nodes:
                now_nodes_names = [node['name'] for node in now_nodes]

            already_in_simbaos_cluster_nodes = []
            for node in self.ready_to_handle_nodes:
                if node.node_name in now_nodes_names:
                    already_in_simbaos_cluster_nodes.append(node.node_name)

            node_in_num = len(already_in_simbaos_cluster_nodes)

            if node_in_num > 0 and node_in_num != len(self.ready_to_handle_nodes):
                raise Exception(f"Only part of nodes in cluster, {already_in_simbaos_cluster_nodes}")

        except Exception as e:
            log.error(f"Failed to preinstall Simbaos, {e}")
            raise e

        if node_in_num == len(self.ready_to_handle_nodes):
            log.info(f"Ready to handle nodes all already in cluster")
            self.already_in_simbaos_cluster = True
            return

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
            service_plane_endpoint="",
            service_plane_endpoint_v6=None,
            kube_pods_cidr=self.config.kube_pods_cidr,
            kube_service_cidr=self.config.kube_service_cidr
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

        simbaos_expand_config = SimbaOSExpandConfig(
            master=self.config.preliminary_node,
            hosts=self.config.expand_nodes)

        self.simbaos_manager.expand(**asdict(simbaos_expand_config))

    def _delete_node_simbaos(self):
        self.simbaos_manager.delete_node(self.ready_to_handle_nodes)
        self.simbaos_manager.reset(self.ready_to_handle_nodes)

    def _pre_expand_dataprotect(self):
        self.dataprotect_manager.pre_expand(self.ready_to_handle_nodes)

    def _rollback_pre_expand_dataprotect(self):
        pass

    def _expand_dataprotect(self):
        node_num = len(self.preliminary_dpclient.simbaos_get_status()["nodes"])
        param_dict = {"chart_package_name": self.chart_package_name, "replicas": node_num, "device_type": "databackup",
                      "node1": "node-0", "node2": "node-1", "node3": "node-2"}
        self.dataprotect_manager.expand(**param_dict)

    def _shrink_dataprotect(self):
        pass

    def define_process(self, task_type) -> List[DeployProcess]:
        if task_type == "install":
            self.ready_to_handle_nodes = []
            self.ready_to_handle_nodes.append(self.config.preliminary_node)

            self.package_paths = [
                                  self.config.simbaos_package_path,
                                  self.config.chart_package_path,
                                  self.config.image_package_path
                                  ]

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
                            install_dataprotect_process
                            ]

        elif task_type == "expand":
            self.ready_to_handle_nodes = self.config.expand_nodes
            self.package_paths = [self.config.simbaos_package_path,
                                  self.config.chart_package_path,
                                  self.config.image_package_path]
            upload_package_process = gen_deploy_process(self._upload_package,
                                                        self._delete_package)

            preinstall_simbaos_process = gen_deploy_process(self._preinstall_simbaos,
                                                            self._rollback_preinstall_simbaos)
            expand_simbaos_process = gen_deploy_process(self._expand_node_simbaos,
                                                        self._delete_node_simbaos)
            pre_expand_dataprotect_process = gen_deploy_process(self._pre_expand_dataprotect,
                                                                self._rollback_pre_expand_dataprotect)
            pre_expand_image_load_dataprotect_process = gen_deploy_process(self._preinstall_dataprotect,
                                                                           self._reset_preisntall_dataprotect)

            expand_dataprotect_process = gen_deploy_process(self._expand_dataprotect,
                                                            self._shrink_dataprotect)

            process_list = [
                            upload_package_process,
                            preinstall_simbaos_process,
                            expand_simbaos_process,
                            pre_expand_dataprotect_process,
                            pre_expand_image_load_dataprotect_process,
                            expand_dataprotect_process
                            ]
        else:
            raise Exception(f"Not support task type:{task_type}")
        return process_list
