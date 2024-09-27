package openbackup.obs.plugin.service.impl;

import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyString;

import openbackup.access.framework.resource.service.AgentBusinessService;
import openbackup.data.protection.access.provider.sdk.base.Authentication;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import com.huawei.oceanprotect.kms.sdk.EncryptorService;
import openbackup.obs.plugin.common.constants.EnvironmentConstant;
import openbackup.system.base.common.utils.StringUtil;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import org.junit.Assert;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.InjectMocks;
import org.mockito.Mock;
import org.mockito.Mockito;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.core.classloader.annotations.PowerMockIgnore;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.PowerMockRunner;
import org.powermock.modules.junit4.PowerMockRunnerDelegate;
import org.springframework.test.context.junit4.SpringRunner;

import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Optional;

/**
 * 功能描述
 *
 * @author w00607005
 * @since 2023-11-20
 */
@RunWith(PowerMockRunner.class)
@PowerMockRunnerDelegate(SpringRunner.class)
@PrepareForTest(value = {StringUtil.class})
@PowerMockIgnore( {"javax.management.*", "jdk.internal.reflect.*"})
public class ObjectStorageConnectionCheckerTest {
    @Mock
    private ResourceService resourceService;

    @Mock
    private AgentBusinessService agentBusinessService;

    @Mock
    private EncryptorService encryptorService;

    @InjectMocks
    private ObjectStorageConnectionChecker objectStorageConnectionChecker;

    /**
     * 用例名称：监测成功
     * 前置条件：无
     * 检查点：无报错
     */
    @Test
    public void test_applicable_success() {
        // run the test
        boolean result = objectStorageConnectionChecker.applicable(createProtectedResource());

        // verify the results
        Assert.assertTrue(result);
    }

    /**
     * 用例名称：查找agent成功
     * 前置条件：无
     * 检查点：无报错
     */
    @Test
    public void test_collect_connectable_resources_success() {
        // setup
        ProtectedEnvironment protectedEnvironment = new ProtectedEnvironment();
        PowerMockito.when(resourceService.getResourceById(anyString())).thenReturn(Optional.of(protectedEnvironment));

        // run the test
        Map<ProtectedResource, List<ProtectedEnvironment>> result
            = objectStorageConnectionChecker.collectConnectableResources(createProtectedResource());

        // verify the results
        Assert.assertEquals(1, result.size());
    }

    private ProtectedResource createProtectedResource() {
        ProtectedResource protectedResource = new ProtectedResource();
        protectedResource.setSubType(ResourceSubTypeEnum.OBJECT_STORAGE.getType());
        Map<String, String> extendInfo = new HashMap<>();
        extendInfo.put(EnvironmentConstant.KEY_AGENTS, "");
        protectedResource.setExtendInfo(extendInfo);
        return protectedResource;
    }

    @Test
    public void test_collect_connectable_resources_decrypt() {
        // setup
        ProtectedEnvironment protectedEnvironment = new ProtectedEnvironment();
        PowerMockito.when(resourceService.getResourceById(anyString())).thenReturn(Optional.of(protectedEnvironment));

        ProtectedResource resource = createProtectedResource();
        Authentication auth = new Authentication();
        Map<String, String> authInfo = new HashMap<>();
        authInfo.put(EnvironmentConstant.KEY_SK, "sk");
        auth.setExtendInfo(authInfo);
        resource.setAuth(auth);
        resource.setUuid("1");
        // run the test
        Map<ProtectedResource, List<ProtectedEnvironment>> result
            = objectStorageConnectionChecker.collectConnectableResources(resource);

        // verify the results
        Mockito.verify(encryptorService, Mockito.times(1)).decrypt(any());
        Assert.assertEquals(1, result.size());
    }
}