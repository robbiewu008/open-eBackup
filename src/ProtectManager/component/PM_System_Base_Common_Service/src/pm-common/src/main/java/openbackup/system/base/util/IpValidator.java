/*
* This file is a part of the open-eBackup project.
* This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
* If a copy of the MPL was not distributed with this file, You can obtain one at
* http://mozilla.org/MPL/2.0/.
*
* Copyright (c) [2024] Huawei Technologies Co.,Ltd.
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*/
package openbackup.system.base.util;

import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.validator.constants.RegexpConstants;
import openbackup.system.base.sdk.infrastructure.InfrastructureRestApi;
import openbackup.system.base.sdk.infrastructure.model.InfraResponseWithError;
import openbackup.system.base.sdk.infrastructure.model.beans.NodeControllerInfo;

import lombok.extern.slf4j.Slf4j;

import org.springframework.stereotype.Component;

/**
 * 校验IP合法性工具类。为防止SSRF攻击，判断输入ip是否为A8000内部节点ip，如果是则抛出非法参数。
 *
 */
@Slf4j
@Component
public class IpValidator {
    private final InfrastructureRestApi infrastructureRestApi;

    IpValidator(InfrastructureRestApi infrastructureRestApi) {
        this.infrastructureRestApi = infrastructureRestApi;
    }

    /**
     * 校验输入ip是否为A8000内部ip。
     *
     * @param ip 待校验ip。输入ip的正确性需要由调用者保障。
     * @return true/false 如果ip为A8000内部ip，返回true。
     */
    public boolean isIpInA8000(String ip) {
        InfraResponseWithError<NodeControllerInfo> infraResponseWithError =
            infrastructureRestApi.getNodeInfoByPodIp(ip);
        if (infraResponseWithError != null && infraResponseWithError.getError() == null) {
            log.debug("The ip is in A8000");
            return true;
        }
        return false;
    }

    /**
     * 校验ip合法性（1.ip非本机ip; 2. ip符合校验规则），否则抛出参数异常
     *
     * @param ip 待校验ip地址
     * @throws LegoCheckedException 假设IP非法，抛出参数非法异常
     */
    public void checkIp(String ip) throws LegoCheckedException {
        if (!ip.matches(RegexpConstants.IP_V4V6_ADDRESS) || isIpInA8000(ip)) {
            throw new LegoCheckedException(CommonErrorCode.ILLEGAL_PARAM);
        }
    }
}
