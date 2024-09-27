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

import openbackup.clickhouse.plugin.common.ExceptionMatcher;
import openbackup.clickhouse.plugin.constant.ClickHouseConstant;
import openbackup.data.protection.access.provider.sdk.base.Authentication;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceConstants;
import openbackup.database.base.plugin.common.DatabaseConstants;

import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import com.google.common.collect.Lists;

import org.junit.Rule;
import org.junit.Test;
import org.junit.rules.ExpectedException;
import org.junit.runner.RunWith;
import org.powermock.modules.junit4.PowerMockRunner;

import java.util.HashMap;
import java.util.List;

/**
 * ClickHouse Validator Test
 *
 * @author w00439064
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-06-27
 */
@RunWith(PowerMockRunner.class)
public class ClickHouseValidatorTest {
    /**
     * 预期异常
     */
    @Rule
    public ExpectedException expectedException = ExpectedException.none();

    /**
     * 用例场景：创建集群，name为空
     * 前置条件：服务正常、集群节点正常
     * 检查点: 报错，错误码：ILLEGAL_PARAM
     */
    @Test
    public void should_throw_LegoCheckedException_if_cluster_param_without_name() {
        expectedException.expect(LegoCheckedException.class);
        expectedException.expect(new ExceptionMatcher(CommonErrorCode.ILLEGAL_PARAM, "environment name is empty"));
        ClickHouseValidator.checkClickHouseCluster(new ProtectedEnvironment());
    }

    /**
     * 用例场景：创建集群，type为空
     * 前置条件：服务正常、集群节点正常
     * 检查点: 报错，错误码：ILLEGAL_PARAM
     */
    @Test
    public void should_throw_LegoCheckedException_if_cluster_param_without_type() {
        expectedException.expect(LegoCheckedException.class);
        expectedException.expect(new ExceptionMatcher(CommonErrorCode.ILLEGAL_PARAM, "Type is not correct."));
        ProtectedEnvironment environment = new ProtectedEnvironment();
        environment.setName("T192168");
        ClickHouseValidator.checkClickHouseCluster(environment);
    }

    /**
     * 用例场景：创建集群，extendInfo为空
     * 前置条件：服务正常、集群节点正常，参数信息正确
     * 检查点: 报错，错误码：ILLEGAL_PARAM
     */
    @Test
    public void should_throw_LegoCheckedException_if_cluster_param_without_extendInfo() {
        expectedException.expect(LegoCheckedException.class);
        expectedException.expect(
            new ExceptionMatcher(CommonErrorCode.ILLEGAL_PARAM, "ClickHouse cluster extendInfo is null."));
        ProtectedEnvironment environment = new ProtectedEnvironment();
        environment.setName("T192168");
        environment.setType(ClickHouseConstant.CLUSTER_TYPE);
        environment.setSubType(ResourceSubTypeEnum.CLICK_HOUSE.getType());
        ClickHouseValidator.checkClickHouseCluster(environment);
    }

    /**
     * 用例场景：创建集群，extendInfo中的type为空
     * 前置条件：服务正常、集群节点正常，参数信息正确
     * 检查点: 报错，错误码：ILLEGAL_PARAM
     */
    @Test
    public void should_throw_LegoCheckedException_if_cluster_param_without_type_of_extendInfo() {
        expectedException.expect(LegoCheckedException.class);
        expectedException.expect(
            new ExceptionMatcher(CommonErrorCode.ILLEGAL_PARAM, "ClickHouse cluster type is not correct."));
        ProtectedEnvironment environment = new ProtectedEnvironment();
        environment.setName("T192168");
        environment.setType(ClickHouseConstant.CLUSTER_TYPE);
        environment.setSubType(ResourceSubTypeEnum.CLICK_HOUSE.getType());
        environment.setExtendInfo(new HashMap<String, String>() {
            {
                put("kerberosId", "15cf44359b63400cbbca9dfe167ae01b");
            }
        });
        ClickHouseValidator.checkClickHouseCluster(environment);
    }

    /**
     * 用例场景：创建集群，children为空
     * 前置条件：服务正常、集群节点正常，参数信息正确
     * 检查点: 报错，错误码：ILLEGAL_PARAM
     */
    @Test
    public void should_throw_LegoCheckedException_if_cluster_param_without_children() {
        expectedException.expect(LegoCheckedException.class);
        expectedException.expect(
            new ExceptionMatcher(CommonErrorCode.ILLEGAL_PARAM, "ClickHouse cluster childrenMap is is empty."));
        ProtectedEnvironment environment = new ProtectedEnvironment();
        environment.setName("T192168");
        environment.setType(ClickHouseConstant.CLUSTER_TYPE);
        environment.setSubType(ResourceSubTypeEnum.CLICK_HOUSE.getType());
        environment.setExtendInfo(new HashMap<String, String>() {
            {
                put(ClickHouseConstant.TYPE, DatabaseConstants.CLUSTER_TARGET);
            }
        });
        ClickHouseValidator.checkClickHouseCluster(environment);
    }

