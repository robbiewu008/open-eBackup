package openbackup.data.access.framework.livemount.provider;

import openbackup.data.access.client.sdk.api.framework.dee.DeeLiveMountRestApi;
import openbackup.data.access.framework.core.manager.ProviderManager;
import openbackup.data.access.framework.livemount.common.model.LiveMountCloneRequest;
import openbackup.data.access.framework.livemount.provider.mock.TestAbstractLiveMountProvider;

import openbackup.data.access.framework.livemount.service.impl.PerformanceValidator;
import openbackup.data.protection.access.provider.sdk.copy.CopyInfoBo;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.model.livemount.LiveMountEntity;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.copy.model.Copy;
import openbackup.system.base.sdk.resource.EnvironmentRestApi;
import openbackup.system.base.service.DeployTypeService;

import org.junit.Assert;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mockito;
import org.powermock.modules.junit4.PowerMockRunner;
import org.powermock.modules.junit4.PowerMockRunnerDelegate;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.boot.test.context.SpringBootTest;
import org.springframework.boot.test.mock.mockito.MockBean;
import org.springframework.test.context.junit4.SpringRunner;

import java.util.HashMap;
import java.util.Map;

/**
 * AbstractLiveMountProvider测试
 *
 * @author h30027154
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-07-20
 */
@RunWith(PowerMockRunner.class)
@PowerMockRunnerDelegate(SpringRunner.class)
@SpringBootTest(classes = {TestAbstractLiveMountProvider.class, ProviderManager.class})
@MockBean( {
    CopyRestApi.class, PerformanceValidator.class
})
public class AbstractLiveMountProviderTest {
    @Autowired
    private TestAbstractLiveMountProvider liveMountProvider;

    @MockBean
    private CopyRestApi copyRestApi;

    @MockBean
    private PerformanceValidator performanceValidator;

    @MockBean
    private DeployTypeService deployTypeService;

    @MockBean
    private EnvironmentRestApi environmentRestApi;

    @MockBean
    private ResourceService resourceService;

    @MockBean
    private DeeLiveMountRestApi deeLiveMountRestApi;

    /**
     * 用例场景：克隆副本成功
     * 前置条件：无
     * 检查点：副本信息克隆成功
     */
    @Test
    public void clone_copy() {
        LiveMountCloneRequest cloneRequest = new LiveMountCloneRequest();
        cloneRequest.setTargetCopyUuid("targetCopyId");

        LiveMountEntity liveMountEntity = new LiveMountEntity();
        cloneRequest.setLiveMountEntity(liveMountEntity);
        liveMountEntity.setFileSystemShareInfo("{}");

        Copy copy = new Copy();
        cloneRequest.setSourceCopy(copy);
        copy.setUuid("copyId");
        copy.setResourceSubType("subType");
        copy.setProperties("{}");
        CopyInfoBo copyInfoBo = liveMountProvider.cloneCopy(cloneRequest);
        Assert.assertEquals("targetCopyId", copyInfoBo.getUuid());
    }

    /**
     * 用例场景：更新perfomance设置成功
     * 前置条件：无
     * 检查点：更新成功
     */
    @Test
    public void update_performance_setting() {
        LiveMountEntity liveMountEntity = new LiveMountEntity();
        liveMountEntity.setParameters("{\"performance\":{}}");

        Mockito.when(copyRestApi.queryCopyByID(Mockito.any())).thenReturn(new Copy());
        liveMountProvider.updatePerformanceSetting(liveMountEntity);

        Mockito.verify(performanceValidator).loadPerformance(Mockito.any());
    }

    @Test
    public void check_script_path_and_type() {
        Map<String, Object> params = new HashMap<>();
        params.put(AbstractLiveMountProvider.PRE_SCRIPT, "-fas");
        params.put(AbstractLiveMountProvider.POST_SCRIPT, "fd.batt");
        params.put(AbstractLiveMountProvider.FAIL_SCRIPT, "fdas.shddd");

        Assert.assertThrows(LegoCheckedException.class,
            () -> liveMountProvider.checkScriptPathAndType(params, AbstractLiveMountProvider.WINDOWS));
        Assert.assertThrows(LegoCheckedException.class,
            () -> liveMountProvider.checkScriptPathAndType(params, "other"));
    }
}
