package openbackup.data.access.framework.livemount.controller;

import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyMap;
import static org.mockito.ArgumentMatchers.anyString;
import static org.powermock.api.mockito.PowerMockito.doNothing;
import static org.powermock.api.mockito.PowerMockito.when;

import openbackup.data.access.framework.livemount.common.LiveMountRestApi;
import openbackup.data.access.framework.livemount.common.model.LiveMountModel;
import openbackup.data.access.framework.livemount.controller.livemount.LiveMountController;
import openbackup.data.access.framework.livemount.controller.livemount.model.LiveMountMigrateRequest;
import openbackup.data.access.framework.livemount.controller.livemount.model.LiveMountStatus;
import openbackup.data.access.framework.livemount.controller.livemount.model.LiveMountUpdate;
import openbackup.data.access.framework.livemount.controller.livemount.model.LiveMountUpdateMode;
import openbackup.data.access.framework.livemount.dao.LiveMountEntityDao;
import openbackup.data.access.framework.livemount.dao.LiveMountModelDao;
import openbackup.data.access.framework.livemount.dao.LiveMountPolicyEntityDao;
import openbackup.data.access.framework.livemount.service.LiveMountService;
import openbackup.data.access.framework.livemount.service.PolicyService;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.system.base.common.aspect.OperationInterceptor;
import openbackup.system.base.common.aspect.OperationLogAspect;
import openbackup.system.base.common.aspect.OperationLogService;
import openbackup.system.base.common.aspect.RightsControlInterceptor;
import openbackup.system.base.common.aspect.StringifyConverter;
import openbackup.system.base.common.constants.TokenBo;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.model.livemount.LiveMountEntity;
import openbackup.system.base.common.scurity.TokenVerificationService;
import openbackup.system.base.common.utils.RightsControl;
import openbackup.system.base.query.PageQueryService;
import openbackup.system.base.sdk.SystemSpecificationService;
import openbackup.system.base.sdk.auth.AuthRestApi;
import openbackup.system.base.sdk.auth.api.AuthNativeApi;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.copy.model.BasePage;
import openbackup.system.base.sdk.copy.model.Copy;
import openbackup.system.base.sdk.resource.EnvironmentRestApi;
import openbackup.system.base.sdk.resource.ResourceRestApi;
import openbackup.system.base.sdk.resource.model.Environment;
import openbackup.system.base.sdk.resource.model.FileSetEntity;
import openbackup.system.base.sdk.schedule.ScheduleRestApi;
import openbackup.system.base.service.DeployTypeService;
import openbackup.system.base.util.ProviderRegistry;

import org.junit.Assert;
import org.junit.Before;
import org.junit.Rule;
import org.junit.Test;
import org.junit.rules.ExpectedException;
import org.junit.runner.RunWith;
import org.mockito.InjectMocks;
import org.mockito.Mockito;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.reflect.Whitebox;
import org.springframework.aop.aspectj.annotation.AspectJProxyFactory;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.beans.factory.annotation.Value;
import org.springframework.boot.test.context.SpringBootTest;
import org.springframework.boot.test.mock.mockito.MockBean;
import org.springframework.test.context.ContextConfiguration;
import org.springframework.test.context.junit4.SpringRunner;
import org.springframework.web.servlet.mvc.method.annotation.RequestMappingHandlerMapping;

import java.util.AbstractMap;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Optional;

/**
 * Live Mount Controller Test
 *
 * @author l00272247
 * @since 2021-01-16
 */
@RunWith(SpringRunner.class)
@PrepareForTest(LiveMountController.class)
@SpringBootTest
@ContextConfiguration(classes = {LiveMountController.class, OperationLogAspect.class, StringifyConverter.class})
public class LiveMountControllerTest {
    /**
     * ExpectedException
     */
    @Rule
    public final ExpectedException expectedException = ExpectedException.none();

    @MockBean
    private LiveMountEntityDao liveMountEntityDao;

    @MockBean
    private LiveMountPolicyEntityDao liveMountPolicyEntityDao;

    @MockBean
    private PolicyService policyService;

    @MockBean
    private CopyRestApi copyRestApi;

    @MockBean
    private ResourceRestApi resourceRestApi;

    @MockBean
    private LiveMountRestApi liveMountClientRestApi;

    @MockBean
    private ScheduleRestApi scheduleRestApi;

    @MockBean
    private PageQueryService pageQueryService;

    @MockBean
    private LiveMountModelDao liveMountModelDao;

    @MockBean
    private EnvironmentRestApi environmentRestApi;

    @MockBean
    private ProviderRegistry providerRegistry;

    @MockBean
    private AuthRestApi authRestApi;

    @MockBean
    private LiveMountService liveMountService;

    @Autowired
    private LiveMountController liveMountController;

