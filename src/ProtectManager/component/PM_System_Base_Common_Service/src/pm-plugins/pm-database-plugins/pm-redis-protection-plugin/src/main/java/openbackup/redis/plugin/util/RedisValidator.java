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
package openbackup.redis.plugin.util;

import openbackup.access.framework.resource.util.EnvironmentParamCheckUtil;
import openbackup.data.protection.access.provider.sdk.base.Authentication;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceConstants;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.redis.plugin.constant.RedisConstant;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.constants.IsmConstant;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.ValidateUtil;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.common.validator.constants.RegexpConstants;
import openbackup.system.base.sdk.resource.model.ResourceTypeEnum;

import lombok.extern.slf4j.Slf4j;

import org.apache.commons.collections.CollectionUtils;
import org.apache.commons.collections.MapUtils;
import org.apache.commons.lang3.StringUtils;

import java.util.List;
import java.util.Map;
import java.util.Objects;

/**
 * 功能描述: 校验 Redis各种参数信息
 *
 */
@Slf4j
public class RedisValidator {
    private RedisValidator() {
    }

    /**
     * 校验 Redis集群参数信息
     *
     * @param resource 集群节点信息
     */
    public static void checkCluster(ProtectedResource resource) {
        verifyEnvName(resource.getName());
        verifyType(resource.getType());
        Map<String, String> extendInfo = resource.getExtendInfo();
        if (MapUtils.isEmpty(extendInfo)) {
            log.error("Redis cluster extendInfo is null.");
            throw new LegoCheckedException(CommonErrorCode.ILLEGAL_PARAM, "Redis cluster extendInfo is null.");
        }

        if (!StringUtils.equals(MapUtils.getString(extendInfo, RedisConstant.TYPE), DatabaseConstants.CLUSTER_TARGET)) {
            log.error("Redis cluster type: {} is not correct.", MapUtils.getString(extendInfo, RedisConstant.TYPE));
            throw new LegoCheckedException(CommonErrorCode.ILLEGAL_PARAM, "Redis cluster type is not correct.");
        }
        Map<String, List<ProtectedResource>> childrenMap = resource.getDependencies();
        if (MapUtils.isEmpty(childrenMap) || CollectionUtils.isEmpty(childrenMap.get(ResourceConstants.CHILDREN))) {
            log.error("Redis cluster childrenMap: {} is empty.", MapUtils.getString(extendInfo, RedisConstant.TYPE));
            throw new LegoCheckedException(CommonErrorCode.ILLEGAL_PARAM, "Redis cluster childrenMap is is empty.");
        }
    }

