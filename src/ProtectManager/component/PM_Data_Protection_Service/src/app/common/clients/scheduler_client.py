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
import warnings
from http import HTTPStatus
from typing import List

from app.common import logger
from app.common.clients.client_util import SystemBaseHttpsClient, decode_response_data, parse_response_data
from app.common.enums.schedule_enum import ScheduleTypes, ExecuteType
from app.common.exception.common_error_codes import CommonErrorCodes
from app.common.exception.unified_exception import EmeiStorBizException

LOGGER = logger.get_logger(__name__)
DEFAULT_SCAN_INTERVAL = 3600


class SchedulerClient(object):
    @staticmethod
    def create_interval_schedule(
            schedule_id,
            policy,
            schedule_type,
            protected_obj,
            chain_id
    ):
        warnings.warn("create_interval_schedule is deprecated",
                      DeprecationWarning)
        url = f'/v1/schedules'
        policy_schedule = policy["schedule"]
        schedule_req = {
            "schedule_name": schedule_id,
            "interval": f'{policy_schedule["interval"]}{policy_schedule["interval_unit"]}',
            "schedule_type": schedule_type,
            "action": "schedule." + policy["type"],
            "params": json.dumps({
                "resource_id": protected_obj.resource_id,
                "sla_id": str(protected_obj.sla_id),
                "chain_id": chain_id,
                "policy": policy,
                "execute_type": ExecuteType.AUTOMATIC.value
            }),
            'start_date': policy_schedule["start_time"]
        }
        LOGGER.info(f'invoke api to create schedule, request url is {url}')
        response = SystemBaseHttpsClient().request("POST", url, body=json.dumps(schedule_req))
        if response.status == HTTPStatus.OK:
            return parse_response_data(response.data).get("schedule_id")
        else:
            LOGGER.info(f'invoke api to create schedule, url={url}, schedule_req={schedule_req}')

    @staticmethod
    def create_interval_schedule_new(
            schedule_id,
            schedule_interval,
            schedule_type,
            start_time,
            topic,
            params
    ):
        url = f'/v1/schedules'
        schedule_req = {
            "schedule_name": schedule_id,
            "interval": schedule_interval,
            "schedule_type": schedule_type,
            "action": topic,
            "params": json.dumps(params),
            'start_date': start_time
        }
        if not schedule_id:
            schedule_req.update(schedule_name=schedule_id)
        LOGGER.info(f'invoke api to create schedule, request url is {url}')
        response = SystemBaseHttpsClient().request("POST", url, body=json.dumps(schedule_req))
        if response.status == HTTPStatus.OK:
            return parse_response_data(response.data).get("schedule_id")
        else:
            LOGGER.info(f'invoke api to create schedule, url={url}, schedule_req={schedule_req}')
            raise EmeiStorBizException(CommonErrorCodes.SYSTEM_ERROR,
                                       message="Create interval schedule task failed")

    @staticmethod
    def create_customize_interval_schedule(
            schedule_id,
            day_of_week,
            day_of_month,
            daily_start_time,
            daily_end_time,
            schedule_type,
            topic,
            params
    ):
        url = f'/v1/schedules'
        schedule_req = {
            "schedule_name": schedule_id,
            "day_of_week": day_of_week,
            "day_of_month": day_of_month,
            "daily_start_time": daily_start_time,
            "daily_end_time": daily_end_time,
            "schedule_type": schedule_type,
            "action": topic,
            "params": json.dumps(params)
        }
        LOGGER.info(f'invoke api to create customize interval schedule, request url is {url}')
        exception_info = f'invoke api to create customize interval schedule, url={url}, schedule_req={schedule_req}'
        response = SystemBaseHttpsClient().request("POST", url, body=json.dumps(schedule_req),
                                                   build_exception=True, exception_info=exception_info)
        return parse_response_data(response.data).get("schedule_id")

    @staticmethod
    def get_schedule_next_time(schedule_id):
        url = f'/v1/schedules/next-time/{schedule_id}'
        LOGGER.info(f'invoke api to get schedule next time, request url is {url}')
        response = SystemBaseHttpsClient().request("GET", url, build_exception=True)
        if response.status == HTTPStatus.OK:
            return decode_response_data(response.data)
        else:
            LOGGER.info(f'invoke api to get schedule next time, url={url}, schedule_id={schedule_id}')

    @staticmethod
    def create_immediate_schedule(topic, params):
        url = f'/v1/schedules'
        schedule_req = {
            "schedule_type": ScheduleTypes.immediate.value,
            "action": topic,
            "params": json.dumps(params)
        }
        LOGGER.info(f'invoke api to create schedule, request url is {url}')
        response = SystemBaseHttpsClient().request("POST", url, body=json.dumps(schedule_req))
        if response.status == HTTPStatus.OK:
            return decode_response_data(response.data)
        else:
            LOGGER.error(f'invoke api to create schedule, url={url}, schedule_req={schedule_req}')

    @staticmethod
    def batch_delete_schedules(
            schedule_id_list: List[str]
    ) -> bool:
        for schedule_id in schedule_id_list:
            url = f'/v1/schedules/{schedule_id}'
            response = SystemBaseHttpsClient().request("DELETE", url)
            if response.status != HTTPStatus.OK:
                raise EmeiStorBizException(CommonErrorCodes.SYSTEM_ERROR,
                                           message="delete schedule failed")
        return True

    @staticmethod
    def submit_immediate_job(job_action, job_params):
        """
        提交立即执行job
        :param job_action: Job的action
        :param job_params: Job参数
        :return:
        """
        schedule_req = {
            "schedule_type": ScheduleTypes.immediate.value,
            "action": job_action,
            "params": job_params.json()
        }
        response = SystemBaseHttpsClient().request(
            "POST", f'/v1/schedules', body=json.dumps(schedule_req))
        if response.status != HTTPStatus.OK:
            raise EmeiStorBizException(CommonErrorCodes.SYSTEM_ERROR,
                                       message="Submit immediate job failed")

    @staticmethod
    def submit_interval_job(job_action, job_params=None, job_name=None, interval=DEFAULT_SCAN_INTERVAL,
                            start_date=None):
        """
        提交定时执行Job
        :param start_date:
        :param interval:
        :param job_name:
        :param job_action: Job的Action
        :param job_params: Job的参数
        :return:
        """
        schedule_req = {
            'schedule_type': ScheduleTypes.interval.value,
            'interval': str(interval) + "s",
            'action': job_action,
            'params': job_params.json() if job_params else "{}"
        }
        if start_date:
            schedule_req["start_date"] = start_date
        else:
            # 不设置则开始时间为当前时间延后一个周期
            start_time = (datetime.datetime.now() + datetime.timedelta(seconds=interval)).strftime(
                '%Y-%m-%d %H:%M:%S')
            schedule_req["start_date"] = start_time
        if job_name:
            schedule_req["schedule_name"] = job_name
        response = SystemBaseHttpsClient().request(
            "POST", f'/v1/schedules', body=json.dumps(schedule_req))
        if response.status != HTTPStatus.OK:
            raise EmeiStorBizException(CommonErrorCodes.SYSTEM_ERROR,
                                       message="Submit interval job failed")
