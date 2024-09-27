#
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
#

import importlib
import json
import time
from contextlib import contextmanager
from functools import wraps
from queue import Queue
from threading import Thread

from common.common import output_result_file
from common.common_models import SubJobDetails, ActionResult, LogDetail
from common.const import SubJobStatusEnum, ExecuteResultEnum, DBLogLevel
from common.exception.common_exception import ErrCodeException
from common.number_const import NumberConst
from common.parse_parafile import ParamFileUtil
from common.util.backup import query_progress
from common.util.backup_utils import BackupStatus
from mongodb import LOGGER
from mongodb.comm.const import ErrorCode, REPORT_INTERVAL_SEC, ActionCode
from mongodb.comm.utils import job_report
from validation.common.json_util import find_all_value_by_key
from validation.validator import ParamValidator


def report_job_details(job_id: str, sub_job_details: dict):
    try:
        result_info = job_report(job_id, sub_job_details)
    except Exception as err:
        LOGGER.exception(f"Invoke rpc_tool interface exception, err: {err}.")
        return False
    if not result_info:
        return False
    ret_code = result_info.get("code", -1)
    if ret_code != 0:
        LOGGER.error("Invoke rpc_tool interface failed, result code: %s.", ret_code)
        return False
    return True


@contextmanager
def report_progress(report_step, *report_args):
    rp_thread = Thread(target=report_step, args=report_args)
    rp_thread.setDaemon(True)
    rp_thread.start()
    yield rp_thread
    rp_thread.join()


def record_result(ignore_normal_record=False, ignore_exception_record=False):
    """
    输出执行结果装饰器，任务上报输出结果统一入口
    :param output:输出结构体
    :param ignore_normal_record:（执行结果正常）是否忽略上报
    :param ignore_exception_record: （执行结果异常）是否忽略上报
    """

    def decorate(func):
        @wraps(func)
        def inner(obj, *args):
            """
            :param obj: restore_service实例
            """
            pid = obj.pid
            excepted = False
            code = ExecuteResultEnum.SUCCESS
            try:
                func(obj, *args)
            except ErrCodeException as ex:
                excepted = True
                err_code, err_msg = ex.error_code, ex.error_message
                LOGGER.exception(f"Execute task failed, function name: {func.__name__}, error code: {err_code}, "
                                 f"error message: {err_msg}.")
            except Exception as ex:
                excepted = True
                LOGGER.exception(f"Execute task failed. Function Name: {func.__name__}.")
                err_code = ExecuteResultEnum.INTERNAL_ERROR
                err_msg = f"Execute task failed. Error Message: {str(ex)}."
            else:
                err_code = ExecuteResultEnum.SUCCESS
                err_msg = ""
            output = obj.result
            if ignore_normal_record and excepted == ignore_exception_record:
                return output
            if excepted:
                code = ExecuteResultEnum.INTERNAL_ERROR
            if output is None:
                output = ActionResult(code=code)
            if isinstance(output, ActionResult):
                output.code = code
                output.body_err = err_code
                output.message = err_msg
                output = output.dict(by_alias=True)
            if isinstance(output, SubJobDetails):
                output = output.dict(by_alias=True)
            try:
                output_result_file(pid, output)
            except Exception as not_named_e:
                LOGGER.error("Write output file failed with err:%s", str(not_named_e))
                raise not_named_e
            return output

        return inner

    return decorate


