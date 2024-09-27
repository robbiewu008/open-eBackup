/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

package openbackup.system.base.util;

import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.sdk.auth.UserAuthRequest;
import openbackup.system.base.sdk.cluster.TargetClusterRestApi;
import openbackup.system.base.sdk.cluster.model.DataMoverIps;
import openbackup.system.base.sdk.cluster.model.TargetClusterVo;
import openbackup.system.base.sdk.infrastructure.model.beans.NodePodInfo;

import io.jsonwebtoken.lang.Assert;
import openbackup.system.base.util.Routing;

import org.junit.Ignore;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mockito;
import org.mockito.junit.MockitoJUnitRunner;
import org.springframework.boot.test.context.SpringBootTest;
import org.springframework.context.ApplicationContext;

import java.net.ConnectException;
import java.util.ArrayList;
import java.util.List;

import javax.net.ssl.SSLHandshakeException;

/**
 * Routing Test
 *
 * @author twx1009756
 * @since 2021-03-17
 */
@RunWith(MockitoJUnitRunner.class)
@SpringBootTest(classes = {
    Routing.class, ApplicationContext.class, TargetClusterVo.class, TargetClusterRestApi.class
})
public class RoutingTest {

    private final TargetClusterRestApi targetClusterRestApi = Mockito.mock(TargetClusterRestApi.class);

    /**
     * 测试Routing方法
     */
    @Test
    @Ignore
    public void testGet() {
        TargetClusterVo targetCluster = getTargetCluster();
        try {
            getToken(targetCluster, targetClusterRestApi);
        } catch (Exception ex) {
            ex.printStackTrace();
        }
    }

    private static void getToken(TargetClusterVo targetCluster, TargetClusterRestApi targetClusterRestApi) {
        UserAuthRequest req = new UserAuthRequest();
        req.setUserName(targetCluster.getUsername());
        String password = "password";
        req.setPassword(password);
        // targetCluster.call(uri -> targetClusterRestApi.getToken(uri, req).getToken());
        targetCluster.get(uri -> targetClusterRestApi.getToken(uri, req).getToken());
    }

    private static TargetClusterVo getTargetCluster() {
        TargetClusterVo targetCluster = new TargetClusterVo();
        targetCluster.setClusterId("2");
        List<String> mgrIpList = new ArrayList<>();
        mgrIpList.add("8.40.99.103");
        mgrIpList.add("8.40.102.109");
        mgrIpList.add("8.40.102.110");
        targetCluster.setMgrIpList(mgrIpList);
        targetCluster.setPort(25081);
        targetCluster.setUsername("username");
        targetCluster.setPassword("password");
        targetCluster.setStatus(6);
        DataMoverIps ips = new DataMoverIps();
        targetCluster.setIps(ips);
        targetCluster.setEsn("202103231925");
        List<NodePodInfo> netplaneInfo = new ArrayList<>();
        targetCluster.setNetplaneInfo(netplaneInfo);
        return targetCluster;
    }

    /**
     * 测试FindProvider方法
     */
    @Test
    public void testFindProvider() {
        try {
            List hosts = buildHosts();
            int port = 6379;
            String protocol = "8.40.99.102";
            new Routing(hosts);
            new Routing(hosts, port);
            new Routing(protocol, hosts, port);
        } catch (Exception ex) {
            ex.printStackTrace();
        }
    }

    /**
     * 测试Routing方法
     */
    @Test
    public void test2() {
        try {
            List hosts = new ArrayList<>();
            new Routing(hosts);
        } catch (Exception ex) {
            ex.printStackTrace();
        }
    }

    /**
     * 测试Routing方法
     */
    @Test
    public void testRunning() {
        TargetClusterVo targetCluster = getTargetCluster();
        targetCluster.run(Assert::notNull);
    }

    /**
     * 测试Routing方法
     */
    @Test
    public void testRun() {
        TargetClusterVo targetCluster = getTargetCluster();
        targetCluster.run(uri -> new LegoCheckedException("good bye."));
    }

    /**
     * 用例场景：判断异常是否属于可重试异常
     * 前置条件：1.可重试异常
     * 检  查  点：异常为重试异常
     */
    @Test
    public void check_exception_is_retryable_exception_success() {
        Routing routing = new Routing(buildHosts(), 8088);
        ConnectException exception = new ConnectException("test");
        org.junit.Assert.assertTrue(routing.isRetryableException(exception));
    }

    /**
     * 用例场景：判断异常是否属于可重试异常
     * 前置条件：1.可重试异常
     * 检  查  点：异常不为重试异常
     */
    @Test
    public void check_exception_is_not_retryable_exception_success() {
        Routing routing = new Routing(buildHosts(), 8088);
        LegoCheckedException exception = new LegoCheckedException("test");
        org.junit.Assert.assertFalse(routing.isRetryableException(exception));
    }

    /**
     * 用例场景：判断异常是否属于可重试异常
     * 前置条件：1.证书异常
     * 检  查  点：异常不为重试异常
     */
    @Test
    public void check_exception_is_ssl_handshake_exception_success() {
        Routing routing = new Routing(buildHosts(), 8088);
        SSLHandshakeException exception = new SSLHandshakeException("test");
        org.junit.Assert.assertFalse(routing.isRetryableException(exception));
    }

    private List<String> buildHosts() {
        List<String> hosts = new ArrayList<>();
        hosts.add("8.40.99.101");
        hosts.add("8.40.99.102");
        hosts.add("8.40.99.103");
        return hosts;
    }
}
