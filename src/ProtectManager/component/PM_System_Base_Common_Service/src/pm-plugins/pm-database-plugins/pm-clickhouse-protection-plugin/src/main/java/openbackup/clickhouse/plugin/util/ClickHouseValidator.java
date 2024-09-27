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
package openbackup.clickhouse.plugin.util;

import openbackup.access.framework.resource.util.EnvironmentParamCheckUtil;
import openbackup.clickhouse.plugin.constant.ClickHouseConstant;
import openbackup.data.protection.access.provider.sdk.base.Authentication;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceConstants;
import openbackup.database.base.plugin.common.DatabaseConstants;

import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.constants.IsmConstant;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.ValidateUtil;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.common.validator.constants.RegexpConstants;

import lombok.extern.slf4j.Slf4j;

import org.apache.commons.collections.CollectionUtils;
import org.apache.commons.collections.MapUtils;
import org.apache.commons.lang3.StringUtils;

import java.util.List;
import java.util.Map;
import java.util.Objects;

/**
 * ClickHouse校验器，创建/更新时，校验参数
 *
 * @author q00464130
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-06-27
 */
@Slf4j
public class ClickHouseValidator {
    private ClickHouseValidator() {
    }

    /**
     * 创建/更新ClickHouse集群时，校验参数
     *
     * @param protectedEnvironment 集群环境
     */
    public static void checkClickHouseCluster(ProtectedEnvironment protectedEnvironment) {
        verifyEnvName(protectedEnvironment.getName());
        verifyClusterType(protectedEnvironment.getType());
        Map<String, String> extendInfo = protectedEnvironment.getExtendInfo();
        if (MapUtils.isEmpty(extendInfo)) {
            log.error("ClickHouse cluster extendInfo is null.");
            throw new LegoCheckedException(CommonErrorCode.ILLEGAL_PARAM, "ClickHouse cluster extendInfo is null.");
        }

        if (!StringUtils.equals(MapUtils.getString(extendInfo, ClickHouseConstant.TYPE),
            DatabaseConstants.CLUSTER_TARGET)) {
            log.error("ClickHouse cluster type: {} is not correct.",
                MapUtils.getString(extendInfo, ClickHouseConstant.TYPE));
            throw new LegoCheckedException(CommonErrorCode.ILLEGAL_PARAM, "ClickHouse cluster type is not correct.");
        }
        Map<String, List<ProtectedResource>> childrenMap = protectedEnvironment.getDependencies();
        if (MapUtils.isEmpty(childrenMap) || CollectionUtils.isEmpty(childrenMap.get(ResourceConstants.CHILDREN))) {
            log.error("ClickHouse cluster childrenMap: {} is empty.",
                MapUtils.getString(extendInfo, ClickHouseConstant.TYPE));
            throw new LegoCheckedException(CommonErrorCode.ILLEGAL_PARAM,
                "ClickHouse cluster childrenMap is is empty.");
        }
    }

    /**
     * 校验 ClickHouse集群节点参数信息
     *
     * @param resource 集群节点信息
     */
    public static void checkNode(ProtectedResource resource) {
        verifyName(resource.getName());
        verifyNodeType(resource.getType());

        // 认证方式0或2或5。认证方式为2时，密码非空；认证方式为5时，扩展信息 extendInfo中kerberosId相关信息非空；
        Authentication auth = resource.getAuth();
        if (Objects.isNull(auth)) {
            log.error("ClickHouse node Authentication is null.");
            throw new LegoCheckedException(CommonErrorCode.ILLEGAL_PARAM, "Authentication is null.");
        }
        verifyAuth(auth);

        // 校验environment的端口和ip
        ProtectedEnvironment environment = resource.getEnvironment();
        if (Objects.isNull(environment)) {
            log.error("Redis node Environment is null.");
            throw new LegoCheckedException(CommonErrorCode.ILLEGAL_PARAM, "Environment is null.");
        }
        verifyPort(environment.getPort());
        verifyIp(environment.getEndpoint());

        Map<String, String> extendInfo = resource.getExtendInfo();
        if (MapUtils.isEmpty(extendInfo)) {
            log.error("ClickHouse node extendInfo is null.");
            throw new LegoCheckedException(CommonErrorCode.ILLEGAL_PARAM, "ClickHouse node extendInfo is null.");
        }

        if (!StringUtils.equals(MapUtils.getString(extendInfo, ClickHouseConstant.TYPE),
            DatabaseConstants.NODE_TARGET)) {
            log.error("ClickHouse node type: {} is not correct.",
                MapUtils.getString(extendInfo, ClickHouseConstant.TYPE));
            throw new LegoCheckedException(CommonErrorCode.ILLEGAL_PARAM, "ClickHouse node type is not correct.");
        }

        // 校验extendInfo的端口、ip、客户端安装路径
        verifyPort(MapUtils.getInteger(extendInfo, ClickHouseConstant.PORT, 0));
        verifyIp(MapUtils.getString(extendInfo, ClickHouseConstant.IP));
        verifyPath(MapUtils.getString(extendInfo, ClickHouseConstant.CLIENT_PATH));

        // 校验主机
        Map<String, List<ProtectedResource>> agentsMap = resource.getDependencies();
        if (Objects.isNull(agentsMap) || CollectionUtils.isEmpty(agentsMap.get(DatabaseConstants.AGENTS))) {
            throw new LegoCheckedException(CommonErrorCode.ILLEGAL_PARAM, "Agents is null.");
        }
    }