    /**
     * 用例场景：创建集群，进行参数检验
     * 前置条件：服务正常、集群节点正常，参数信息正确
     * 检查点: 不报错
     */
    @Test
    public void check_cluster_param_success() {
        ProtectedEnvironment environment = new ProtectedEnvironment();
        environment.setName("T192168");
        environment.setType(ClickHouseConstant.CLUSTER_TYPE);
        environment.setSubType(ResourceSubTypeEnum.CLICK_HOUSE.getType());
        environment.setExtendInfo(new HashMap<String, String>() {
            {
                put(ClickHouseConstant.TYPE, DatabaseConstants.CLUSTER_TARGET);
            }
        });
        environment.setDependencies(new HashMap<String, List<ProtectedResource>>() {
            {
                put(ResourceConstants.CHILDREN, Lists.newArrayList(new ProtectedResource()));
            }
        });
        ClickHouseValidator.checkClickHouseCluster(environment);
    }

    /**
     * 用例场景：创建集群，集群节点的验证信息为空
     * 前置条件：服务正常、集群节点正常
     * 检查点: 报错，错误码：ILLEGAL_PARAM
     */
    @Test
    public void should_throw_LegoCheckedException_if_node_param_without_auth() {
        expectedException.expect(LegoCheckedException.class);
        expectedException.expect(new ExceptionMatcher(CommonErrorCode.ILLEGAL_PARAM, "Authentication is null."));
        ProtectedResource resource = new ProtectedResource();
        resource.setName("192168");
        resource.setType(ClickHouseConstant.NODE_TYPE);
        resource.setSubType(ResourceSubTypeEnum.CLICK_HOUSE.getType());
        ClickHouseValidator.checkNode(resource);
    }

    /**
     * 用例场景：创建集群，集群节点的验证信息的AuthType错误
     * 前置条件：服务正常、集群节点正常
     * 检查点: 报错，错误码：ILLEGAL_PARAM
     */
    @Test
    public void should_throw_LegoCheckedException_if_node_param_with_wrong_authType() {
        expectedException.expect(LegoCheckedException.class);
        expectedException.expect(new ExceptionMatcher(CommonErrorCode.ILLEGAL_PARAM, "AuthType is not support."));
        ProtectedResource resource = new ProtectedResource();
        resource.setName("192168");
        resource.setType(ClickHouseConstant.NODE_TYPE);
        resource.setSubType(ResourceSubTypeEnum.CLICK_HOUSE.getType());
        Authentication auth = new Authentication();
        auth.setAuthType(Authentication.AKSK);
        resource.setAuth(auth);
        ClickHouseValidator.checkNode(resource);
    }

    /**
     * 用例场景：创建集群，集群节点的验证类型为应用密码认证时，密码为空
     * 前置条件：服务正常、集群节点正常
     * 检查点: 报错，错误码：ILLEGAL_PARAM
     */
    @Test
    public void should_throw_LegoCheckedException_if_node_param_without_password() {
        expectedException.expect(LegoCheckedException.class);
        expectedException.expect(new ExceptionMatcher(CommonErrorCode.ILLEGAL_PARAM, "Password is null."));
        ProtectedResource resource = new ProtectedResource();
        resource.setName("192168");
        resource.setType(ClickHouseConstant.NODE_TYPE);
        resource.setSubType(ResourceSubTypeEnum.CLICK_HOUSE.getType());
        Authentication auth = new Authentication();
        auth.setAuthType(Authentication.APP_PASSWORD);
        resource.setAuth(auth);
        ClickHouseValidator.checkNode(resource);
    }

    /**
     * 用例场景：创建集群，集群节点的验证类型为KERBEROS认证时，KerberosId为空
     * 前置条件：服务正常、集群节点正常
     * 检查点: 报错，错误码：ILLEGAL_PARAM
     */
    @Test
    public void should_throw_LegoCheckedException_if_node_param_without_kerberosId() {
        expectedException.expect(LegoCheckedException.class);
        expectedException.expect(new ExceptionMatcher(CommonErrorCode.ILLEGAL_PARAM, "KerberosId is null."));
        ProtectedResource resource = new ProtectedResource();
        resource.setName("192168");
        resource.setType(ClickHouseConstant.NODE_TYPE);
        resource.setSubType(ResourceSubTypeEnum.CLICK_HOUSE.getType());
        Authentication auth = new Authentication();
        auth.setAuthType(Authentication.KERBEROS);
        resource.setAuth(auth);
        ClickHouseValidator.checkNode(resource);
    }