class JobManager:
    """
      处理插件具体接口任务，任务状态、统一流程、结果记录、进度上报、
    """
    _service_map = {}

    def __init__(self, pid, job_id=None, sub_job_id=None):
        """
        :param pid: 插件下发process id
        :param job_id: 插件下发任务Id/参数任务Id
        :param sub_job_id: 插件下发子任务Id/参数子任务Id
        """
        self.pid = pid
        self.job_id = job_id if job_id else ""
        self.sub_job_id = sub_job_id if sub_job_id else ""
        self.status = SubJobStatusEnum.RUNNING
        self.exception = None
        self.progress = 0
        self.per_progress = 0
        self.data_size = 0
        self.log_map = {}
        self._report = False
        self._report_queue = Queue()
        self.report_detail = SubJobDetails(
            taskId=self.job_id,
            subTaskId=self.sub_job_id,
            taskStatus=self.status,
            progress=self.progress
        )
        self.result = None

    @classmethod
    def register(cls, interface):
        """
        注册接口服务装饰器，将接口服务注册到job
        :param interface: 脚本接口
        :return: wrapper
        """

        def _wrapper(key, obj):
            cls._service_map[key] = obj
            return obj

        if callable(interface):
            name = interface.__name__
            return _wrapper(name, interface)

        return lambda x: _wrapper(interface, x)

    @classmethod
    def get_interface_service(cls, interface):
        if not cls._service_map:
            cls.load_service_model()
        return cls._service_map.get(interface)

    @classmethod
    def load_service_model(cls):
        importlib.import_module("service", package=__file__)

    def update(self, result_struct):
        """
        更新接口任务结果，需要上报的执行一次上报
        :param result_struct: server_cls 执行结果
        :return:
        """
        self.result = result_struct
        err_code = 0
        msg = ""
        if isinstance(self.result, ActionResult):
            err_code = self.result.body_err if self.result.body_err else self.result.code
            msg = self.result.message
        elif isinstance(self.result, SubJobDetails):
            self.status = self.result.task_status
            self.data_size = self.result.data_size
            if self.result.log_detail:
                log = self.result.log_detail[0]
                err_code = log.log_detail if log.log_detail else 0
            self._report_queue.put(self.result)
            self._update_report_detail()
            report_job_details(self.job_id, result_struct.dict(by_alias=True))
        if err_code != ActionCode.SUCCESS:
            self.status = SubJobStatusEnum.FAILED
            self.exception = ErrCodeException(
                err_code=err_code,
                message=msg
            )

    def report_job_detail(self, interval_sec=REPORT_INTERVAL_SEC):
        report = self._report
        while report:
            if not self._is_running():
                report = False
                interval_sec = 0
            report_detail = self._update_report_detail()
            try:
                ret = report_job_details(self.job_id, report_detail.dict(by_alias=True))
            except Exception as not_named_e:
                LOGGER.exception(f"Report failed with exception:{not_named_e}.")
                continue
            if not ret:
                LOGGER.error("Failed report this time, check the report tools")
            time.sleep(interval_sec)
        LOGGER.debug("Finish report sub job detail, detail: %s", self.report_detail.dict(by_alias=True))

    def report_exception(self, err: ErrCodeException):
        if not self._report:
            return True
        self.status = SubJobStatusEnum.FAILED
        self._update_report_detail()
        body_err = err.error_code
        param = err.parameter_list
        msg = err.error_message
        log_info = self.log_map.get("failed", "")
        if log_info:
            log_detail = LogDetail(logInfo=log_info, logInfoParam=[self.sub_job_id], logDetail=body_err,
                                   logDetailParam=param,
                                   logDetailInfo=[msg], logLevel=DBLogLevel.ERROR.value)
            self.report_detail.log_detail = [log_detail]
        ret = report_job_details(self.job_id, self.report_detail.dict(by_alias=True))
        return ret

    def execute_steps(self, steps, report=True):
        """
        实现接口任务的步骤调度
        :param steps: server_cls 接口任务执行步骤
        :param report: 接口任务是否需要上报进度
        :return:
        """
        if not report:
            self._execute_steps(steps)
            self._update_progress()
            return
        self._report = report
        with report_progress(self.report_job_detail) as report_th:
            try:
                self._execute_steps(steps)
            except ErrCodeException:
                report_th.join()
                raise

    @record_result()
    def run(self, interface_func):
        LOGGER.info("Begin to execute interface job: %s, job id: %s.", interface_func, self.job_id)
        param_dict = self._parse_params()
        service_cls = JobManager.get_interface_service(interface_func)
        service = service_cls(self, param_dict)
        steps = service.get_steps()
        report = service.need_report
        self.log_map.update(service.log_detail)
        self.execute_steps(steps, report=report)
        LOGGER.info("Execute interface job: %s completed. job id: %s.", interface_func, self.job_id)

    def _is_running(self):
        if self.status != SubJobStatusEnum.RUNNING:
            return False
        self._update_progress()
        if self.progress == NumberConst.HUNDRED:
            self.status = SubJobStatusEnum.COMPLETED
            return False
        return True

    def _update_progress(self):
        status, progress, data_size = query_progress(self.job_id)
        LOGGER.debug("Update_progress status:%s, progress:%s, data_size:%s", status, progress, data_size)
        if status != BackupStatus.BACKUP_FAILED.value:
            self.progress = self.per_progress * progress // 100
        else:
            if progress:
                self.status = SubJobStatusEnum.FAILED
            else:
                self.progress = self.per_progress

    def _update_report_detail(self):
        if not self._report_queue.empty():
            self.report_detail = self._report_queue.get()
            self.status = self.report_detail.task_status
            self.progress = self.report_detail.progress
            return self.report_detail
        self.report_detail.task_status = self.status
        self.report_detail.progress = self.progress
        self.report_detail.data_size = self.data_size
        if self.status == SubJobStatusEnum.RUNNING:
            lab_key = "start"
            log_level = DBLogLevel.INFO.value
        elif self.status == SubJobStatusEnum.COMPLETED:
            lab_key = "success"
            log_level = DBLogLevel.INFO.value
        else:
            lab_key = "failed"
            log_level = DBLogLevel.ERROR.value
        log_info = self.log_map.get(lab_key)
        job_id = self.sub_job_id if self.sub_job_id else self.job_id
        if log_info:
            log_detail = LogDetail(
                logInfo=log_info, logInfoParam=[job_id],
                logDetailInfo=[], logLevel=log_level)
            self.report_detail.log_detail = [log_detail]
        return self.report_detail

    def _parse_params(self):
        """解析任务参数文件"""
        mongodb_schema_path = "mongodb/jsonschema/mongo_base_define.json"
        try:
            parm_cont_dict = ParamFileUtil.parse_param_file_and_valid_by_schema(self.pid, mongodb_schema_path)

            cluster_nodes_list = find_all_value_by_key(parm_cont_dict, "clusterNodes")
            for cluster_nodes in cluster_nodes_list:
                if cluster_nodes:
                    ParamValidator.valid_data_by_schema(json.loads(cluster_nodes),
                                                        "mongodb/jsonschema/cluster_nodes_define.json")

            vpc_info_list = find_all_value_by_key(parm_cont_dict, "vpc_info")
            for vpc_info in vpc_info_list:
                if vpc_info:
                    ParamValidator.valid_data_by_schema(json.loads(vpc_info),
                                                        "mongodb/jsonschema/vpc_info_define.json")

            LOGGER.info("Param is valid!")

        except Exception as inner_err:
            msg = "Parse param file failed as %s." % inner_err
            LOGGER.error(msg)
            raise ErrCodeException(
                err_code=ErrorCode.ERROR_PARAM,
                message=msg
            ) from inner_err
        if not parm_cont_dict:
            msg = "Param dict is none."
            LOGGER.error(msg)
            raise ErrCodeException(
                err_code=ErrorCode.ERROR_PARAM,
                message=msg
            )
        return parm_cont_dict

    def _execute_steps(self, steps):
        if not steps:
            self.progress = NumberConst.HUNDRED
            self.status = SubJobStatusEnum.COMPLETED
            return
        per_progress = NumberConst.HUNDRED // len(steps)
        self.per_progress = NumberConst.HUNDRED % len(steps)
        for step in steps:
            try:
                step()
            except ErrCodeException as err:
                self.report_exception(err)
                raise
            except Exception as inner_err:
                err = ErrCodeException(
                    err_code=ErrorCode.OPERATE_FAILED,
                    message=str(inner_err)
                )
                self.report_exception(err)
                raise err from inner_err
            else:
                if self.exception:
                    self.report_exception(self.exception)
                    raise self.exception
                self.per_progress += per_progress
