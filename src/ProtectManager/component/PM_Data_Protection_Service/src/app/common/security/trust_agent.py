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
from functools import wraps

from app.common.deploy_type import DeployType
from app.common.exception.common_error_codes import CommonErrorCodes
from app.common.exception.unified_exception import EmeiStorBizException
from app.common.rpc.system_base_rpc import get_host_trust


def check_host_trust(agent_ip):
    # 不可信情况抛出错误码
    if not get_host_trust(agent_ip):
        raise EmeiStorBizException(error=CommonErrorCodes.RESOURCE_NOT_TRUST)


def trust_agent(func):
    @wraps(func)
    def wrapper(*args, **kwargs):
        if not need_check_trust_deploy_type():
            return func(*args, **kwargs)
        agent_ip = kwargs.get("forward_ip", "")
        # ip存在的情况下才做
        if agent_ip:
            ip_list = agent_ip.split(",")
            # forward_ip 存在伪造添加的风险，故取倒数第二个，最后两个是nginx写入的不存在伪造的情况。
            if len(ip_list) >= 2:
                # 去除空格传入参数
                check_host_trust(ip_list[-2].strip())
        return func(*args, **kwargs)
    return wrapper


def need_check_trust_deploy_type():
    deploy_type = DeployType()
    if deploy_type.is_ocean_protect_type():
        return True
    return False
