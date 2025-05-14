# This file is a part of the open-eBackup project.
# This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
# If a copy of the MPL was not distributed with this file, You can obtain one at
# http://mozilla.org/MPL/2.0/.
#
# Copyright (c) [2024] Huawei Technologies Co.,Ltd.
#
# THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
# EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
# MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
import json
import os
import math
import random
from typing import List
from collections import defaultdict
from app.common import logger
from app.smart_balance.schemas import ExecuteCluster
from app.smart_balance.schemas import option, ModelName, smconst
import app.smart_balance.model as model
from app.smart_balance.utils import load_txt_from_database, confirm_reload_database, confirm_refresh_model
from app.smart_balance.selfcorrect import load_model, record_predict, online_finetune
from app.smart_balance.utils.preprocess_utils import seq_predict, ml_predict
from app.smart_balance.selfcorrect.monitor import read_history_online

log = logger.get_logger(__name__)


def reschedule_job(execute_clusters: List[ExecuteCluster]):
    if execute_clusters[0].running_flag == "first":
        return return_random_pool_ids(execute_clusters)
    confirm_refresh_model()
    if not os.path.exists(option.dataset_path) or not confirm_reload_database():
        load_txt_from_database(option.file_dir, option.dataset_path)
    else:
        log.info("No need reload raw database.")
    # dispatch the incoming job to applicable node
    if not load_model():
        log.info(f"Model is not ready, job {execute_clusters[0].resource_id} use overweight way.")
        # if AI model not ready, use the overweight way
        sortd_node_index = overweight_policy_for_node(execute_clusters)
        return overweight_policy_for_pool(execute_clusters, sortd_node_index)
    else:
        log.info(f"AI schedule model is ready, job {execute_clusters[0].resource_id} use AI way.")
    sortd_node_index, predict_capacity = ai_policy_for_node(execute_clusters)
    return ai_policy_for_pool(execute_clusters, sortd_node_index, predict_capacity)


def return_random_pool_ids(execute_clusters):
    # 第一次分发非实际分发，无需实际运行smart_balance模块
    random_pool_ids = []
    for node in execute_clusters:
        parsed_data = json.loads(node.units)
        for pool_info in parsed_data:
            random_pool_ids.append(pool_info["id"])
    random.shuffle(random_pool_ids)
    log.info(f"No need really run, random dispatch.")
    return random_pool_ids


def get_percent(args1, args2, flag="capacity"):
    if float(args1) < 0 or float(args2) <= 0:
        # 传过来的数值异常，固定优先级为最低
        return 2.0
    # 该指标越小越好, 一旦超过可用容量或者并行任务数，健康度的影响减小，任务选择优先免排队策略
    # 剩余容量不足或者节点可执行的任务数量已经超过极限，超过的比例越多，优先级越接近于最低优先级2
    if float(args1) >= float(args2):
        if flag == "capacity":
            if float(args1) == float(args2):
                return 2.0 - smconst.epsilon * float(args2)
            return 2.0 - smconst.epsilon / ((float(args1) - float(args2)) / float(args2))
        else:
            return 2.0

    if math.isclose(float(args1), 0):
        if flag == "capacity":
            # 节点上还没有放任务，剩余容量越多的指标应该越小
            return smconst.epsilon / float(args2)
        else:
            # 节点上还没有放任务，剩余运行任务数最多，优先级最高
            return 0.0
    return float(args1) / float(args2) + 1


def get_healthy_percent(args1, args2):
    # 该指标越大越好
    if float(args1) > float(args2) or float(args1) < 0 or float(args2) < 0:
        # 传过来的数值异常，固定优先级为1
        return 1.0
    if math.isclose(float(args1), 0):
        # 初始全为0，优先级固定为最高2
        if math.isclose(float(args2), 0):
            return 2.0
        # 传来的节点必定可用，由于历史原因某节点总是表现不好，优先级低，但其他节点线程跑完的情况下应仍使用该节点
        # 成功任务的数量不增加，但是总任务数量增加，执行总任务数量更多的节点表现的更差，优先级更接近于最低优先级1
        return smconst.epsilon / float(args2) + 1
    return float(args1) / float(args2) + 1


