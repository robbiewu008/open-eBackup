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

import abc

from common.common_models import ActionResult, SubJobDetails
from common.const import ReportDBLabel


class MetaService(metaclass=abc.ABCMeta):
    def __init__(self, job_manager, param_dict):
        self.job_manager = job_manager
        self.param_dict = param_dict
        self.log_detail = {}

    @abc.abstractmethod
    def get_steps(self):
        """
        接口服务的任务详细步骤
        :return list
        """
        pass

    @abc.abstractmethod
    def gen_param_obj(self):
        """
        生成对应接口服务的参数对象
        """
        pass


class MetaServiceWorker(metaclass=abc.ABCMeta):
    """
    备份、恢复、资源接入功能实现基类，
    接收任务对象、参数对象和命令对象为入参，
    任务基本步骤功能实现，只关注任务业务功能实现
    基本功能函数通过
        调用update_action_result  返回action结果
        调用update_report_result  返回job_detail结果
    """

    def __init__(self, job_manager, param_obj, cmd_obj=None):
        self.job_manager = job_manager
        self.param = param_obj
        self.cmd = cmd_obj
        self.return_result = None
        self.interface_steps = {}

    def update_result(self):
        """
        更新功能函数执行结果，并调用job_manager.update更新任务结果状态
        :param : 功能函数执行结果结构体返回
        :return: None
        """
        self.job_manager.update(self.return_result)

    def update_action_result(self, code=0, body_err_code=0, msg="", err_param=None):
        """

        :param code: action 执行结果code
        :param body_err_code: 错误码
        :param msg: 执行作息
        :param err_param: 错误参数
        :return:
        """
        self.return_result = ActionResult(code=code, bodyErr=body_err_code, msg=msg, bodyErrParams=err_param)
        self.update_result()

    def update_report_result(self, status, progress, data_size: int = None, log_details: list = None):
        """

        :param status: 任务状态
        :param progress:任务进度
        :param data_size: 数据大小
        :param log_details:上报label结构体
        :return:
        """
        self.return_result = SubJobDetails(
            taskId=self.job_manager.job_id,
            subTaskId=self.job_manager.sub_job_id,
            progress=progress,
            taskStatus=status,
            dataSize=0
        )
        if data_size:
            self.return_result.data_size = data_size
        if log_details:
            self.return_result.log_detail = log_details
        self.update_result()


class MetaInterface(metaclass=abc.ABCMeta):
    """
    接口公共属性混合类
    need_report: 接口任务是否支持上报进度
    """
    need_report = False
    log_detail = {}


class CheckApplicationInterfaceMixin(MetaInterface):
    pass


class SupportResourceInterfaceMixin(MetaInterface):
    pass


class AllowInterfaceMixin(MetaInterface):
    pass


class PrerequisiteInterfaceMixin(MetaInterface):
    need_report = True
    log_detail = {
        "failed": ReportDBLabel.PRE_REQUISIT_FAILED,
        "success": "plugin_execute_prerequisit_task_success_label"
    }


class PostInterfaceMixin(MetaInterface):
    need_report = True
    log_detail = {
        "failed": ReportDBLabel.POST_TASK_FAIL
    }


class ExecuteInterfaceMixin(MetaInterface):
    need_report = True


class PauseInterfaceMixin(MetaInterface):
    pass


class StopInterfaceMixin(MetaInterface):
    pass


class GenSubInterfaceMixin(MetaInterface):
    need_report = False
