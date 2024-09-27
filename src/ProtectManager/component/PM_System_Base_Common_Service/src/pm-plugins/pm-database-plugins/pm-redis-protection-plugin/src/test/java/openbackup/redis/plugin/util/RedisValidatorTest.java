package openbackup.redis.plugin.util;

import openbackup.data.protection.access.provider.sdk.base.Authentication;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceConstants;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.redis.plugin.common.ExceptionMatcher;
import openbackup.redis.plugin.constant.RedisConstant;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.sdk.resource.model.ResourceTypeEnum;

import com.google.common.collect.Lists;

import org.junit.Rule;
import org.junit.Test;
import org.junit.rules.ExpectedException;
import org.junit.runner.RunWith;
import org.powermock.modules.junit4.PowerMockRunner;

import java.util.HashMap;
import java.util.List;

/**
 * Redis Validator Test
 *
 * @author x30028756
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-06-14
 */
@RunWith(PowerMockRunner.class)
public class RedisValidatorTest {
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
    public void should_throw_LegoCheckedException_if_cluster_param_without_name_when_register_cluster() {
        expectedException.expect(LegoCheckedException.class);
        expectedException.expect(new ExceptionMatcher(CommonErrorCode.ILLEGAL_PARAM, "environment name is empty"));
        RedisValidator.checkCluster(new ProtectedResource());
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
        ProtectedResource resource = new ProtectedResource();
        resource.setName("T192168");
        RedisValidator.checkCluster(resource);
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
            new ExceptionMatcher(CommonErrorCode.ILLEGAL_PARAM, "Redis cluster extendInfo is null."));
        ProtectedResource resource = new ProtectedResource();
        resource.setName("T192168");
        resource.setType(ResourceTypeEnum.DATABASE.getType());
        resource.setSubType(ResourceSubTypeEnum.REDIS.getType());
        RedisValidator.checkCluster(resource);
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
            new ExceptionMatcher(CommonErrorCode.ILLEGAL_PARAM, "Redis cluster type is not correct."));
        ProtectedResource resource = new ProtectedResource();
        resource.setName("T192168");
        resource.setType(ResourceTypeEnum.DATABASE.getType());
        resource.setSubType(ResourceSubTypeEnum.REDIS.getType());
        resource.setExtendInfo(new HashMap<String, String>() {
            {
                put("kerberosId", "15cf44359b63400cbbca9dfe167ae01b");
            }
        });
        RedisValidator.checkCluster(resource);
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
            new ExceptionMatcher(CommonErrorCode.ILLEGAL_PARAM, "Redis cluster childrenMap is is empty."));
        ProtectedResource resource = new ProtectedResource();
        resource.setName("T192168");
        resource.setType(ResourceTypeEnum.DATABASE.getType());
        resource.setSubType(ResourceSubTypeEnum.REDIS.getType());
        resource.setExtendInfo(new HashMap<String, String>() {
            {
                put(RedisConstant.TYPE, DatabaseConstants.CLUSTER_TARGET);
            }
        });
        RedisValidator.checkCluster(resource);
    }

    /**
     * 用例场景：创建集群，进行参数检验
     * 前置条件：服务正常、集群节点正常，参数信息正确
     * 检查点: 不报错
     */
    @Test
    public void check_cluster_param_success() {
        ProtectedResource resource = new ProtectedResource();
        resource.setName("T192168");
        resource.setType(ResourceTypeEnum.DATABASE.getType());
        resource.setSubType(ResourceSubTypeEnum.REDIS.getType());
        resource.setExtendInfo(new HashMap<String, String>() {
            {
                put(RedisConstant.TYPE, DatabaseConstants.CLUSTER_TARGET);
            }
        });
        resource.setDependencies(new HashMap<String, List<ProtectedResource>>() {
            {
                put(ResourceConstants.CHILDREN, Lists.newArrayList(new ProtectedResource()));
            }
        });
        RedisValidator.checkCluster(resource);
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
        resource.setType(ResourceTypeEnum.DATABASE.getType());
        resource.setSubType(ResourceSubTypeEnum.REDIS.getType());
        RedisValidator.checkNode(resource);
    }

    /**
     * 用例场景：创建集群，集群节点的验证信息的AuthType错误
     * 前置条件：服务正常、集群节点正常
     * 检查点: 报错，错误码：ILLEGAL_PARAM
     */
    @Test
    public void should_throw_LegoCheckedException_if_node_param_with_wrong_authType_when_register_cluster() {
        expectedException.expect(LegoCheckedException.class);
        expectedException.expect(new ExceptionMatcher(CommonErrorCode.ILLEGAL_PARAM, "AuthType is not support."));
        ProtectedResource resource = new ProtectedResource();
        resource.setName("192168");
        resource.setType(ResourceTypeEnum.DATABASE.getType());
        resource.setSubType(ResourceSubTypeEnum.REDIS.getType());
        Authentication auth = new Authentication();
        auth.setAuthType(Authentication.AKSK);
        resource.setAuth(auth);
        RedisValidator.checkNode(resource);
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
        resource.setType(ResourceTypeEnum.DATABASE.getType());
        resource.setSubType(ResourceSubTypeEnum.REDIS.getType());
        Authentication auth = new Authentication();
        auth.setAuthType(Authentication.APP_PASSWORD);
        resource.setAuth(auth);
        RedisValidator.checkNode(resource);
    }

    /**
     * 用例场景：创建集群，集群节点的验证类型为KERBEROS认证时，KerberosId为空
     * 前置条件：服务正常、集群节点正常
     * 检查点: 报错，错误码：ILLEGAL_PARAM
     */
    @Test
    public void should_throw_LegoCheckedException_if_node_param_without_kerberosId_when_register_cluster() {
        expectedException.expect(LegoCheckedException.class);
        expectedException.expect(new ExceptionMatcher(CommonErrorCode.ILLEGAL_PARAM, "KerberosId is null."));
        ProtectedResource resource = new ProtectedResource();
        resource.setName("192168");
        resource.setType(ResourceTypeEnum.DATABASE.getType());
        resource.setSubType(ResourceSubTypeEnum.REDIS.getType());
        Authentication auth = new Authentication();
        auth.setAuthType(Authentication.KERBEROS);
        resource.setAuth(auth);
        RedisValidator.checkNode(resource);
    }

    /**
     * 用例场景：创建集群，集群节点的extendInfo为空
     * 前置条件：服务正常、集群节点正常
     * 检查点: 报错，错误码：ILLEGAL_PARAM
     */
    @Test
    public void should_throw_LegoCheckedException_if_node_param_without_extendInfo() {
        expectedException.expect(LegoCheckedException.class);
        expectedException.expect(new ExceptionMatcher(CommonErrorCode.ILLEGAL_PARAM, "Redis node extendInfo is null."));
        ProtectedResource resource = new ProtectedResource();
        resource.setName("192168");
        resource.setType(ResourceTypeEnum.DATABASE.getType());
        resource.setSubType(ResourceSubTypeEnum.REDIS.getType());
        Authentication auth = new Authentication();
        resource.setAuth(auth);
        ProtectedEnvironment protectedEnvironment = new ProtectedEnvironment();
        protectedEnvironment.setPort(11);
        protectedEnvironment.setEndpoint("192.167.1.1");
        resource.setEnvironment(protectedEnvironment);
        RedisValidator.checkNode(resource);
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
            new ExceptionMatcher(CommonErrorCode.ILLEGAL_PARAM, "Redis node type is not correct."));
        ProtectedResource resource2 = new ProtectedResource();
        resource2.setName("192168");
        resource2.setType(ResourceTypeEnum.DATABASE.getType());
        resource2.setSubType(ResourceSubTypeEnum.REDIS.getType());
        Authentication auth = new Authentication();
        resource2.setAuth(auth);
        ProtectedEnvironment protectedEnvironment = new ProtectedEnvironment();
        protectedEnvironment.setPort(11);
        protectedEnvironment.setEndpoint("192.167.1.1");
        resource2.setEnvironment(protectedEnvironment);
        resource2.setExtendInfo(new HashMap<String, String>() {
            {
                put(RedisConstant.TYPE, DatabaseConstants.CLUSTER_TARGET);
            }
        });
        RedisValidator.checkNode(resource2);
    }

    /**
     * 用例场景：创建集群，集群节点的extendInfo的ip为空
     * 前置条件：服务正常、集群节点正常
     * 检查点: 报错，错误码：ILLEGAL_PARAM
     */
    @Test
    public void should_throw_LegoCheckedException_if_node_param_without_ip_of_extendInfo_when_register_cluster() {
        expectedException.expect(LegoCheckedException.class);
        expectedException.expect(new ExceptionMatcher(CommonErrorCode.ILLEGAL_PARAM, "Ip is illegal."));
        ProtectedResource resource3 = new ProtectedResource();
        resource3.setName("192168");
        resource3.setType(ResourceTypeEnum.DATABASE.getType());
        resource3.setSubType(ResourceSubTypeEnum.REDIS.getType());
        resource3.setAuth(new Authentication());
        ProtectedEnvironment protectedEnvironment = new ProtectedEnvironment();
        protectedEnvironment.setPort(11);
        protectedEnvironment.setEndpoint("192.167.1.1");
        resource3.setEnvironment(protectedEnvironment);
        resource3.setExtendInfo(new HashMap<String, String>() {
            {
                put(RedisConstant.TYPE, DatabaseConstants.NODE_TARGET);
                put(RedisConstant.PORT, "127");
            }
        });
        RedisValidator.checkNode(resource3);
    }

    /**
     * 用例场景：创建集群，集群节点的extendInfo的端口不正确
     * 前置条件：服务正常、集群节点正常
     * 检查点: 报错，错误码：PORT_ERROR
     */
    @Test
    public void should_throw_LegoCheckedException_if_node_param_with_wrong_port_of_extendInfo_when_register_cluster() {
        expectedException.expect(LegoCheckedException.class);
        expectedException.expect(
            new ExceptionMatcher(CommonErrorCode.IP_PORT_ERROR, "Port can not be empty and out of range."));
        ProtectedResource resource = new ProtectedResource();
        resource.setName("192168");
        resource.setType(ResourceTypeEnum.DATABASE.getType());
        resource.setSubType(ResourceSubTypeEnum.REDIS.getType());
        Authentication auth = new Authentication();
        resource.setAuth(auth);
        ProtectedEnvironment protectedEnvironment = new ProtectedEnvironment();
        protectedEnvironment.setPort(65536);
        protectedEnvironment.setEndpoint("192.167.1.1");
        resource.setEnvironment(protectedEnvironment);
        resource.setExtendInfo(new HashMap<String, String>() {
            {
                put(RedisConstant.TYPE, DatabaseConstants.NODE_TARGET);
                put(RedisConstant.PORT, "65536");
                put(RedisConstant.IP, "192.164.1.4");
            }
        });
        RedisValidator.checkNode(resource);
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
        ProtectedResource resource4 = new ProtectedResource();
        resource4.setName("192168");
        resource4.setType(ResourceTypeEnum.DATABASE.getType());
        resource4.setAuth(new Authentication());
        resource4.setSubType(ResourceSubTypeEnum.REDIS.getType());
        ProtectedEnvironment protectedEnvironment = new ProtectedEnvironment();
        protectedEnvironment.setPort(11);
        protectedEnvironment.setEndpoint("192.167.1.1");
        resource4.setEnvironment(protectedEnvironment);
        resource4.setExtendInfo(new HashMap<String, String>() {
            {
                put(RedisConstant.TYPE, DatabaseConstants.NODE_TARGET);
                put(RedisConstant.PORT, "127");
                put(RedisConstant.IP, "192.164.1.4");
            }
        });
        RedisValidator.checkNode(resource4);
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
        ProtectedResource resource = new ProtectedResource();
        resource.setName("192168");
        resource.setType(ResourceTypeEnum.DATABASE.getType());
        resource.setSubType(ResourceSubTypeEnum.REDIS.getType());
        Authentication auth = new Authentication();
        resource.setAuth(auth);
        ProtectedEnvironment protectedEnvironment = new ProtectedEnvironment();
        protectedEnvironment.setEndpoint("192.167.1.1");
        protectedEnvironment.setPort(11);
        resource.setEnvironment(protectedEnvironment);
        resource.setExtendInfo(new HashMap<String, String>() {
            {
                put(RedisConstant.TYPE, DatabaseConstants.NODE_TARGET);
                put(RedisConstant.PORT, "127");
                put(RedisConstant.IP, "192.164.1.4");
                put(RedisConstant.CLIENT_PATH, "/srv/BigData/test");
            }
        });
        RedisValidator.checkNode(resource);
    }

    /**
     * 用例场景：创建集群，进行集群节点检验
     * 前置条件：服务正常、集群节点正常，参数信息正确
     * 检查点: 不报错
     */
    @Test
    public void check_node_param_success() {
        ProtectedResource resource = new ProtectedResource();
        resource.setName("192168");
        resource.setType(ResourceTypeEnum.DATABASE.getType());
        resource.setSubType(ResourceSubTypeEnum.REDIS.getType());
        resource.setAuth(new Authentication());
        ProtectedEnvironment protectedEnvironment = new ProtectedEnvironment();
        protectedEnvironment.setEndpoint("192.167.1.1");
        protectedEnvironment.setPort(11);
        resource.setEnvironment(protectedEnvironment);
        resource.setExtendInfo(new HashMap<String, String>() {
            {
                put(RedisConstant.IP, "192.164.1.4");
                put(RedisConstant.TYPE, DatabaseConstants.NODE_TARGET);
                put(RedisConstant.PORT, "127");
                put(RedisConstant.CLIENT_PATH, "/srv/BigData/test");
            }
        });
        resource.setDependencies(new HashMap<String, List<ProtectedResource>>() {
            {
                put(DatabaseConstants.AGENTS, Lists.newArrayList(new ProtectedResource()));
            }
        });
        RedisValidator.checkNode(resource);
    }
}