    /**
     * 校验 Redis集群节点参数信息
     *
     * @param resource 集群节点信息
     */
    public static void checkNode(ProtectedResource resource) {
        verifyName(resource.getName());
        verifyType(resource.getType());

        // 认证方式0或2或5。认证方式为2时，密码非空；认证方式为5时，扩展信息 extendInfo中kerberosId相关信息非空；
        Authentication auth = resource.getAuth();
        if (Objects.isNull(auth)) {
            log.error("Redis node Authentication is null.");
            throw new LegoCheckedException(CommonErrorCode.ILLEGAL_PARAM, "Authentication is null.");
        }
        if (auth.getAuthType() == Authentication.APP_PASSWORD) {
            if (VerifyUtil.isEmpty(auth.getAuthPwd())) {
                log.error("Redis node Password is null.");
                throw new LegoCheckedException(CommonErrorCode.ILLEGAL_PARAM, "Password is null.");
            }
        } else if (auth.getAuthType() == Authentication.KERBEROS) {
            if (VerifyUtil.isEmpty(
                MapUtils.getString(auth.getExtendInfo(), DatabaseConstants.EXTEND_INFO_KEY_KERBEROS_ID))) {
                log.error("Redis node KerberosId is null.");
                throw new LegoCheckedException(CommonErrorCode.ILLEGAL_PARAM, "KerberosId is null.");
            }
        } else if (auth.getAuthType() == Authentication.NO_AUTH) {
            log.debug("Redis node AuthType is 0");
        } else {
            log.error("Redis node AuthType is not support.");
            throw new LegoCheckedException(CommonErrorCode.ILLEGAL_PARAM, "AuthType is not support.");
        }

        Map<String, String> extendInfo = resource.getExtendInfo();
        if (MapUtils.isEmpty(extendInfo)) {
            log.error("Redis node extendInfo is null.");
            throw new LegoCheckedException(CommonErrorCode.ILLEGAL_PARAM, "Redis node extendInfo is null.");
        }

        if (!StringUtils.equals(MapUtils.getString(extendInfo, RedisConstant.TYPE), DatabaseConstants.NODE_TARGET)) {
            log.error("Redis node type: {} is not correct.", MapUtils.getString(extendInfo, RedisConstant.TYPE));
            throw new LegoCheckedException(CommonErrorCode.ILLEGAL_PARAM, "Redis node type is not correct.");
        }

        // 校验extendInfo的端口、ip、客户端安装路径
        verifyPort(MapUtils.getInteger(extendInfo, RedisConstant.PORT, 0));
        verifyIp(MapUtils.getString(extendInfo, RedisConstant.IP));
        verifyPath(MapUtils.getString(extendInfo, RedisConstant.CLIENT_PATH));

        // 校验主机
        Map<String, List<ProtectedResource>> agentsMap = resource.getDependencies();
        if (Objects.isNull(agentsMap) || CollectionUtils.isEmpty(agentsMap.get(DatabaseConstants.AGENTS))) {
            throw new LegoCheckedException(CommonErrorCode.ILLEGAL_PARAM, "Agents is null.");
        }
    }

    /**
     * 路径是否符合要求
     *
     * @param path 备份策略中的路径
     */
    private static void verifyPath(String path) {
        if (VerifyUtil.isEmpty(path) || !RedisConstant.LINUX_PATH_PATTERN.matcher(path).matches()) {
            log.error("Redis path:{} is invalid, destPath: {}.", path);
            throw new LegoCheckedException(CommonErrorCode.ILLEGAL_PARAM, "Path is invalid.");
        }
    }

    private static void verifyType(String type) {
        if (VerifyUtil.isEmpty(type) || !StringUtils.equals(type, ResourceTypeEnum.DATABASE.getType())) {
            log.error("Redis type: {} is not correct.", type);
            throw new LegoCheckedException(CommonErrorCode.ILLEGAL_PARAM, "Type is not correct.");
        }
    }

    private static void verifyEnvName(String name) {
        EnvironmentParamCheckUtil.checkEnvironmentNameEmpty(name);
        EnvironmentParamCheckUtil.checkEnvironmentNamePattern(name);
    }

    private static void verifyName(String name) {
        if (VerifyUtil.isEmpty(name) || (name.length() < 1 || name.length() > RedisConstant.NMAE_MAX_LENGTH)
            || !ValidateUtil.match(RegexpConstants.NAME_STR, name)) {
            log.error("Redis name: {} is not correct.", name);
            throw new LegoCheckedException(CommonErrorCode.ILLEGAL_PARAM, "The format of name is not correct.");
        }
    }

    private static void verifyPort(Integer port) {
        if (VerifyUtil.isEmpty(port) || !(port >= IsmConstant.PORT_MIN && port <= IsmConstant.PORT_MAX)) {
            log.error("Redis port: {} is not correct.", port);
            throw new LegoCheckedException(CommonErrorCode.IP_PORT_ERROR, "Port can not be empty and out of range.");
        }
    }

    private static void verifyIp(String ip) {
        if (VerifyUtil.isEmpty(ip) || !ip.matches(RegexpConstants.IP_V4V6_ADDRESS)) {
            log.error("Redis ip: {} is not correct.", ip);
            throw new LegoCheckedException(CommonErrorCode.ILLEGAL_PARAM, "Ip is illegal.");
        }
    }
}