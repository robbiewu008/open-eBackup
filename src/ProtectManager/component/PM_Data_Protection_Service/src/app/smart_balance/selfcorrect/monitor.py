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
import stat
from app.smart_balance.utils.dataset_utils import dimensional_normalization
from app.smart_balance.schemas import option, ModelName, smconst
from app.common import logger
from app.common.toolkit import query_job_list
from app.smart_balance.utils.dataset_utils import decode_job_info_from_database, check_type

log = logger.get_logger(__name__)


def get_record_file(chosen_model):
    record_path = option.seq_record_path
    if chosen_model == ModelName.gbrt:
        record_path = option.gbrt_record_path
    elif chosen_model == ModelName.rf:
        record_path = option.rf_record_path
    elif chosen_model == ModelName.adabr:
        record_path = option.adabr_record_path
    return record_path


def record_predict(job_id, data_amount, chosen_model):
    # record the predicted information from model
    if not os.path.exists(option.file_dir):
        os.makedirs(option.file_dir)
    record_path = get_record_file(chosen_model)
    with os.fdopen(os.open(record_path, os.O_CREAT | os.O_RDWR, stat.S_IWUSR | stat.S_IRUSR), 'a') as txt_file:
        strr = ' '.join([str(job_id), str(data_amount)])
        txt_file.write(strr + '\n')
    log.info(f"Record the prediction success.")


def catch_threshold(predicted_value, real_value, threshold=0.3):
    # compare the predicted values with the real values, diff threshold is defaulted as 30%
    try:
        diff = (float(predicted_value) - float(real_value)) / float(real_value)
    except ZeroDivisionError:
        diff = (float(predicted_value) - 1e-9) / 1e-9
    if diff > threshold:
        return 1
    if diff < -threshold:
        return -1
    return 0


def check_threshold(row, history_amount, count):
    # Auxiliary function that determines the direction of finetune
    if catch_threshold(row[1], history_amount[-count]) == 1:
        return 1
    elif catch_threshold(row[1], history_amount[-count]) == -1:
        return -1
    return 0


def read_history_online(job_id, job_type):
    # read history data from database online
    job_type = check_type.get(job_type, smconst.other_backuptype)
    history_type, history_time, history_amount = [], [], []
    previous_type, previous_backup_time, previous_data_amount = 0.0, 0.0, 0.0
    windowlen_count = 0
    for i in range(smconst.dataset_pages):
        count = 0
        response = query_job_list(
            {'types': ['BACKUP'], 'statusList': ['SUCCESS'],
             'orderBy': 'sourceName', 'orderType': 'asc', 'pageSize': 500, 'startPage': i})

        all_data = json.loads(response)['records']
        for each_data in all_data:
            source_name, backup_type, backup_time, data_amount = decode_job_info_from_database(each_data)
            count += 1
            if str(source_name) != str(job_id):
                continue
            history_type.append(float(backup_type))
            last_time_scale, time_scale_length = dimensional_normalization(float(backup_time))
            last_size_scale, size_scale_length = dimensional_normalization(float(data_amount))
            history_time.extend([last_time_scale, time_scale_length])
            history_amount.extend([last_size_scale, size_scale_length])
            windowlen_count += 1
            if float(job_type) == float(backup_type):
                previous_type = float(backup_type)
                previous_backup_time = float(backup_time)
                previous_data_amount = float(data_amount)

        if count < smconst.dataset_pages:
            break
    if windowlen_count >= option.window_len:
        previous_seq_data = [
            history_type[-option.window_len:], history_time[-option.window_len:],
            history_amount[-option.window_len:]
        ]
    else:
        previous_seq_data = [[], [], []]
    log.info("Get ANet history data online success.")
    previous_ml_data = [previous_type, previous_backup_time, previous_data_amount]
    log.info("Get ML history data online success.")
    return previous_seq_data, previous_ml_data


def online_finetune(job_id, chosen_model, previous_seq_data, predict_capacity):
    """
    @param job_id: the incoming job's resource name
    @return: history_data_list, need_finetune_ornot_flag, open_AI_mode_or_not
    """
    finetune_flag_bigger, finetune_flag_smaller, finetune_flag = 0, 0, 0
    history_amount = previous_seq_data[-1]
    record_path = get_record_file(chosen_model)

    if not os.path.isfile(record_path) or float(history_amount[-1]) <= 0:
        log.info(f"No Finetune.")
        return predict_capacity

    # seq AI way needs window_len-1 history data
    txt_file = open(record_path, 'r')
    lines = txt_file.readlines()
    # compare the lastest data with the recorded predition values
    lines = reversed(lines)
    count = 1
    for row in lines:
        row = row.strip().split(" ")
        log.info(f"JobID here:{job_id},row[0]:{row[0]}")
        if str(job_id) == str(row[0]):
            finetune_direction = check_threshold(row, history_amount, count)
            log.info(f"Get online finetune direction.")
            log.info(f"Finetune_flag_smaller:{finetune_flag_bigger}, finetune_flag_smaller {finetune_flag_smaller}")
            if finetune_direction > 0:
                finetune_flag_bigger += 1
            if finetune_direction < 0:
                finetune_flag_smaller += 1
            count += 1
        # when the predicted value bigger/smaller than the real value 5 times in 9 times
        # finetune the output prediction model
        if count == option.window_len:
            if finetune_flag_bigger >= smconst.finetune_threshold:
                finetune_flag = -1
            elif finetune_flag_smaller >= smconst.finetune_threshold:
                finetune_flag = 1
    txt_file.close()
    if finetune_flag == -1:
        predict_capacity *= 0.7
        log.info(f"Finetune success.")
    elif finetune_flag == 1:
        predict_capacity *= 1.3
        log.info(f"Finetune success.")
    else:
        log.info(f"No Finetune.")

    return predict_capacity
