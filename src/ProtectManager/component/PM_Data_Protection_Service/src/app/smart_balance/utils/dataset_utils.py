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
import datetime
import json
import os
import stat

import pytz

from app.common import logger
from app.common.toolkit import query_job_list
from app.smart_balance.schemas import option, smconst

log = logger.get_logger(__name__)

check_type = {
    "full": 1, "cumulative_increment": 2, "difference_increment": 3, "log": 4, "permanent_increment": 5,
    "snapshot": 6
}


def parse_job_size(data_after_reduction):
    # transfrom the job size to MB unit
    if 'TB' in data_after_reduction:
        job_size = float(data_after_reduction.rstrip('TB').rstrip()) * 1024 * 1024
    elif 'GB' in data_after_reduction:
        job_size = float(data_after_reduction.rstrip('GB').rstrip()) * 1024
    elif 'MB' in data_after_reduction:
        job_size = float(data_after_reduction.rstrip('MB').rstrip())
    elif 'KB' in data_after_reduction:
        job_size = float(data_after_reduction.rstrip('KB').rstrip()) / 1024
    elif 'B' in data_after_reduction:
        job_size = float(data_after_reduction.rstrip('B').rstrip()) / 1024 / 1024
    else:
        log.info(
            f'error, unsupported unit in [{data_after_reduction}]! for job size')
        job_size = 0.0

    return job_size


def get_time(filepath):
    if not os.path.isfile(filepath):
        log.info(f"{filepath} has no file, need rebuild")
        return -1
    create_time = os.path.getctime(filepath)
    create_time_utc = datetime.datetime.utcfromtimestamp(create_time).replace(tzinfo=pytz.utc)
    current_time_utc = datetime.datetime.now(pytz.utc)
    time_difference = current_time_utc - create_time_utc
    return time_difference.days


def confirm_refresh_model():
    # retrain model each 3 days
    for _, _, filenames in os.walk(option.save_dir):
        for filename in filenames:
            filepath = os.path.join(option.save_dir, filename)
            time_difference = get_time(filepath)
            if time_difference >= smconst.rm_days:
                os.remove(filepath)


def confirm_reload_database():
    # reload local dataset each 1 day
    time_difference = get_time(option.dataset_path)
    if time_difference == -1:
        return False
    if time_difference >= smconst.rd_days:
        os.remove(option.dataset_path)
        return False
    return True


def decode_job_info_from_database(each_data):
    try:
        extendstr = json.loads(each_data["extendStr"])
        data_after_reduction = extendstr.get("dataAfterReduction", 0)
        data_amount = parse_job_size(data_after_reduction)
        backuptype = extendstr.get("backupType", smconst.other_backuptype)
    except Exception:
        log.info(f"Get from database, unsatisfied format.")
        return [[], [], [], []]

    if float(data_amount) <= 0.0:
        log.info(f"Get from database, unsatisfied data_amount format.")
        return [[], [], [], []]

    source_name = each_data.get("sourceName", [])
    backup_type = check_type.get(backuptype, smconst.other_backuptype)
    start_time = each_data.get("startTime", 0)
    end_time = each_data.get("endTime", 0)
    backup_time = (end_time - start_time) / 1000
    return [source_name, backup_type, backup_time, data_amount]


def write_file(all_data, wfile):
    count = 0
    for each_data in all_data:
        source_name, backup_type, backup_time, data_amount = decode_job_info_from_database(each_data)
        if not source_name:
            continue
        count += 1
        wfile.writelines(
            [str(source_name), " ", str(backup_type), " ", str(backup_time), " ", str(data_amount), "\n"])
    return count


def load_txt_from_database(file_dir, dataset_path):
    log.info("Start load txt")
    if not os.path.exists(file_dir):
        os.makedirs(file_dir)
    with os.fdopen(os.open(dataset_path, os.O_CREAT | os.O_RDWR, stat.S_IWUSR | stat.S_IRUSR), 'w') as txt_file:
        txt_file.writelines(
            ["sourceName ", "backup_type ", "backup_time ", "data_amount ",
             "\n"])
        for page in range(smconst.dataset_pages):
            response = query_job_list(
                {'types': ['BACKUP'], 'statusList': ['SUCCESS'],
                 'orderBy': 'sourceName', 'orderType': 'asc', 'pageSize': 500, 'startPage': page})
            all_data = json.loads(response)['records']
            count = write_file(all_data, txt_file)
            log.info(f"startPage {page} count{count}")
            if count < smconst.dataset_pages:
                break
    log.info("Loaded txt from database")


def dimensional_normalization(input_feature):
    # feature normalization, 34513 -> (0.34513, 5)
    feature_scale_length = len(str(input_feature).split(".")[0])
    feature_scale = float(input_feature) / (10 ** feature_scale_length)
    return feature_scale, feature_scale_length


def mean_data(input_feature):
    feature_sum = 0
    scale_sum = 0
    for i in range(0, len(input_feature), 2):
        feature_sum += input_feature[i] * (10 ** input_feature[i + 1])
        scale_sum += input_feature[i + 1]
    return [feature_sum / (len(input_feature) / 2), scale_sum / (len(input_feature) / 2)]


def fill_nan(input_features, input_size, input_time, input_type):
    # fill data when data is missing
    data_amount, backup_time, type_list = input_features[0], input_features[1], input_features[2]
    his_len = len(type_list)
    less_len = option.window_len - his_len
    if less_len <= 2:
        data_amount.extend(mean_data(data_amount) * less_len)
        backup_time.extend(mean_data(backup_time) * less_len)
        type_list.extend([type_list[-1]] * less_len)
        input_size.append(data_amount)
        input_time.append(backup_time)
        input_type.append(type_list)
    return [input_size, input_time, input_type]


def read_txt_for_seq_train():
    if not os.path.exists(option.dataset_path):
        return [], [], []
    # read local dataset for train model in the sequential form
    log.info("Start read txt for train seq")
    data_amount, backup_time, type_list = [], [], []
    input_type, input_size, input_time = [], [], []
    vm_id_list = []
    with open(option.dataset_path, 'r', encoding="utf-8") as txt_file:
        for row in txt_file.readlines()[1:]:
            row = row.split(" ")
            if row[0] not in vm_id_list:
                input_size, input_time, input_type = fill_nan([data_amount, backup_time, type_list], input_size,
                                                              input_time, input_type)
                data_amount, backup_time, type_list = [], [], []
                type_list.append(int(row[1]))
                last_time_scale, time_scale_length = dimensional_normalization(row[2])
                last_size_scale, size_scale_length = dimensional_normalization(row[3])
                backup_time.extend([last_time_scale, time_scale_length])
                data_amount.extend([last_size_scale, size_scale_length])
                vm_id_list.append(row[0])
            else:
                if len(type_list) == option.window_len:
                    input_size.append(data_amount)
                    input_time.append(backup_time)
                    input_type.append(type_list)
                    data_amount, backup_time, type_list = [], [], []
                type_list.append(int(row[1]))
                last_time_scale, time_scale_length = dimensional_normalization(row[2])
                last_size_scale, size_scale_length = dimensional_normalization(row[3])
                backup_time.extend([last_time_scale, time_scale_length])
                data_amount.extend([last_size_scale, size_scale_length])
    return input_type, input_size, input_time