def overweight_policy_for_node(nodes: List[ExecuteCluster]):
    # choose bigger weight node, same weight choose smaller used_capacity_percent node
    weight_list = defaultdict()
    # record the map of node index and node esn
    esn_index = defaultdict()
    for index, node in enumerate(nodes):
        esn_index[node.esn] = index
        used_capacity_percent = get_percent(node.used_capacity, node.total_capacity, "capacity")
        healthy = get_healthy_percent(node.history_success_backup_job_count, node.history_backup_job_count)
        corunjob_percent = get_percent(node.running_backup_job_count, node.running_task_spec_count, "corun")
        weight = (2 - used_capacity_percent) * (2 - corunjob_percent) * float(healthy)
        weight_list[node.esn] = {"res_capacity": 2 - used_capacity_percent, "weight": weight}
        log.info(f"Overweight node {node.esn} weight:{weight_list[node.esn]['weight']}")
        log.info(
            f"Overweight trace, res_capacity_percent, healthy, res_corunjob_percent:"
            f"{2 - used_capacity_percent}, {healthy}, {2 - corunjob_percent}")
    sorted_keys = sorted(weight_list, key=lambda x: (weight_list[x]["weight"], weight_list[x]["res_capacity"]),
                         reverse=True)

    log.info(f"Get the order of cluster nodes:{sorted_keys}")
    sorted_nodes = [esn_index[index] for index in sorted_keys]
    return sorted_nodes


def overweight_policy_for_pool(nodes: List[ExecuteCluster], sortd_node_index):
    # choose bigger weight node, same weight choose smaller used_capacity_percent node
    total_sorted_pool_id_list = []
    # record sorted nodes' pool weight
    for node_index in sortd_node_index:
        weight_list = defaultdict()
        parsed_data = json.loads(nodes[node_index].units)
        for pool_info in parsed_data:
            used_capacity_percent = get_percent(pool_info["usedCapacityPool"], pool_info["totalCapacityPool"],
                                                "capacity")
            healthy = get_healthy_percent(pool_info["historySuccessBackupJobCountPool"],
                                          pool_info["historyBackupJobCountPool"])
            weight = (2 - used_capacity_percent) * float(healthy)
            weight_list[pool_info["id"]] = {"res_capacity": 2 - used_capacity_percent, "weight": weight}
            log.info(f"Overweight node {nodes[node_index].esn} pool weight:{weight_list[pool_info['id']]['weight']}")
        sorted_keys = sorted(weight_list, key=lambda x: (weight_list[x]["weight"], weight_list[x]["res_capacity"]),
                             reverse=True)
        total_sorted_pool_id_list.extend(sorted_keys)
    log.info(f"Get the order of pool ids:{total_sorted_pool_id_list}")
    return total_sorted_pool_id_list