    /**
     * 用例场景：创建集群，集群节点的extendInfo为空
     * 前置条件：服务正常、集群节点正常
     * 检查点: 报错，错误码：ILLEGAL_PARAM
     */
    @Test
    public void should_throw_LegoCheckedException_if_node_param_without_extendInfo() {
        expectedException.expect(LegoCheckedException.class);
        expectedException.expect(
            new ExceptionMatcher(CommonErrorCode.ILLEGAL_PARAM, "ClickHouse node extendInfo is null."));
        ProtectedResource resource = new ProtectedResource();
        resource.setName("192168");
        resource.setType(ClickHouseConstant.NODE_TYPE);
        resource.setSubType(ResourceSubTypeEnum.CLICK_HOUSE.getType());
        Authentication auth = new Authentication();
        resource.setAuth(auth);
        ProtectedEnvironment protectedEnvironment = new ProtectedEnvironment();
        protectedEnvironment.setPort(11);
        protectedEnvironment.setEndpoint("192.167.1.1");
        resource.setEnvironment(protectedEnvironment);
        ClickHouseValidator.checkNode(resource);
    }

    /**
     * 用例场景：创建集群，集群节点的extendInfo的type不正确
     * 前置条件：服务正常、集群节点正常
     * 检查点: 报错，错误码：ILLEGAL_PARAM
     */
    @Test
    public void should_throw_LegoCheckedException_if_node_param_with_wrong_type_of_extendInfo() {
        expectedException.expect(LegoCheckedException.class);
        expectedException.expect(
            new ExceptionMatcher(CommonErrorCode.ILLEGAL_PARAM, "ClickHouse node type is not correct."));
        ProtectedResource resource17201 = new ProtectedResource();
        resource17201.setName("192168");
        resource17201.setType(ClickHouseConstant.NODE_TYPE);
        resource17201.setSubType(ResourceSubTypeEnum.CLICK_HOUSE.getType());
        Authentication auth = new Authentication();
        resource17201.setAuth(auth);
        ProtectedEnvironment protectedEnvironment = new ProtectedEnvironment();
        protectedEnvironment.setPort(11);
        protectedEnvironment.setEndpoint("192.167.1.1");
        resource17201.setEnvironment(protectedEnvironment);
        resource17201.setExtendInfo(new HashMap<String, String>() {
            {
                put(ClickHouseConstant.TYPE, DatabaseConstants.CLUSTER_TARGET);
            }
        });
        ClickHouseValidator.checkNode(resource17201);
    }

    /**
     * 用例场景：创建集群，集群节点的extendInfo的ip为空
     * 前置条件：服务正常、集群节点正常
     * 检查点: 报错，错误码：ILLEGAL_PARAM
     */
    @Test
    public void should_throw_LegoCheckedException_if_node_param_without_ip_of_extendInfo() {
        expectedException.expect(LegoCheckedException.class);
        expectedException.expect(new ExceptionMatcher(CommonErrorCode.ILLEGAL_PARAM, "Ip is illegal."));
        ProtectedResource resource17202 = new ProtectedResource();
        resource17202.setName("192168");
        resource17202.setType(ClickHouseConstant.NODE_TYPE);
        resource17202.setSubType(ResourceSubTypeEnum.CLICK_HOUSE.getType());
        Authentication auth = new Authentication();
        resource17202.setAuth(auth);
        ProtectedEnvironment protectedEnvironment = new ProtectedEnvironment();
        protectedEnvironment.setPort(11);
        protectedEnvironment.setEndpoint("192.167.1.1");
        resource17202.setEnvironment(protectedEnvironment);
        resource17202.setExtendInfo(new HashMap<String, String>() {
            {
                put(ClickHouseConstant.TYPE, DatabaseConstants.NODE_TARGET);
                put(ClickHouseConstant.PORT, "127");
            }
        });
        ClickHouseValidator.checkNode(resource17202);
    }

    /**
     * 用例场景：创建集群，集群节点的extendInfo的端口不正确
     * 前置条件：服务正常、集群节点正常
     * 检查点: 报错，错误码：PORT_ERROR
     */
    @Test
    public void should_throw_LegoCheckedException_if_node_param_with_wrong_port_of_extendInfo() {
        expectedException.expect(LegoCheckedException.class);
        expectedException.expect(
            new ExceptionMatcher(CommonErrorCode.IP_PORT_ERROR, "Port can not be empty and out of range."));
        ProtectedResource resource = new ProtectedResource();
        resource.setName("192168");
        resource.setType(ClickHouseConstant.NODE_TYPE);
        resource.setSubType(ResourceSubTypeEnum.CLICK_HOUSE.getType());
        Authentication auth = new Authentication();
        resource.setAuth(auth);
        ProtectedEnvironment protectedEnvironment = new ProtectedEnvironment();
        protectedEnvironment.setPort(65536);
        protectedEnvironment.setEndpoint("192.167.1.1");
        resource.setEnvironment(protectedEnvironment);
        resource.setExtendInfo(new HashMap<String, String>() {
            {
                put(ClickHouseConstant.TYPE, DatabaseConstants.NODE_TARGET);
                put(ClickHouseConstant.PORT, "65536");
                put(ClickHouseConstant.IP, "192.164.1.4");
            }
        });
        ClickHouseValidator.checkNode(resource);
    }