    @MockBean
    private RequestMappingHandlerMapping requestMappingHandlerMapping;

    @Value("${service.url.alarm-manager}")
    private String alarmUrl;

    @MockBean
    private List<OperationInterceptor<?>> operationInterceptors;

    @MockBean
    private TokenVerificationService tokenVerificationService;

    @MockBean
    private OperationLogService operationLogService;

    @InjectMocks
    @Autowired
    private OperationLogAspect operationLogAspect;

    @MockBean
    private RightsControlInterceptor rightsControlInterceptor;

    @MockBean
    private SystemSpecificationService systemSpecificationService;

    @MockBean
    private DeployTypeService deployTypeService;

    @MockBean
    private ResourceService resourceService;

    @MockBean
    private AuthNativeApi authNativeApi;

    private LiveMountController controller;

    /**
     * before set LiveMountController
     */
    @Before
    public void setLiveMountController() {
        AspectJProxyFactory factory = new AspectJProxyFactory(liveMountController);
        factory.addAspect(operationLogAspect);
        controller = factory.getProxy();
        TokenBo.UserBo userBo = TokenBo.UserBo.builder().name("user").id("id").build();
        TokenBo tokenBo = TokenBo.builder().user(userBo).build();
        when(tokenVerificationService.parsingTokenFromRequest()).thenReturn(tokenBo);
        when(rightsControlInterceptor.getSupportedAnnotationType()).thenReturn(RightsControl.class);
        doNothing().when(rightsControlInterceptor).intercept(any(), any(), anyMap(), any());
    }

    /**
     * updateLiveMount can not mount
     */
    @Test
    public void updateLiveMountCannotMount() {
        expectedException.expect(LegoCheckedException.class);
        expectedException.expectMessage("Current status is not allow update operation");
        LiveMountEntity liveMountEntity = getLiveMountEntity();
        liveMountEntity.setStatus(LiveMountStatus.UNMOUNTING.getName());
        PowerMockito.when(liveMountService.selectLiveMountEntityById("1")).thenReturn(liveMountEntity);
        controller.updateLiveMount("1", getLiveMountUpdate());
    }

    /**
     * updateLiveMount copy mode is not latest
     */
    @Test
    public void updateLiveMountCopyModeIsNotLatest() {
        LiveMountUpdate liveMountUpdate = getLiveMountUpdate();
        liveMountUpdate.setMode(LiveMountUpdateMode.SPECIFIED);
        LiveMountEntity liveMountEntity = getLiveMountEntity();
        PowerMockito.when(liveMountEntityDao.selectById(any())).thenReturn(liveMountEntity);
        PowerMockito.when(liveMountService.selectLiveMountEntityById("1")).thenReturn(liveMountEntity);
        PowerMockito.when(resourceRestApi.queryResource(any())).thenReturn(getFileSetEntity());
        PowerMockito.when(liveMountService.queryValidCopy(liveMountEntity.getResourceId(),
                liveMountUpdate.getCopyId())).thenReturn(getCopy());
        controller.updateLiveMount("1", liveMountUpdate);
        Assert.assertEquals(liveMountEntity.getCopyId(), getCopy().getUuid());
    }

    /**
     * updateLiveMount policy is null
     */
    @Test
    public void updateLiveMountPolicyIsNull() {
        LiveMountEntity liveMountEntity = getLiveMountEntity();
        liveMountEntity.setPolicyId(null);
        PowerMockito.when(liveMountEntityDao.selectById(any())).thenReturn(liveMountEntity);
        PowerMockito.when(liveMountService.selectLiveMountEntityById("1")).thenReturn(liveMountEntity);
        PowerMockito.when(resourceRestApi.queryResource(any())).thenReturn(getFileSetEntity());
        controller.updateLiveMount("1", getLiveMountUpdate());
        Assert.assertNull(liveMountEntity.getCopyId());
    }

    /**
     * updateLiveMount
     */
    @Test
    public void updateLiveMount() {
        LiveMountUpdate liveMountUpdate = getLiveMountUpdate();
        LiveMountEntity liveMountEntity = getLiveMountEntity();
        PowerMockito.when(liveMountEntityDao.selectById(any())).thenReturn(liveMountEntity);
        PowerMockito.when(liveMountService.selectLiveMountEntityById("1")).thenReturn(liveMountEntity);
        PowerMockito.when(resourceRestApi.queryResource(any())).thenReturn(getFileSetEntity());
        controller.updateLiveMount("1", liveMountUpdate);
        Assert.assertNull(liveMountEntity.getCopyId());
    }

    /**
     * unmountLiveMount
     */
    @Test
    public void unmountLiveMount() {
        controller.unmountLiveMount("1", false,
                false);
        Assert.assertNotNull(controller);
    }