    private static void verifyAuth(Authentication auth) {
        if (auth.getAuthType() == Authentication.APP_PASSWORD) {
            if (VerifyUtil.isEmpty(auth.getAuthPwd())) {
                log.error("ClickHouse node Password is null.");
                throw new LegoCheckedException(CommonErrorCode.ILLEGAL_PARAM, "Password is null.");
            }
        } else if (auth.getAuthType() == Authentication.KERBEROS) {
            if (VerifyUtil.isEmpty(
                MapUtils.getString(auth.getExtendInfo(), DatabaseConstants.EXTEND_INFO_KEY_KERBEROS_ID))) {
                log.error("ClickHouse node KerberosId is null.");
                throw new LegoCheckedException(CommonErrorCode.ILLEGAL_PARAM, "KerberosId is null.");
            }
        } else if (auth.getAuthType() == Authentication.NO_AUTH) {
            log.debug("ClickHouse node AuthType is 0");
        } else {
            log.error("ClickHouse node AuthType is not support.");
            throw new LegoCheckedException(CommonErrorCode.ILLEGAL_PARAM, "AuthType is not support.");
        }
    }

    /**
     * 路径是否符合要求
     *
     * @param path 备份策略中的路径
     */
    private static void verifyPath(String path) {
        if (VerifyUtil.isEmpty(path) || !ClickHouseConstant.LINUX_PATH_PATTERN.matcher(path).matches()) {
            log.error("ClickHouse path is invalid, destPath: {}.", path);
            throw new LegoCheckedException(CommonErrorCode.ILLEGAL_PARAM, "Path is invalid.");
        }
    }

    private static void verifyClusterType(String type) {
        if (VerifyUtil.isEmpty(type) || !StringUtils.equals(type, ClickHouseConstant.CLUSTER_TYPE)) {
            log.error("ClickHouse type: {} is not correct.", type);
            throw new LegoCheckedException(CommonErrorCode.ILLEGAL_PARAM, "Type is not correct.");
        }
    }

    private static void verifyNodeType(String type) {
        if (VerifyUtil.isEmpty(type) || !StringUtils.equals(type, ClickHouseConstant.NODE_TYPE)) {
            log.error("ClickHouse type: {} is not correct.", type);
            throw new LegoCheckedException(CommonErrorCode.ILLEGAL_PARAM, "Type is not correct.");
        }
    }

    private static void verifyEnvName(String name) {
        EnvironmentParamCheckUtil.checkEnvironmentNameEmpty(name);
        EnvironmentParamCheckUtil.checkEnvironmentNamePattern(name);
    }

    private static void verifyUserName(String userName) {
        // 由数字、字母、下划线（_）、中划线（-）或空格组成，且不能以中划线（-）开头，不区分大小写，长度为3～64位，
        // 不能与系统或操作系统中已有的用户名相同。
        if (VerifyUtil.isEmpty(userName) || userName.length() < ClickHouseConstant.NMAE_MIN_LENGTH
            || userName.length() > ClickHouseConstant.NMAE_MAX_LENGTH || !ValidateUtil.match(RegexpConstants.NAME_STR,
            userName)) {
            log.error("ClickHouse userName: {} is not correct.", userName);
            throw new LegoCheckedException(CommonErrorCode.ILLEGAL_PARAM, "The format of userName is not correct.");
        }
    }

    private static void verifyPwd(String authPwd) {
        if (VerifyUtil.isEmpty(authPwd) || authPwd.length() < ClickHouseConstant.PWD_MIN_LENGTH
            || authPwd.length() > ClickHouseConstant.NMAE_MAX_LENGTH || authPwd.length() != ValidateUtil.matchSize(
            authPwd)) {
            throw new LegoCheckedException(CommonErrorCode.ILLEGAL_PARAM, "The format of authPwd is not correct.");
        }
    }

    private static void verifyName(String name) {
        if (VerifyUtil.isEmpty(name) || name.length() < 1 || name.length() > ClickHouseConstant.NMAE_MAX_LENGTH
            || !ValidateUtil.match(RegexpConstants.NAME_STR, name)) {
            log.error("ClickHouse name: {} is not correct.", name);
            throw new LegoCheckedException(CommonErrorCode.ILLEGAL_PARAM, "The format of name is not correct.");
        }
    }

    private static void verifyPort(Integer port) {
        if (VerifyUtil.isEmpty(port) || port < IsmConstant.PORT_MIN || port > IsmConstant.PORT_MAX) {
            log.error("ClickHouse port: {} is not correct.", port);
            throw new LegoCheckedException(CommonErrorCode.IP_PORT_ERROR, "Port can not be empty and out of range.");
        }
    }

    private static void verifyIp(String ip) {
        if (VerifyUtil.isEmpty(ip) || !ip.matches(RegexpConstants.IPADDRESS_V4)) {
            log.error("ClickHouse ip: {} is not correct.", ip);
            throw new LegoCheckedException(CommonErrorCode.ILLEGAL_PARAM, "Ip is illegal.");
        }
    }
}