    /**
     * 用例场景：创建集群，集群节点的extendInfo的Path为空
     * 前置条件：服务正常、集群节点正常
     * 检查点: 报错，错误码：ILLEGAL_PARAM
     */
    @Test
    public void should_throw_LegoCheckedException_if_node_param_without_path_of_extendInfo() {
        expectedException.expect(LegoCheckedException.class);
        expectedException.expect(new ExceptionMatcher(CommonErrorCode.ILLEGAL_PARAM, "Path is invalid."));
        ProtectedResource resource17203 = new ProtectedResource();
        resource17203.setName("192168");
        resource17203.setType(ClickHouseConstant.NODE_TYPE);
        resource17203.setSubType(ResourceSubTypeEnum.CLICK_HOUSE.getType());
        Authentication auth = new Authentication();
        resource17203.setAuth(auth);
        ProtectedEnvironment protectedEnvironment = new ProtectedEnvironment();
        protectedEnvironment.setPort(11);
        protectedEnvironment.setEndpoint("192.167.1.1");
        resource17203.setEnvironment(protectedEnvironment);
        resource17203.setExtendInfo(new HashMap<String, String>() {
            {
                put(ClickHouseConstant.TYPE, DatabaseConstants.NODE_TARGET);
                put(ClickHouseConstant.PORT, "127");
                put(ClickHouseConstant.IP, "192.164.1.4");
            }
        });
        ClickHouseValidator.checkNode(resource17203);
    }

    /**
     * 用例场景：创建集群，集群节点的extendInfo的agents为空
     * 前置条件：服务正常、集群节点正常
     * 检查点: 报错，错误码：ILLEGAL_PARAM
     */
    @Test
    public void should_throw_LegoCheckedException_if_node_param_without_agents() {
        expectedException.expect(LegoCheckedException.class);
        expectedException.expect(new ExceptionMatcher(CommonErrorCode.ILLEGAL_PARAM, "Agents is null."));
        ProtectedResource resource17204 = new ProtectedResource();
        resource17204.setName("192168");
        resource17204.setType(ClickHouseConstant.NODE_TYPE);
        resource17204.setSubType(ResourceSubTypeEnum.CLICK_HOUSE.getType());
        Authentication auth = new Authentication();
        resource17204.setAuth(auth);
        ProtectedEnvironment protectedEnvironment = new ProtectedEnvironment();
        protectedEnvironment.setPort(11);
        protectedEnvironment.setEndpoint("192.167.1.1");
        resource17204.setEnvironment(protectedEnvironment);
        resource17204.setExtendInfo(new HashMap<String, String>() {
            {
                put(ClickHouseConstant.TYPE, DatabaseConstants.NODE_TARGET);
                put(ClickHouseConstant.PORT, "127");
                put(ClickHouseConstant.IP, "192.164.1.4");
                put(ClickHouseConstant.CLIENT_PATH, "/srv/BigData/test");
            }
        });
        ClickHouseValidator.checkNode(resource17204);
    }

    /**
     * 用例场景：创建集群，进行集群节点检验
     * 前置条件：服务正常、集群节点正常，参数信息正确
     * 检查点: 不报错
     */
    @Test
    public void check_node_param_success() {
        ProtectedResource resource17205 = new ProtectedResource();
        resource17205.setName("192168");
        resource17205.setType(ClickHouseConstant.NODE_TYPE);
        resource17205.setSubType(ResourceSubTypeEnum.CLICK_HOUSE.getType());
        Authentication auth = new Authentication();
        resource17205.setAuth(auth);
        ProtectedEnvironment protectedEnvironment = new ProtectedEnvironment();
        protectedEnvironment.setPort(11);
        protectedEnvironment.setEndpoint("192.167.1.1");
        resource17205.setEnvironment(protectedEnvironment);
        resource17205.setExtendInfo(new HashMap<String, String>() {
            {
                put(ClickHouseConstant.TYPE, DatabaseConstants.NODE_TARGET);
                put(ClickHouseConstant.PORT, "127");
                put(ClickHouseConstant.IP, "192.164.1.4");
                put(ClickHouseConstant.CLIENT_PATH, "/srv/BigData/test");
            }
        });
        resource17205.setDependencies(new HashMap<String, List<ProtectedResource>>() {
            {
                put(DatabaseConstants.AGENTS, Lists.newArrayList(new ProtectedResource()));
            }
        });
        ClickHouseValidator.checkNode(resource17205);
    }
}