    /**
     * queryLiveMountEntities
     */
    @Test
    public void queryLiveMountEntities() {
        BasePage<LiveMountModel> liveMountModelBasePage = controller.queryLiveMountEntities(0, 0, "",
                new ArrayList<>());
        Assert.assertNull(liveMountModelBasePage);
    }

    /**
     * queryTargetEnvironments
     */
    @Test
    public void queryTargetEnvironments() {
        BasePage<Environment> basePage = new BasePage<>();
        PowerMockito.when(liveMountService.queryTargetEnvironments(anyString())).thenReturn(basePage);
        BasePage<Environment> targetEnvironment = controller.queryTargetEnvironments("1");
        Assert.assertEquals(targetEnvironment, basePage);
    }

    /**
     * active livemount id is null
     */
    @Test
    public void activeLiveMountIsNull() {
        Assert.assertThrows("live mount id is not null.", LegoCheckedException.class, () -> controller.active(null));
    }

    /**
     * active
     */
    @Test
    public void active() {
        controller.active("1");
        Assert.assertNotNull(controller);
    }

    /**
     * deactivate live mount id is null
     */
    @Test
    public void deactivateLiveMountIsNull() {
        expectedException.expect(LegoCheckedException.class);
        expectedException.expectMessage("live mount id is not null.");
        controller.deactivate(null);
    }

    /**
     * deactivate
     */
    @Test
    public void deactivate() {
        controller.deactivate("1");
        Assert.assertNotNull(controller);
    }

    /**
     * migrate live mount id is null
     */
    @Test
    public void migrateLiveMountIsNull() {
        expectedException.expect(LegoCheckedException.class);
        expectedException.expectMessage("live mount id is not null.");
        controller.migrate(null, new LiveMountMigrateRequest());
    }

    /**
     * migrate
     */
    @Test
    public void migrate() {
        controller.migrate("1", new LiveMountMigrateRequest());
        Assert.assertNotNull(controller);
    }

    /**
     * CyberEngine can't restore when worm
     */
    @Test
    public void test_checkResourceSubType_when_cyberEngineWorm_then_exception() {
        LiveMountController controller = new LiveMountController();
        DeployTypeService deployTypeService = Mockito.mock(DeployTypeService.class);
        Mockito.doReturn(true).when(deployTypeService).isCyberEngine();
        ResourceService resourceService = Mockito.mock(ResourceService.class);
        ProtectedResource protectedResource = new ProtectedResource();
        Map<String, String> extInfo = new HashMap<>();
        extInfo.put("fileSubType", "1");
        protectedResource.setExtendInfo(extInfo);
        Mockito.doReturn(Optional.of(protectedResource)).when(resourceService).getResourceById(anyString());
        Whitebox.setInternalState(controller, "deployTypeService", deployTypeService);
        Whitebox.setInternalState(controller, "resourceService", resourceService);
        Assert.assertThrows(LegoCheckedException.class,
            () -> Whitebox.invokeMethod(controller, "checkResourceSubType", "test"));
    }

    /**
     * getEntry
     *
     * @return Map
     */
    public Map.Entry<Copy, List<LiveMountEntity>> getEntry() {
        List<LiveMountEntity> liveMountUuidList = new ArrayList<>();
        LiveMountEntity liveMountEntity = new LiveMountEntity();
        liveMountEntity.setId("1");
        liveMountUuidList.add(liveMountEntity);
        return new AbstractMap.SimpleEntry<>(getCopy(), liveMountUuidList);
    }

    /**
     * getCopy
     *
     * @return Copy
     */
    private Copy getCopy() {
        Copy copy = new Copy();
        copy.setStatus("Normal");
        copy.setResourceId("uuid0");
        copy.setGeneration(1);
        copy.setTimestamp(Long.toString(System.currentTimeMillis()));
        return copy;
    }

    /**
     * getLiveMountUpdate
     *
     * @return LiveMountUpdate
     */
    private LiveMountUpdate getLiveMountUpdate() {
        LiveMountUpdate liveMountUpdate = new LiveMountUpdate();
        liveMountUpdate.setCopyId("1");
        liveMountUpdate.setMode(LiveMountUpdateMode.LATEST);
        return liveMountUpdate;
    }

    /**
     * getLiveMountEntity
     *
     * @return LiveMountEntity
     */
    private LiveMountEntity getLiveMountEntity() {
        LiveMountEntity entity = new LiveMountEntity();
        entity.setId("1");
        entity.setStatus(LiveMountStatus.AVAILABLE.getName());
        return entity;
    }

    /**
     * getFileSetEntity
     *
     * @return FileSetEntity
     */
    private FileSetEntity getFileSetEntity() {
        FileSetEntity fileSetEntity = new FileSetEntity();
        fileSetEntity.setEnvironmentUuid("1");
        return fileSetEntity;
    }
}