def ai_policy_for_node(nodes: List[ExecuteCluster]):
    previous_seq_data, previous_ml_data = read_history_online(nodes[0].esn, nodes[0].backup_type)
    predict_capacity, chosen_model = online_interference(previous_seq_data, previous_ml_data)
    if chosen_model == "None":
        log.info(f"AI model not fit, job {nodes[0].resource_id}  get the order of cluster nodes from overweight_policy")
        return overweight_policy_for_node(nodes), -1
    else:
        log.info(f"Get job {nodes[0].resource_id} online interference result success {predict_capacity}.")
        predict_capacity = online_finetune(nodes[0].esn, chosen_model, previous_seq_data, predict_capacity)
        record_predict(nodes[0].esn, predict_capacity, chosen_model)

    weight_list = defaultdict()
    esn_index = defaultdict()
    for index, node in enumerate(nodes):
        # predict value is MB，nodes capacity gives KB
        esn_index[node.esn] = index
        used_capacity_percent = get_percent(float(node.used_capacity) / 1024 + predict_capacity,
                                            float(node.total_capacity) / 1024, "capacity")
        healthy = get_healthy_percent(node.history_success_backup_job_count, node.history_backup_job_count)
        corunjob_percent = get_percent(node.running_backup_job_count, node.running_task_spec_count, "corun")
        weight = (2 - used_capacity_percent) * (2 - corunjob_percent) * float(healthy)
        weight_list[node.esn] = {"res_capacity": 2 - used_capacity_percent, "weight": weight}
        log.info(f"AI node {node.esn} weight:{weight_list[node.esn]['weight']}")
        log.info(
            f"AI trace, res_capacity_percent, healthy, res_corunjob_percent:"
            f"{2 - used_capacity_percent}, {healthy}, {2 - corunjob_percent}")

    sorted_keys = sorted(weight_list, key=lambda x: (weight_list[x]["weight"], weight_list[x]["res_capacity"]),
                         reverse=True)
    log.info(f"Get the order of cluster nodes from AI model:{sorted_keys}")
    sorted_nodes = [esn_index[index] for index in sorted_keys]
    return sorted_nodes, predict_capacity


def ai_policy_for_pool(nodes: List[ExecuteCluster], sortd_node_index, predict_capacity):
    if predict_capacity == -1:
        return overweight_policy_for_pool(nodes, sortd_node_index)
    total_sorted_pool_id_list = []
    # record sorted nodes' pool weight
    for node_index in sortd_node_index:
        weight_list = defaultdict()
        parsed_data = json.loads(nodes[node_index].units)
        for pool_info in parsed_data:
            # predict value is MB，nodes capacity gives KB
            used_capacity_percent = get_percent(float(pool_info["usedCapacityPool"]) + predict_capacity * 1024,
                                                float(pool_info["totalCapacityPool"]), "capacity")
            healthy = get_healthy_percent(pool_info["historySuccessBackupJobCountPool"],
                                          pool_info["historyBackupJobCountPool"])
            weight = (2 - used_capacity_percent) * float(healthy)
            weight_list[pool_info["id"]] = {"res_capacity": 2 - used_capacity_percent, "weight": weight}

            log.info(f"AI node {nodes[node_index].esn} pool weight:{weight_list[pool_info['id']]['weight']}")
        sorted_keys = sorted(weight_list, key=lambda x: (weight_list[x]["weight"], weight_list[x]["res_capacity"]),
                             reverse=True)
        total_sorted_pool_id_list.extend(sorted_keys)
    log.info(f"Get the order of pool ids from AI model:{total_sorted_pool_id_list}")
    return total_sorted_pool_id_list


def online_interference(previous_seq_data, previous_ml_data):
    # make AI inference online from trained model
    predict_list = defaultdict(int)
    if os.path.isfile(option.seq_model_path):
        predict_list[ModelName.anet] = seq_predict(previous_seq_data, model.StaticContainer.seq_model)
    if os.path.isfile(option.gbrt_model_path):
        predict_list[ModelName.gbrt] = ml_predict(previous_ml_data, model.StaticContainer.gbrt_model, ModelName.gbrt)
    if os.path.isfile(option.rf_model_path):
        predict_list[ModelName.rf] = ml_predict(previous_ml_data, model.StaticContainer.rf_model, ModelName.rf)
    if os.path.isfile(option.adabr_model_path):
        predict_list[ModelName.adabr] = ml_predict(previous_ml_data, model.StaticContainer.ada_model, ModelName.adabr)
    # if predict < 0 means lack history data, remove
    predict_list = {key: value for key, value in predict_list.items() if value > 0}
    if predict_list:
        # vote predict results from multiple ai models for HA
        mean_value = sum(predict_list.values()) / len(predict_list.values())
        diff, result, chosen_model = 1e9, 0, "None"
        for key, value in predict_list.items():
            if abs(value - mean_value) < diff:
                result = value
                chosen_model = key
                diff = abs(value - mean_value)
        return result, chosen_model
    return -1, "None"
