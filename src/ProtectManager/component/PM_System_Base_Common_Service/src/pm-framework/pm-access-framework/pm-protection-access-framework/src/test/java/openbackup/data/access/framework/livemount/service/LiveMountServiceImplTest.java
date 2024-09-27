package openbackup.data.access.framework.livemount.service;

import static org.assertj.core.api.Assertions.assertThat;
import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyBoolean;
import static org.mockito.ArgumentMatchers.anyString;
import static org.mockito.ArgumentMatchers.isNull;
import static org.powermock.api.mockito.PowerMockito.when;

import com.google.common.base.Joiner;
import com.google.common.collect.ImmutableMap;

import openbackup.data.access.framework.copy.mng.service.CopyAuthVerifyService;
import openbackup.data.access.framework.copy.mng.service.CopyService;
import openbackup.data.access.framework.core.dao.CopyMapper;
import openbackup.data.access.framework.core.entity.CopiesEntity;
import openbackup.data.access.framework.core.manager.ProviderManager;
import openbackup.data.access.framework.livemount.common.LiveMountOperateType;
import openbackup.data.access.framework.livemount.common.LiveMountRestApi;
import openbackup.data.access.framework.livemount.common.enums.CopyDataSelection;
import openbackup.data.access.framework.livemount.common.model.LiveMountFileSystemShareInfo;
import openbackup.data.access.framework.livemount.common.model.LiveMountModel;
import openbackup.data.access.framework.livemount.common.model.LiveMountObject;
import openbackup.data.access.framework.livemount.common.model.UnmountExtendParam;
import openbackup.data.access.framework.livemount.controller.livemount.model.LiveMountEnableStatus;
import openbackup.data.access.framework.livemount.controller.livemount.model.LiveMountStatus;
import openbackup.data.access.framework.livemount.dao.LiveMountEntityDao;
import openbackup.data.access.framework.livemount.dao.LiveMountModelDao;
import openbackup.data.access.framework.livemount.dao.LiveMountPolicyEntityDao;
import openbackup.data.access.framework.livemount.data.LiveMountServiceImplTestData;
import openbackup.data.access.framework.livemount.entity.LiveMountPolicyEntity;
import openbackup.data.access.framework.livemount.provider.DefaultLiveMountServiceProvider;
import openbackup.data.access.framework.livemount.provider.LiveMountFlowService;
import openbackup.data.access.framework.livemount.provider.LiveMountServiceProvider;
import openbackup.data.access.framework.livemount.service.impl.LiveMountServiceImpl;
import openbackup.data.protection.access.provider.sdk.livemount.LiveMountInterceptorProvider;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;

import com.huawei.oceanprotect.base.cluster.sdk.service.StorageUnitService;
import com.huawei.oceanprotect.job.sdk.JobService;

import openbackup.system.base.common.aspect.OperationLogService;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.constants.IsmNumberConstant;
import openbackup.system.base.common.constants.LegoInternalEvent;
import openbackup.system.base.common.constants.TokenBo;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.model.livemount.LiveMountEntity;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.query.PageQueryService;
import openbackup.system.base.sdk.SystemSpecificationService;
import openbackup.system.base.sdk.auth.api.AuthNativeApi;
import openbackup.system.base.sdk.cluster.model.StorageUnitVo;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.copy.model.BasePage;
import openbackup.system.base.sdk.copy.model.Copy;
import openbackup.system.base.sdk.copy.model.CopyGeneratedByEnum;
import openbackup.system.base.sdk.copy.model.CopyResourceSummary;
import openbackup.system.base.sdk.copy.model.CopyStatus;
import openbackup.system.base.sdk.livemount.model.LiveMountTargetLocation;
import openbackup.system.base.sdk.livemount.model.Performance;
import openbackup.system.base.sdk.resource.EnvironmentRestApi;
import openbackup.system.base.sdk.resource.ResourceRestApi;
import openbackup.system.base.sdk.resource.VMWareRestApi;
import openbackup.system.base.sdk.resource.model.Environment;
import openbackup.system.base.sdk.resource.model.FileSetEntity;
import openbackup.system.base.sdk.resource.model.ResourceEntity;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.sdk.resource.model.VirtualResourceSchema;
import openbackup.system.base.sdk.schedule.ScheduleRestApi;
import openbackup.system.base.sdk.schedule.model.ScheduleResponse;
import openbackup.system.base.service.DeployTypeService;
import openbackup.system.base.util.ProviderRegistry;

import com.baomidou.mybatisplus.core.MybatisConfiguration;
import com.baomidou.mybatisplus.core.metadata.TableInfoHelper;

import org.apache.ibatis.builder.MapperBuilderAssistant;
import org.assertj.core.util.Lists;
import org.junit.Assert;
import org.junit.Before;
import org.junit.Rule;
import org.junit.Test;
import org.junit.jupiter.api.Assertions;
import org.junit.rules.ExpectedException;
import org.junit.runner.RunWith;
import org.mockito.ArgumentMatchers;
import org.mockito.InjectMocks;
import org.mockito.Mock;
import org.mockito.Mockito;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.PowerMockRunner;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.boot.test.context.SpringBootTest;
import org.springframework.test.context.ContextConfiguration;
import org.springframework.test.util.ReflectionTestUtils;

import java.lang.reflect.Field;
import java.util.AbstractMap;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Optional;
import java.util.concurrent.atomic.AtomicReference;

/**
 * 功能描述
 *
 * @author h30003246
 * @since 2021-02-27
 */
@RunWith(PowerMockRunner.class)
@PrepareForTest(TokenBo.class)
@SpringBootTest
@ContextConfiguration(classes = {LiveMountServiceImpl.class})
public class LiveMountServiceImplTest {
    /**
     * ExpectedException
     */
    @Rule
    public final ExpectedException expectedException = ExpectedException.none();

    @InjectMocks
    @Autowired
    private LiveMountServiceImpl liveMountService;

    @Mock
    private JobService jobService;

    @Mock
    private CopyService copyService;

    @Mock
    private LiveMountServiceProvider liveMountServiceProvider;

    @Mock
    private ResourceService resourceService;

    @Mock
    private ResourceRestApi resourceRestApi;

    @Mock
    private LiveMountEntityDao liveMountEntityDao;

    @Mock
    private CopyRestApi copyRestApi;

    @Mock
    private LiveMountRestApi liveMountClientRestApi;

    @Mock
    private ScheduleRestApi scheduleRestApi;

    @Mock
    private PageQueryService pageQueryService;

    @Mock
    private LiveMountModelDao liveMountModelDao;

    @Mock
    private LiveMountPolicyEntityDao liveMountPolicyEntityDao;

    @Mock
    private EnvironmentRestApi environmentRestApi;

    @Mock
    private ProviderRegistry providerRegistry;

    @Mock
    private VMWareRestApi vmWareRestApi;

    private BasePage<VirtualResourceSchema> virtualResourceSchemaBasePage;

    private BasePage<Environment> environmentBasePage;

    private TokenBo tokenBo;

    @Mock
    private ProviderManager providerManager;

    @Mock
    private DefaultLiveMountServiceProvider defaultLiveMountServiceProvider;

    @Mock
    private DeployTypeService deployTypeService;

    @Mock
    private PolicyService policyService;

    @Mock
    private SystemSpecificationService systemSpecificationService;

    @Mock
    private AuthNativeApi authNativeApi;

    @Mock
    private OperationLogService operationLogService;

    @Mock
    private CopyAuthVerifyService copyAuthVerifyService;

    @Mock
    private CopyMapper copyMapper;

    @Mock
    private StorageUnitService storageUnitService;

    /**
     * set token
     */
    @Before
    public void setUp() {
        tokenBo = LiveMountServiceImplTestData.getTokenBo();
        PowerMockito.mockStatic(TokenBo.class);
        PowerMockito.when(TokenBo.get()).thenReturn(tokenBo);
        Copy copy = getCopy();
        when(copyRestApi.queryCopyByID(any())).thenReturn(copy);
        CopyResourceSummary copyResourceSummary = Mockito.mock(CopyResourceSummary.class);
        when(copyService.queryCopyResourceSummary(anyString())).thenReturn(copyResourceSummary);
        Mockito.when(copyResourceSummary.getResourceType()).thenReturn("type");
    }

    /**
     * test createLiveMounts live mount data is not null
     */
    @Test
    public void createLiveMountsLiveMountIsNotNull() {
        expectedException.expect(LegoCheckedException.class);
        expectedException.expectMessage("can not mount same name resource on same target machine");
        LiveMountObject liveMountObject = LiveMountServiceImplTestData.getLiveMountObject();
        CopyResourceSummary copyResourceSummary = LiveMountServiceImplTestData.getCopyResourceSummary();
        PowerMockito.when(copyService.queryCopyResourceSummary(liveMountObject.getSourceResourceId()))
                .thenReturn(copyResourceSummary);
        PowerMockito.when(
                        providerRegistry.findProvider(
                                LiveMountFlowService.class, copyResourceSummary.getResourceSubType()))
                .thenReturn(null);
        FileSetEntity fileSetEntity = LiveMountServiceImplTestData.getFileSetEntity();
        PowerMockito.when(resourceRestApi.queryResource(any())).thenReturn(fileSetEntity);
        PowerMockito.when(liveMountEntityDao.selectOne(any())).thenReturn(new LiveMountEntity());
        liveMountService.createLiveMounts(liveMountObject, LiveMountServiceImplTestData.getCopy(), null);
    }

    /**
     * test createLiveMounts strict is true and count > 0
     */
    @Test(expected = LegoCheckedException.class)
    public void createLiveMountsStrictIsTrueAndCountIsOne() {
        expectedException.expect(LegoCheckedException.class);
        expectedException.expectMessage("not allow create duplicated live mount");
        LiveMountObject liveMountObject = LiveMountServiceImplTestData.getLiveMountObject();
        CopyResourceSummary copyResourceSummary = LiveMountServiceImplTestData.getCopyResourceSummary();
        PowerMockito.when(copyService.queryCopyResourceSummary(liveMountObject.getSourceResourceId()))
                .thenReturn(copyResourceSummary);
        LiveMountFlowService liveMountFlowService = PowerMockito.mock(LiveMountFlowService.class);
        PowerMockito.when(
                providerRegistry.findProvider(LiveMountFlowService.class, copyResourceSummary.getResourceSubType()))
            .thenReturn(liveMountFlowService);
        LiveMountInterceptorProvider liveMountInterceptorProvider = PowerMockito.mock(LiveMountInterceptorProvider.class);
        PowerMockito.when(
                providerManager.findProvider(LiveMountInterceptorProvider.class, copyResourceSummary.getResourceSubType()))
            .thenReturn(liveMountInterceptorProvider);
        LiveMountServiceProvider liveMountServiceProvider = PowerMockito.mock(LiveMountServiceProvider.class);
        PowerMockito.when(providerManager.findProviderOrDefault(LiveMountServiceProvider.class,
                copyResourceSummary.getResourceSubType(), defaultLiveMountServiceProvider))
            .thenReturn(liveMountServiceProvider);
        LiveMountEntity liveMountEntity = PowerMockito.mock(LiveMountEntity.class);
        PowerMockito.when(liveMountServiceProvider.buildLiveMountEntity(any(),any(),any())).thenReturn(liveMountEntity);
        FileSetEntity fileSetEntity = LiveMountServiceImplTestData.getFileSetEntity();
        PowerMockito.when(resourceRestApi.queryResource(any())).thenReturn(fileSetEntity);
        PowerMockito.when(liveMountEntityDao.selectCount(any())).thenReturn(1L);
        liveMountService.createLiveMounts(liveMountObject, LiveMountServiceImplTestData.getCopy(), null);
    }

    /**
     * test createLiveMounts strict is true and count = 0
     */
    @Test(expected = LegoCheckedException.class)
    public void createLiveMountsStrictIsTrueAndCountIsZero() {
        LiveMountObject liveMountObject = LiveMountServiceImplTestData.getLiveMountObject();
        CopyResourceSummary copyResourceSummary = LiveMountServiceImplTestData.getCopyResourceSummary();
        PowerMockito.when(copyService.queryCopyResourceSummary(liveMountObject.getSourceResourceId()))
                .thenReturn(copyResourceSummary);
        PowerMockito.when(
                        providerRegistry.findProvider(
                                LiveMountFlowService.class, copyResourceSummary.getResourceSubType()))
                .thenReturn(null);
        PowerMockito.when(providerManager.findProviderOrDefault(LiveMountServiceProvider.class,
            copyResourceSummary.getResourceSubType(), defaultLiveMountServiceProvider))
            .thenReturn(defaultLiveMountServiceProvider);
        FileSetEntity fileSetEntity = LiveMountServiceImplTestData.getFileSetEntity();
        PowerMockito.when(deployTypeService.isCyberEngine()).thenReturn(true);
        PowerMockito.when(resourceRestApi.queryResource(any())).thenReturn(fileSetEntity);
        PowerMockito.when(liveMountEntityDao.selectCount(any())).thenReturn(0L);
        LiveMountInterceptorProvider liveMountInterceptorProvider = PowerMockito.mock(LiveMountInterceptorProvider.class);
        PowerMockito.when(
                providerManager.findProvider(LiveMountInterceptorProvider.class, copyResourceSummary.getResourceSubType()))
            .thenReturn(liveMountInterceptorProvider);
        LiveMountServiceProvider liveMountServiceProvider = PowerMockito.mock(LiveMountServiceProvider.class);
        PowerMockito.when(providerManager.findProviderOrDefault(LiveMountServiceProvider.class,
                copyResourceSummary.getResourceSubType(), defaultLiveMountServiceProvider))
            .thenReturn(liveMountServiceProvider);
        LiveMountEntity liveMountEntity = PowerMockito.mock(LiveMountEntity.class);
        PowerMockito.when(liveMountServiceProvider.buildLiveMountEntity(any(),any(),any())).thenReturn(liveMountEntity);
        Map.Entry<Copy, List<LiveMountEntity>> liveMounts = liveMountService.createLiveMounts(liveMountObject,
            LiveMountServiceImplTestData.getCopy(), null);
        assertThat(liveMounts).isNotNull();
        Mockito.verify(liveMountEntityDao, Mockito.times(2)).insert(any());
    }

    /**
     * test createLiveMounts SubType is vwmare
     */
    @Test
    public void createLiveMountsVmwareAndFileset() {
        LiveMountObject liveMountObject = LiveMountServiceImplTestData.getLiveMountObject();
        CopyResourceSummary copyResourceSummary = LiveMountServiceImplTestData.getCopyResourceSummary();
        JSONObject jsonObject = JSONObject.fromObject(copyResourceSummary.getResourceProperties());
        jsonObject.set("sub_type", ResourceSubTypeEnum.VMWARE.getType());
        copyResourceSummary.setResourceProperties(jsonObject.toString());
        PowerMockito.when(copyService.queryCopyResourceSummary(liveMountObject.getSourceResourceId()))
                .thenReturn(copyResourceSummary);
        PowerMockito.when(
                        providerRegistry.findProvider(
                                LiveMountFlowService.class, copyResourceSummary.getResourceSubType()))
                .thenReturn(null);
        FileSetEntity fileSetEntity = LiveMountServiceImplTestData.getFileSetEntity();
        PowerMockito.when(resourceRestApi.queryResource(any())).thenReturn(fileSetEntity);
        PowerMockito.when(providerManager.findProviderOrDefault(LiveMountServiceProvider.class,
            copyResourceSummary.getResourceSubType(), defaultLiveMountServiceProvider))
            .thenReturn(defaultLiveMountServiceProvider);
        LiveMountServiceProvider liveMountServiceProvider = PowerMockito.mock(LiveMountServiceProvider.class);
        PowerMockito.when(providerManager.findProviderOrDefault(LiveMountServiceProvider.class,
                copyResourceSummary.getResourceSubType(), defaultLiveMountServiceProvider))
            .thenReturn(liveMountServiceProvider);
        LiveMountEntity liveMountEntity = PowerMockito.mock(LiveMountEntity.class);
        PowerMockito.when(liveMountServiceProvider.buildLiveMountEntity(any(),any(),any())).thenReturn(liveMountEntity);
        liveMountService.createLiveMounts(liveMountObject, LiveMountServiceImplTestData.getCopy(), null);

        //当sub_type为fileSet时 同样需要设置resourceName
        jsonObject.set("sub_type", ResourceSubTypeEnum.FILESET.getType());
        PowerMockito.when(providerManager.findProviderOrDefault(LiveMountServiceProvider.class,
            ResourceSubTypeEnum.FILESET.getType(), defaultLiveMountServiceProvider))
            .thenReturn(defaultLiveMountServiceProvider);
        copyResourceSummary.setResourceProperties(jsonObject.toString());
        PowerMockito.when(copyService.queryCopyResourceSummary(liveMountObject.getSourceResourceId()))
            .thenReturn(copyResourceSummary);
        PowerMockito.when(providerManager.findProviderOrDefault(LiveMountServiceProvider.class,
                copyResourceSummary.getResourceSubType(), defaultLiveMountServiceProvider))
            .thenReturn(liveMountServiceProvider);
        PowerMockito.when(liveMountServiceProvider.buildLiveMountEntity(any(),any(),any())).thenReturn(liveMountEntity);
        Map.Entry<Copy, List<LiveMountEntity>> liveMounts = liveMountService.createLiveMounts(liveMountObject, LiveMountServiceImplTestData.getCopy(), null);
        Assert.assertEquals(liveMounts.getKey().getUuid(), copyResourceSummary.getResourceId());
    }

    /**
     * test createLiveMounts
     */
    @Test
    public void createLiveMountsNotVmware() {
        LiveMountObject liveMountObject = LiveMountServiceImplTestData.getLiveMountObject();
        CopyResourceSummary copyResourceSummary = LiveMountServiceImplTestData.getCopyResourceSummary();
        PowerMockito.when(copyService.queryCopyResourceSummary(liveMountObject.getSourceResourceId()))
                .thenReturn(copyResourceSummary);
        LiveMountFlowService liveMountFlowService = PowerMockito.mock(LiveMountFlowService.class);
        PowerMockito.when(
                        providerRegistry.findProvider(
                                any(), anyString(), any()))
                .thenReturn(liveMountFlowService);
        FileSetEntity fileSetEntity = LiveMountServiceImplTestData.getFileSetEntity();
        PowerMockito.when(resourceRestApi.queryResource(any())).thenReturn(fileSetEntity);
        Map.Entry<Copy, List<LiveMountEntity>> liveMounts = liveMountService.createLiveMounts(liveMountObject, LiveMountServiceImplTestData.getCopy(), null);
        Assert.assertEquals(liveMounts.getKey().getUuid(), copyResourceSummary.getResourceId());
    }

    /**
     * test queryResource
     */
    @Test
    public void queryResource() {
        FileSetEntity fileSetEntity = LiveMountServiceImplTestData.getFileSetEntity();
        PowerMockito.when(resourceRestApi.queryResource(fileSetEntity.getUuid())).thenReturn(fileSetEntity);
        ResourceEntity resourceEntity = liveMountService.queryResource(fileSetEntity.getUuid());
        assert resourceEntity.getUuid().equals(fileSetEntity.getUuid());
    }

    /**
     * test queryCopyResourceSummary
     */
    @Test
    public void queryCopyResourceSummary() {
        CopyResourceSummary copyResourceSummaryParam = LiveMountServiceImplTestData.getCopyResourceSummary();
        PowerMockito.when(copyService.queryCopyResourceSummary(copyResourceSummaryParam.getResourceId()))
                .thenReturn(copyResourceSummaryParam);
        CopyResourceSummary copyResourceSummary =
                liveMountService.queryCopyResourceSummary(copyResourceSummaryParam.getResourceId());
        assert copyResourceSummaryParam.getResourceId().equals(copyResourceSummary.getResourceId());
    }

    /**
     * test getSourceCopyResourceId
     */
    @Test
    public void getSourceCopyResourceId() {
        LiveMountEntity liveMountEntity = LiveMountServiceImplTestData.getLiveMountEntity();
        String resourceId = liveMountService.getSourceCopyResourceId(liveMountEntity);
        assert resourceId.equals(liveMountEntity.getResourceId());
    }

    /**
     * test updateLiveMountPolicy
     */
    @Test
    public void updateLiveMountPolicy() {
        LiveMountEntity liveMountEntity = LiveMountServiceImplTestData.getLiveMountEntity();
        PowerMockito.when(liveMountEntityDao.update(any(), any())).thenReturn(1);
        TableInfoHelper.initTableInfo(
                new MapperBuilderAssistant(new MybatisConfiguration(), ""), LiveMountEntity.class);
        liveMountService.updateLiveMountPolicy(liveMountEntity, "1", "1");
        Mockito.verify(liveMountEntityDao, Mockito.times(1)).update(any(), any());
    }

    /**
     * test updateMountedCopyInfo
     */
    @Test
    public void updateMountedCopyInfo() {
        LiveMountEntity liveMountEntity = LiveMountServiceImplTestData.getLiveMountEntity();
        Copy mountCopy = LiveMountServiceImplTestData.getCopy();
        Copy sourceCopy = LiveMountServiceImplTestData.getCopy();
        PowerMockito.when(liveMountEntityDao.update(any(), any())).thenReturn(1);
        TableInfoHelper.initTableInfo(
                new MapperBuilderAssistant(new MybatisConfiguration(), ""), LiveMountEntity.class);
        liveMountService.updateMountedCopyInfo(liveMountEntity, mountCopy);
        Mockito.verify(liveMountEntityDao, Mockito.times(1)).update(any(), any());
    }

    /**
     * test updateLiveMountPolicyAndProperties
     */
    @Test
    public void updateLiveMountPolicyAndProperties() {
        LiveMountEntity liveMountEntity = LiveMountServiceImplTestData.getLiveMountEntity();
        PowerMockito.when(liveMountEntityDao.update(any(), any())).thenReturn(1);
        TableInfoHelper.initTableInfo(
                new MapperBuilderAssistant(new MybatisConfiguration(), ""), LiveMountEntity.class);
        liveMountService.updateLiveMountPolicyAndProperties(liveMountEntity, "1", "1", new JSONObject());
        Mockito.verify(liveMountEntityDao, Mockito.times(1)).update(any(), any());
    }

    /**
     * test deleteLiveMount
     */
    @Test
    public void deleteLiveMount() {
        PowerMockito.when(liveMountEntityDao.deleteById(anyString())).thenReturn(1);
        liveMountService.deleteLiveMount("1");
        Mockito.verify(liveMountEntityDao, Mockito.times(1)).deleteById(anyString());
    }

    /**
     * test updateLiveMountCopyIsNotNull
     */
    @Test
    public void updateLiveMountCopyIsNotNull() {
        LiveMountEntity liveMountEntity = LiveMountServiceImplTestData.getLiveMountEntity();
        LiveMountPolicyEntity liveMountPolicyEntity = LiveMountServiceImplTestData.getLiveMountPolicyEntity();
        Copy copy = LiveMountServiceImplTestData.getCopy();
        PowerMockito.when(copyRestApi.queryCopyByID(copy.getUuid())).thenReturn(copy);
        LiveMountFlowService liveMountFlowService = PowerMockito.mock(LiveMountFlowService.class);
        PowerMockito.when(
                        providerRegistry.findProvider(LiveMountFlowService.class, liveMountEntity.getResourceSubType()))
                .thenReturn(liveMountFlowService);
        PowerMockito.when(
            providerManager.findProviderOrDefault(LiveMountServiceProvider.class, liveMountEntity.getResourceSubType(),
                defaultLiveMountServiceProvider)).thenReturn(defaultLiveMountServiceProvider);
        liveMountService.updateLiveMount(liveMountEntity, liveMountPolicyEntity, copy.getUuid(), false);
        Mockito.verify(copyRestApi, Mockito.times(1)).queryCopyByID(copy.getUuid());
    }

    /**
     * test updateLiveMountCopyIsNotNull and resource id not same
     */
    @Test
    public void updateLiveMountCopyIsNotNullAndResourceIdNotSame() {
        expectedException.expect(LegoCheckedException.class);
        expectedException.expectMessage("resource of copy is inconsistent with the source resource");
        LiveMountEntity liveMountEntity = LiveMountServiceImplTestData.getLiveMountEntity();
        LiveMountPolicyEntity liveMountPolicyEntity = LiveMountServiceImplTestData.getLiveMountPolicyEntity();
        Copy copy = LiveMountServiceImplTestData.getCopy();
        copy.setResourceId(liveMountEntity.getResourceId() + "1");
        PowerMockito.when(copyRestApi.queryCopyByID(copy.getUuid())).thenReturn(copy);
        liveMountService.updateLiveMount(liveMountEntity, liveMountPolicyEntity, copy.getUuid(), false);
        Mockito.verify(copyRestApi, Mockito.times(1)).queryCopyByID(copy.getUuid());
    }

    /**
     * test updateLiveMountCopyIsNull and CopyDataSelectionPolicy is latest
     */
    @Test
    public void updateLiveMountCopyIsNullAndPolicyIsLatest() {
        LiveMountEntity liveMountEntity = LiveMountServiceImplTestData.getLiveMountEntity();
        LiveMountPolicyEntity liveMountPolicyEntity = LiveMountServiceImplTestData.getLiveMountPolicyEntity();
        liveMountPolicyEntity.setCopyDataSelectionPolicy(CopyDataSelection.LATEST.getName());
        Copy copy = LiveMountServiceImplTestData.getCopy();
        PowerMockito.when(copyRestApi.queryCopyByID(copy.getUuid())).thenReturn(copy);
        PowerMockito.when(
            providerManager.findProviderOrDefault(LiveMountServiceProvider.class, liveMountEntity.getResourceSubType(),
                defaultLiveMountServiceProvider)).thenReturn(defaultLiveMountServiceProvider);
        liveMountService.updateLiveMount(liveMountEntity, liveMountPolicyEntity, null, false);
        Mockito.verify(copyRestApi, Mockito.times(1)).queryCopyByID(copy.getUuid());
    }

    /**
     * test updateLiveMountCopyIsNull and CopyDataSelectionPolicy is late day
     */
    @Test
    public void updateLiveMountCopyIsNullAndPolicyIsLateDay() {
        LiveMountEntity liveMountEntity = LiveMountServiceImplTestData.getLiveMountEntity();
        LiveMountPolicyEntity liveMountPolicyEntity = LiveMountServiceImplTestData.getLiveMountPolicyEntity();
        liveMountPolicyEntity.setCopyDataSelectionPolicy(CopyDataSelection.LAST_DAY.getName());
        Copy copy = LiveMountServiceImplTestData.getCopy();
        PowerMockito.when(copyRestApi.queryCopyByID(copy.getUuid())).thenReturn(copy);
        PowerMockito.when(
            providerManager.findProviderOrDefault(LiveMountServiceProvider.class, liveMountEntity.getResourceSubType(),
                defaultLiveMountServiceProvider)).thenReturn(defaultLiveMountServiceProvider);
        liveMountService.updateLiveMount(liveMountEntity, liveMountPolicyEntity, null, false);
        Mockito.verify(copyRestApi, Mockito.times(1)).queryCopyByID(copy.getUuid());
    }

    /**
     * test updateLiveMountCopyIsNull and CopyDataSelectionPolicy is late day and Strict Is False
     */
    @Test
    public void updateLiveMountSourceCopyIsNullAndStrictIsTrue() {
        expectedException.expect(LegoCheckedException.class);
        expectedException.expectMessage("not found available copy");
        LiveMountEntity liveMountEntity = LiveMountServiceImplTestData.getLiveMountEntity();
        LiveMountPolicyEntity liveMountPolicyEntity = LiveMountServiceImplTestData.getLiveMountPolicyEntity();
        liveMountPolicyEntity.setCopyDataSelectionPolicy(CopyDataSelection.LAST_DAY.getName());
        Copy copy = LiveMountServiceImplTestData.getCopy();
        PowerMockito.when(copyRestApi.queryCopyByID(copy.getUuid())).thenReturn(copy);
        PowerMockito.when(
            providerManager.findProviderOrDefault(LiveMountServiceProvider.class, liveMountEntity.getResourceSubType(),
                defaultLiveMountServiceProvider)).thenReturn(defaultLiveMountServiceProvider);
        PowerMockito.doThrow(new LegoCheckedException(CommonErrorCode.OBJ_NOT_EXIST, "not found available copy"))
            .when(defaultLiveMountServiceProvider).isSourceCopyCanBeMounted(any(), anyBoolean());
        liveMountService.updateLiveMount(liveMountEntity, liveMountPolicyEntity, null, true);
        Mockito.verify(copyRestApi, Mockito.times(1)).queryCopyByID(copy.getUuid());
    }

    /**
     * test updateLiveMountCopyIsNull and CopyDataSelectionPolicy is late day and Strict Is False
     */
    @Test
    public void updateLiveMountSourceCopyIsNullAndStrictIsFalse() {
        LiveMountEntity liveMountEntity = LiveMountServiceImplTestData.getLiveMountEntity();
        LiveMountPolicyEntity liveMountPolicyEntity = LiveMountServiceImplTestData.getLiveMountPolicyEntity();
        liveMountPolicyEntity.setCopyDataSelectionPolicy(CopyDataSelection.LAST_DAY.getName());
        Copy copy = LiveMountServiceImplTestData.getCopy();
        PowerMockito.when(copyRestApi.queryCopyByID(copy.getUuid())).thenReturn(copy);
        PowerMockito.when(
            providerManager.findProviderOrDefault(LiveMountServiceProvider.class, liveMountEntity.getResourceSubType(),
                defaultLiveMountServiceProvider)).thenReturn(defaultLiveMountServiceProvider);
        liveMountService.updateLiveMount(liveMountEntity, liveMountPolicyEntity, null, false);
        Mockito.verify(copyRestApi, Mockito.times(1)).queryCopyByID(copy.getUuid());
    }

    /**
     * test update Live Mount Copy Is not Null
     */
    @Test
    public void updateLiveMountSourceCopyIsNotNull() {
        LiveMountEntity liveMountEntity = LiveMountServiceImplTestData.getLiveMountEntity();
        LiveMountPolicyEntity liveMountPolicyEntity = LiveMountServiceImplTestData.getLiveMountPolicyEntity();
        liveMountPolicyEntity.setCopyDataSelectionPolicy(CopyDataSelection.LAST_DAY.getName());
        Copy copy = LiveMountServiceImplTestData.getCopy();
        PowerMockito.when(copyRestApi.queryCopyByID(copy.getUuid())).thenReturn(copy);
        PowerMockito.when(copyRestApi.queryLatestBackupCopy(any(), any(), any())).thenReturn(copy);
        PowerMockito.when(
            providerManager.findProviderOrDefault(LiveMountServiceProvider.class, liveMountEntity.getResourceSubType(),
                defaultLiveMountServiceProvider)).thenReturn(defaultLiveMountServiceProvider);
        LiveMountFlowService liveMountFlowService = PowerMockito.mock(LiveMountFlowService.class);
        PowerMockito.when(
                        providerRegistry.findProvider(LiveMountFlowService.class, liveMountEntity.getResourceSubType()))
                .thenReturn(liveMountFlowService);
        liveMountService.updateLiveMount(liveMountEntity, liveMountPolicyEntity, null, false);
        Mockito.verify(copyRestApi, Mockito.times(1)).queryCopyByID(copy.getUuid());
    }

    /**
     * test execute Live Mount mounted Copy is null
     */
    @Test
    public void executeLiveMountMountedCopyIsNull() {
        LiveMountEntity liveMountEntity = LiveMountServiceImplTestData.getLiveMountEntity();
        LiveMountPolicyEntity liveMountPolicyEntity = LiveMountServiceImplTestData.getLiveMountPolicyEntity();
        Copy copy = LiveMountServiceImplTestData.getCopy();
        LiveMountFlowService liveMountFlowService = PowerMockito.mock(LiveMountFlowService.class);
        PowerMockito.when(
                        providerRegistry.findProvider(LiveMountFlowService.class, liveMountEntity.getResourceSubType()))
                .thenReturn(liveMountFlowService);
        liveMountService.executeLiveMountOnCopyChanged(liveMountEntity, liveMountPolicyEntity, copy, false);
        Mockito.verify(providerRegistry, Mockito.times(1)).findProvider(LiveMountFlowService.class, liveMountEntity.getResourceSubType());
    }

    /**
     * test execute Live Mount mounted Copy is not null and sourceCopy.getUuid()  mountedCopy.getParentCopyUuid not same
     */
    @Test
    public void executeLiveMountOnCopyChangedAndUuidNotSame() {
        LiveMountEntity liveMountEntity = LiveMountServiceImplTestData.getLiveMountEntity();
        LiveMountPolicyEntity liveMountPolicyEntity = LiveMountServiceImplTestData.getLiveMountPolicyEntity();
        Copy copy = LiveMountServiceImplTestData.getCopy();
        copy.setUuid("1");
        copy.setParentCopyUuid("2");
        PowerMockito.when(copyRestApi.queryCopyByID(liveMountEntity.getMountedCopyId(), false)).thenReturn(copy);
        LiveMountFlowService liveMountFlowService = PowerMockito.mock(LiveMountFlowService.class);
        PowerMockito.when(
                        providerRegistry.findProvider(LiveMountFlowService.class, liveMountEntity.getResourceSubType()))
                .thenReturn(liveMountFlowService);
        liveMountService.executeLiveMountOnCopyChanged(liveMountEntity, liveMountPolicyEntity, copy, false);
        Mockito.verify(copyRestApi, Mockito.times(1)).queryCopyByID(copy.getUuid());
    }

    /**
     * test execute Live Mount mounted Copy is not null and sourceCopy.getUuid() mountedCopy.getParentCopyUuid same
     */
    @Test
    public void executeLiveMountOnCopyChangedAndUuidSameFalse() {
        LiveMountEntity liveMountEntity = LiveMountServiceImplTestData.getLiveMountEntity();
        LiveMountPolicyEntity liveMountPolicyEntity = LiveMountServiceImplTestData.getLiveMountPolicyEntity();
        Copy copy = LiveMountServiceImplTestData.getCopy();
        copy.setParentCopyUuid(copy.getUuid());
        PowerMockito.when(copyRestApi.queryCopyByID(liveMountEntity.getMountedCopyId(), false)).thenReturn(copy);
        PowerMockito.when(
                        providerRegistry.findProvider(LiveMountFlowService.class, liveMountEntity.getResourceSubType()))
                .thenReturn(null);
        liveMountService.executeLiveMountOnCopyChanged(liveMountEntity, liveMountPolicyEntity, copy, false);
        Mockito.verify(copyRestApi, Mockito.times(1)).queryCopyByID(copy.getUuid());
    }

    /**
     * test cleanMountedCopyInfo
     */
    @Test
    public void cleanMountedCopyInfo() {
        LiveMountEntity liveMountEntity = LiveMountServiceImplTestData.getLiveMountEntity();
        TableInfoHelper.initTableInfo(
                new MapperBuilderAssistant(new MybatisConfiguration(), ""), LiveMountEntity.class);
        liveMountService.cleanMountedCopyInfo(liveMountEntity);
        Assert.assertNotNull(liveMountService);
    }

    /**
     * test executeLiveMount source copy is null
     */
    @Test
    public void executeLiveMountSourceCopyIsNull() {
        LiveMountEntity liveMountEntity = LiveMountServiceImplTestData.getLiveMountEntity();
        LiveMountPolicyEntity liveMountPolicyEntity = LiveMountServiceImplTestData.getLiveMountPolicyEntity();
        liveMountService.executeLiveMount(liveMountEntity, liveMountPolicyEntity, null, null, false);
        Assert.assertNotNull(liveMountService);
    }


    /**
     * 用例名称：构造任务payload时，如果获取queueScope为null，则payload为空<br/>
     * 前置条件：无<br/>
     * check点：资源类型对应的即时挂载任务不存在queueScope，则payload不构造值<br/>
     */
    @Test
    public void should_return_origin_params_when_add_job_queue_scope_if_get_null() {
        JSONObject params = new JSONObject();
        Copy copy = new Copy();
        Mockito.when(jobService.extractJobQueueScope(anyString(), anyString())).thenReturn(null);
        ReflectionTestUtils.invokeMethod(liveMountService, "addJobQueueScope", params, copy);
        assertThat(params).isEmpty();
    }

    /**
     * 用例名称：构造任务payload时，如果获取queueScope不为null，则payload为存在值<br/>
     * 前置条件：无<br/>
     * check点：资源类型对应的即时挂载任务存在queueScope，则payload构造对应的值<br/>
     */
    @Test
    public void should_return_root_uuid_when_add_job_queue_scope_if_get_queue_scope() {
        JSONObject params = new JSONObject();
        Copy copy = new Copy();
        copy.setResourceSubType("DWS-table");
        copy.setResourceId("123");
        Mockito.when(jobService.extractJobQueueScope(anyString(), anyString())).thenReturn("root_uuid");

        ProtectedResource resource = new ProtectedResource();
        resource.setUuid("123");
        resource.setRootUuid("456");
        Mockito.when(resourceService.getResourceById(anyString())).thenReturn(Optional.of(resource));
        ReflectionTestUtils.invokeMethod(liveMountService, "addJobQueueScope", params, copy);
        assertThat(params).containsKey("root_uuid");
    }

    /**
     * test executeLiveMount source copy is not null and debuts is false and status is available
     */
    @Test
    public void executeLiveMountDebutsIsFalseAndStatusIsAvailable() {
        LiveMountEntity liveMountEntity = LiveMountServiceImplTestData.getLiveMountEntity();
        LiveMountPolicyEntity liveMountPolicyEntity = LiveMountServiceImplTestData.getLiveMountPolicyEntity();
        Copy sourceCopy = LiveMountServiceImplTestData.getCopy();
        Copy mountCopy = LiveMountServiceImplTestData.getCopy();
        mountCopy.setResourceId("2");
        liveMountEntity.setStatus(LiveMountStatus.AVAILABLE.getName());
        PowerMockito.when(
                        providerRegistry.findProvider(LiveMountFlowService.class, liveMountEntity.getResourceSubType()))
                .thenReturn(null);
        liveMountService.executeLiveMount(liveMountEntity, liveMountPolicyEntity, sourceCopy, mountCopy, false);
        Mockito.verify(providerRegistry, Mockito.times(1)).findProvider(LiveMountFlowService.class,
                liveMountEntity.getResourceSubType());
    }

    /**
     * test executeLiveMount source copy is not null and debuts is false and status is mounting
     */
    @Test
    public void executeLiveMountDebutsIsFalseAndStatusIsMounting() {
        expectedException.expect(LegoCheckedException.class);
        expectedException.expectMessage("the live mount status is:mounting, can't update");
        LiveMountEntity liveMountEntity = LiveMountServiceImplTestData.getLiveMountEntity();
        LiveMountPolicyEntity liveMountPolicyEntity = LiveMountServiceImplTestData.getLiveMountPolicyEntity();
        Copy sourceCopy = LiveMountServiceImplTestData.getCopy();
        Copy mountCopy = LiveMountServiceImplTestData.getCopy();
        mountCopy.setResourceId("2");
        liveMountEntity.setStatus(LiveMountStatus.MOUNTING.getName());
        liveMountService.executeLiveMount(liveMountEntity, liveMountPolicyEntity, sourceCopy, mountCopy, false);
    }

    /**
     * test executeLiveMount source copy is not null and debuts is true
     */
    @Test
    public void executeLiveMountDebutsIsTrue() {
        LiveMountEntity liveMountEntity = LiveMountServiceImplTestData.getLiveMountEntity();
        LiveMountPolicyEntity liveMountPolicyEntity = LiveMountServiceImplTestData.getLiveMountPolicyEntity();
        Copy sourceCopy = LiveMountServiceImplTestData.getCopy();
        Copy mountCopy = LiveMountServiceImplTestData.getCopy();
        mountCopy.setResourceId("2");
        LiveMountFlowService liveMountFlowService = PowerMockito.mock(LiveMountFlowService.class);
        PowerMockito.when(
                        providerRegistry.findProvider(LiveMountFlowService.class, liveMountEntity.getResourceSubType()))
                .thenReturn(liveMountFlowService);
        liveMountService.executeLiveMount(liveMountEntity, liveMountPolicyEntity, sourceCopy, mountCopy, true);
        Mockito.verify(providerRegistry, Mockito.times(1)).findProvider(LiveMountFlowService.class,
                liveMountEntity.getResourceSubType());
    }

    /**
     * test queryValidCopy and resource not same
     */
    @Test
    public void queryValidCopyAndResourceNotSame() {
        expectedException.expect(LegoCheckedException.class);
        expectedException.expectMessage("resource of copy is inconsistent with the source resource");
        Copy copy = LiveMountServiceImplTestData.getCopy();
        PowerMockito.when(copyRestApi.queryCopyByID(copy.getUuid())).thenReturn(copy);
        liveMountService.queryValidCopy(copy.getUuid() + "1", copy.getUuid());
    }

    /**
     * test queryValidCopy
     */
    @Test
    public void queryValidCopy() {
        Copy copy = LiveMountServiceImplTestData.getCopy();
        PowerMockito.when(copyRestApi.queryCopyByID(copy.getUuid())).thenReturn(copy);
        liveMountService.queryValidCopy(copy.getUuid(), copy.getUuid());
        Mockito.verify(copyRestApi, Mockito.times(1)).queryCopyByID(copy.getUuid());
    }

    /**
     * test queryCopyResourceSummary
     */
    @Test
    public void queryLiveMountEntities() {
        LiveMountModel liveMountModel = LiveMountServiceImplTestData.getLiveMountModel();
        BasePage<Object> basePageParam = new BasePage<>();
        basePageParam.setItems(Collections.singletonList(liveMountModel));
        PowerMockito.when(pageQueryService.pageQuery(any(), any(), any(), any(), any())).thenReturn(basePageParam);
        BasePage<LiveMountModel> obj =
                liveMountService.queryLiveMountEntities(0, 20, "", Collections.singletonList("-created_time"));
        if (obj != null) {
            LiveMountModel liveMountModelObj = obj.getItems().get(0);
            if (liveMountModelObj != null) {
                assert liveMountModel.getId().equals(liveMountModelObj.getId());
            }
        }
    }

    /**
     * test queryLiveMountEntitiesByPolicyId and policy id is null
     */
    @Test
    public void queryLiveMountEntitiesByPolicyIdIsNull() {
        Assert.assertThrows("the policy id is null", LegoCheckedException.class,
                () -> liveMountService.queryLiveMountEntitiesByPolicyId(null));
    }

    /**
     * test queryLiveMountEntitiesByPolicyId
     */
    @Test
    public void queryLiveMountEntitiesByPolicyId() {
        PowerMockito.when(liveMountEntityDao.selectList(any())).thenReturn(new ArrayList<>());
        liveMountService.queryLiveMountEntitiesByPolicyId("1");
    }

    /**
     * test selectLiveMountEntityById and entity is null
     */
    @Test
    public void selectLiveMountEntityByIdAndEntityIsNull() {
        LiveMountEntity param = LiveMountServiceImplTestData.getLiveMountEntity();
        expectedException.expect(LegoCheckedException.class);
        expectedException.expectMessage("live mount is not exist. live mount id is " + param.getId());
        liveMountService.selectLiveMountEntityById(param.getId());
    }

    /**
     * test selectLiveMountEntityById
     */
    @Test
    public void selectLiveMountEntityById() {
        LiveMountEntity param = LiveMountServiceImplTestData.getLiveMountEntity();
        PowerMockito.when(liveMountEntityDao.selectById(any())).thenReturn(param);
        LiveMountEntity entity = liveMountService.selectLiveMountEntityById(param.getId());
        assert param.getResourceId().equals(entity.getResourceId());
    }

    /**
     * test modifyLiveMount(FAIL)
     */
    @Test
    public void modifyLiveMount() {
        LiveMountEntity param = LiveMountServiceImplTestData.getLiveMountEntity();
        JSONObject jsonObject = new JSONObject();
        JSONObject performanceJsonOject = new JSONObject();
        jsonObject.set("performance", performanceJsonOject);
        param.setParameters(jsonObject.toString());
        PowerMockito.when(liveMountEntityDao.selectById(any())).thenReturn(param);
        LiveMountPolicyEntity liveMountPolicyEntity = LiveMountServiceImplTestData.getLiveMountPolicyEntity();
        PowerMockito.when(liveMountPolicyEntityDao.selectPolicy(any())).thenReturn(liveMountPolicyEntity);
        CopyResourceSummary copyResourceSummary = new CopyResourceSummary();
        copyResourceSummary.setResourceSubType(ResourceSubTypeEnum.HYPER_V.getType());
        PowerMockito.when(copyService.queryCopyResourceSummary(any())).thenReturn(copyResourceSummary);
        TableInfoHelper.initTableInfo(
                new MapperBuilderAssistant(new MybatisConfiguration(), ""), LiveMountEntity.class);
        liveMountService.modifyLiveMount("1", LiveMountServiceImplTestData.getLiveMountParam());
    }

    /**
     * test unmountLiveMount and liveMountEntity is null
     */
    @Test
    public void unmountLiveMountAndLiveMountEntityIsNull() {
        String liveMountId = LiveMountServiceImplTestData.getLiveMountEntity().getId();
        liveMountService.unmountLiveMount(liveMountId, true, false, new UnmountExtendParam());
        Assert.assertNotNull(liveMountService);
    }

    /**
     * test unmountLiveMount and mountedCopyId is null
     */
    @Test
    public void unmountLiveMountAndMountedCopyIdIsNull() {
        LiveMountEntity liveMountEntity = LiveMountServiceImplTestData.getLiveMountEntity();
        liveMountEntity.setMountedCopyId(null);
        PowerMockito.when(liveMountEntityDao.selectById(ArgumentMatchers.anyString())).thenReturn(liveMountEntity);
        PowerMockito.when(copyRestApi.queryCopyByID(ArgumentMatchers.anyString()))
                .thenReturn(LiveMountServiceImplTestData.getCloneCopy());
        PowerMockito.when(scheduleRestApi.createImmediateSchedule(any(), any(), any())).thenReturn(null);
        PowerMockito.when(
                        providerRegistry.findProvider(LiveMountFlowService.class, liveMountEntity.getResourceSubType()))
                .thenReturn(null);
        String liveMountId = LiveMountServiceImplTestData.getLiveMountEntity().getId();
        liveMountService.unmountLiveMount(liveMountId, true, false,new UnmountExtendParam());
        Mockito.verify(copyRestApi, Mockito.times(1)).queryCopyByID(anyString());
    }

    /**
     * test unmountLiveMount
     */
    @Test
    public void unmountLiveMount() {
        PowerMockito.when(liveMountEntityDao.selectById(ArgumentMatchers.anyString()))
                .thenReturn(LiveMountServiceImplTestData.getLiveMountEntity());
        PowerMockito.when(copyRestApi.queryCopyByID(ArgumentMatchers.anyString()))
                .thenReturn(LiveMountServiceImplTestData.getCloneCopy());
        PowerMockito.when(scheduleRestApi.createImmediateSchedule(any(), any(), any())).thenReturn(null);
        LiveMountEntity liveMountEntity = LiveMountServiceImplTestData.getLiveMountEntity();
        String liveMountId = liveMountEntity.getId();
        PowerMockito.when(
                        providerRegistry.findProvider(LiveMountFlowService.class, liveMountEntity.getResourceSubType()))
                .thenReturn(null);
        liveMountService.unmountLiveMount(liveMountId, true, false,new UnmountExtendParam());
        Mockito.verify(liveMountEntityDao, Mockito.times(1)).selectById(ArgumentMatchers.anyString());
    }

    /**
     * test checkLiveMountStatus and status is mount_failed
     */
    @Test
    public void checkLiveMountStatusWhenStatusIsMountFailed() {
        Assert.assertThrows("the live mount status is:mount_failed, can't migrate", LegoCheckedException.class,
                () -> liveMountService.checkLiveMountStatus(LiveMountStatus.MOUNT_FAILED.getName(), LiveMountOperateType.MIGRATE));
    }

    /**
     * test checkLiveMountStatus
     */
    @Test
    public void checkLiveMountStatus() {
        liveMountService.checkLiveMountStatus(LiveMountStatus.AVAILABLE.getName(), LiveMountOperateType.ACTIVATE);
    }

    /**
     * initialLiveMountSchedule AllEmptyPolicyId
     */
    @Test
    public void initialLiveMountScheduleAllEmptyPolicyId() {
        LiveMountEntity liveMountEntity = new LiveMountEntity();
        Optional<String> liveMountSchedule = liveMountService.initialLiveMountSchedule(liveMountEntity, null);
        Assert.assertEquals(liveMountSchedule, Optional.empty());
    }

    /**
     * initialLiveMountSchedule liveMountEntity PolicyId is not null and schedule id is not null
     */
    @Test
    public void initialLiveMountScheduleEntityPolicyIdIsNotNull() {
        LiveMountEntity param = LiveMountServiceImplTestData.getLiveMountEntity();
        LiveMountPolicyEntity liveMountPolicyEntity = LiveMountServiceImplTestData.getLiveMountPolicyEntity();
        PowerMockito.when(liveMountPolicyEntityDao.selectPolicy(any())).thenReturn(liveMountPolicyEntity);
        ScheduleResponse scheduleResponse = LiveMountServiceImplTestData.getScheduleResponse();
        PowerMockito.when(scheduleRestApi.createIntervalSchedule(any(), any(), any(), any()))
                .thenReturn(scheduleResponse);
        Optional<String> liveMountSchedule = liveMountService.initialLiveMountSchedule(param, null);
        Assert.assertEquals(liveMountSchedule.get(), liveMountPolicyEntity.getPolicyId());
    }

    /**
     * initialLiveMountSchedule liveMountEntity PolicyId is not null and schedule id is null
     */
    @Test
    public void initialLiveMountSchedulePolicyIdNotNullAndScheIdIsNull() {
        LiveMountEntity param = LiveMountServiceImplTestData.getLiveMountEntity();
        param.setScheduleId(null);
        LiveMountPolicyEntity liveMountPolicyEntity = LiveMountServiceImplTestData.getLiveMountPolicyEntity();
        PowerMockito.when(liveMountPolicyEntityDao.selectPolicy(any())).thenReturn(liveMountPolicyEntity);
        ScheduleResponse scheduleResponse = LiveMountServiceImplTestData.getScheduleResponse();
        PowerMockito.when(scheduleRestApi.createIntervalSchedule(any(), any(), any(), any()))
                .thenReturn(scheduleResponse);
        Optional<String> liveMountSchedule = liveMountService.initialLiveMountSchedule(param, null);
        Assert.assertEquals(liveMountSchedule.get(), liveMountPolicyEntity.getPolicyId());
    }

    /**
     * initialLiveMountSchedule
     * test when scheduleName equals period_schedule
     */
    @Test
    public void initialLiveMountScheduleSchedulePolicyIsNotNull() {
        LiveMountEntity param = LiveMountServiceImplTestData.getLiveMountEntity();
        LiveMountPolicyEntity liveMountPolicyEntity = LiveMountServiceImplTestData.getLiveMountPolicyEntity();
        PowerMockito.when(liveMountPolicyEntityDao.selectPolicy(any())).thenReturn(liveMountPolicyEntity);
        ScheduleResponse scheduleResponse = LiveMountServiceImplTestData.getScheduleResponse();
        PowerMockito.when(scheduleRestApi.createIntervalSchedule(any(), any(), any(), any()))
                .thenReturn(scheduleResponse);
        Optional<String> liveMountSchedule = liveMountService.initialLiveMountSchedule(param, "1");
        Assert.assertEquals(liveMountSchedule.get(), liveMountPolicyEntity.getPolicyId());
    }

    /**
     * initialLiveMountSchedule
     * test when scheduleName not equals period_schedule
     */
    @Test
    public void initialLiveMountScheduleSchedulePolicyIsNull() {
        LiveMountEntity param = LiveMountServiceImplTestData.getLiveMountEntity();
        LiveMountPolicyEntity liveMountPolicyEntity = LiveMountServiceImplTestData.getLiveMountPolicyEntity();
        liveMountPolicyEntity.setSchedulePolicy(null);
        PowerMockito.when(liveMountPolicyEntityDao.selectPolicy(any())).thenReturn(liveMountPolicyEntity);
        ScheduleResponse scheduleResponse = LiveMountServiceImplTestData.getScheduleResponse();
        PowerMockito.when(scheduleRestApi.createIntervalSchedule(any(), any(), any(), any()))
                .thenReturn(scheduleResponse);
        Optional<String> liveMountSchedule = liveMountService.initialLiveMountSchedule(param, "1");
        Assert.assertEquals(liveMountSchedule.get(), liveMountPolicyEntity.getPolicyId());
    }

    /**
     * test initialAndUpdateLiveMountSchedule And ScheduleId Is Same
     */
    @Test
    public void initialAndUpdateLiveMountScheduleAndScheduleIdIsSame() {
        LiveMountEntity param = LiveMountServiceImplTestData.getLiveMountEntity();
        param.setScheduleId(param.getScheduleId() + "1");
        LiveMountPolicyEntity liveMountPolicyEntity = LiveMountServiceImplTestData.getLiveMountPolicyEntity();
        PowerMockito.when(liveMountPolicyEntityDao.selectPolicy(any())).thenReturn(liveMountPolicyEntity);
        ScheduleResponse scheduleResponse = LiveMountServiceImplTestData.getScheduleResponse();
        PowerMockito.when(scheduleRestApi.createIntervalSchedule(any(), any(), any(), any()))
                .thenReturn(scheduleResponse);
        TableInfoHelper.initTableInfo(
                new MapperBuilderAssistant(new MybatisConfiguration(), ""), LiveMountEntity.class);
        liveMountService.initialAndUpdateLiveMountSchedule(param, "1");
        Mockito.verify(liveMountPolicyEntityDao, Mockito.times(1)).selectPolicy(any());
    }

    /**
     * test initialAndUpdateLiveMountSchedule
     */
    @Test
    public void initialAndUpdateLiveMountSchedule() {
        LiveMountEntity param = LiveMountServiceImplTestData.getLiveMountEntity();
        LiveMountPolicyEntity liveMountPolicyEntity = LiveMountServiceImplTestData.getLiveMountPolicyEntity();
        PowerMockito.when(liveMountPolicyEntityDao.selectPolicy(any())).thenReturn(liveMountPolicyEntity);
        ScheduleResponse scheduleResponse = LiveMountServiceImplTestData.getScheduleResponse();
        PowerMockito.when(scheduleRestApi.createIntervalSchedule(any(), any(), any(), any()))
                .thenReturn(scheduleResponse);
        TableInfoHelper.initTableInfo(
                new MapperBuilderAssistant(new MybatisConfiguration(), ""), LiveMountEntity.class);
        liveMountService.initialAndUpdateLiveMountSchedule(param, "1");
        Mockito.verify(liveMountPolicyEntityDao, Mockito.times(1)).selectPolicy(any());
    }

    /**
     * test updateLiveMountSchedule
     */
    @Test
    public void updateLiveMountSchedule() {
        LiveMountEntity param = LiveMountServiceImplTestData.getLiveMountEntity();
        TableInfoHelper.initTableInfo(
                new MapperBuilderAssistant(new MybatisConfiguration(), ""), LiveMountEntity.class);
        liveMountService.updateLiveMountSchedule(param, "1");
        Assert.assertNotNull(liveMountService);
    }

    @Test
    public void updateLiveMountParameters() {
        LiveMountEntity param = LiveMountServiceImplTestData.getLiveMountEntity();
        TableInfoHelper.initTableInfo(
            new MapperBuilderAssistant(new MybatisConfiguration(), ""), LiveMountEntity.class);
        liveMountService.updateLiveMountParameters(param,"param");
        Assert.assertNotNull(liveMountService);
    }

    /**
     * test updateLiveMountStatus
     */
    @Test
    public void updateLiveMountStatus() {
        LiveMountEntity param = LiveMountServiceImplTestData.getLiveMountEntity();
        TableInfoHelper.initTableInfo(
                new MapperBuilderAssistant(new MybatisConfiguration(), ""), LiveMountEntity.class);
        liveMountService.updateLiveMountStatus(param, LiveMountStatus.AVAILABLE);
        Assert.assertNotNull(liveMountService);
    }

    /**
     * test updateLiveMountMountedResource
     */
    @Test
    public void updateLiveMountMountedResource() {
        LiveMountEntity param = LiveMountServiceImplTestData.getLiveMountEntity();
        TableInfoHelper.initTableInfo(
                new MapperBuilderAssistant(new MybatisConfiguration(), ""), LiveMountEntity.class);
        liveMountService.updateLiveMountMountedResource(param, "111");
        Assert.assertNotNull(liveMountService);
    }

    /**
     * test activateLiveMount
     */
    @Test
    public void activateLiveMount() {
        LiveMountEntity param = LiveMountServiceImplTestData.getLiveMountEntity();
        param.setEnableStatus(LiveMountEnableStatus.DISABLED.getName());
        TableInfoHelper.initTableInfo(
                new MapperBuilderAssistant(new MybatisConfiguration(), ""), LiveMountEntity.class);
        PowerMockito.when(liveMountEntityDao.selectById(any())).thenReturn(param);
        liveMountService.activateLiveMount(param.getId());
        Mockito.verify(liveMountEntityDao, Mockito.times(1)).selectById(any());
    }

    /**
     * test deactivateLiveMount LiveMountEntity is null
     */
    @Test
    public void deactivateLiveMountEntityIsNull() {
        TableInfoHelper.initTableInfo(
                new MapperBuilderAssistant(new MybatisConfiguration(), ""), LiveMountEntity.class);
        Assert.assertThrows("live mount is not exist.", LegoCheckedException.class, () ->
                liveMountService.deactivateLiveMount(LiveMountServiceImplTestData.getLiveMountEntity().getId()));
    }

    /**
     * test deactivateLiveMount status can not allow change live mount
     */
    @Test
    public void deactivateLiveMountNotAllowLiveMount() {
        expectedException.expect(LegoCheckedException.class);
        expectedException.expectMessage("Not allow change live mount status when it was migrating");
        LiveMountEntity param = LiveMountServiceImplTestData.getLiveMountEntity();
        param.setStatus(LiveMountStatus.MIGRATING.getName());
        TableInfoHelper.initTableInfo(
                new MapperBuilderAssistant(new MybatisConfiguration(), ""), LiveMountEntity.class);
        PowerMockito.when(liveMountEntityDao.selectById(any())).thenReturn(param);
        liveMountService.deactivateLiveMount(param.getId());
    }

    /**
     * test deactivateLiveMount
     */
    @Test
    public void deactivateLiveMountStatusIsDisabled() {
        expectedException.expect(LegoCheckedException.class);
        expectedException.expectMessage("The live mount is disabled.");
        LiveMountEntity param = LiveMountServiceImplTestData.getLiveMountEntity();
        param.setEnableStatus(LiveMountEnableStatus.DISABLED.getName());
        TableInfoHelper.initTableInfo(
                new MapperBuilderAssistant(new MybatisConfiguration(), ""), LiveMountEntity.class);
        PowerMockito.when(liveMountEntityDao.selectById(any())).thenReturn(param);
        liveMountService.deactivateLiveMount(param.getId());
    }

    /**
     * test deactivateLiveMount
     */
    @Test
    public void deactivateLiveMount() {
        LiveMountEntity param = LiveMountServiceImplTestData.getLiveMountEntity();
        param.setEnableStatus(LiveMountEnableStatus.ACTIVATED.getName());
        TableInfoHelper.initTableInfo(
                new MapperBuilderAssistant(new MybatisConfiguration(), ""), LiveMountEntity.class);
        PowerMockito.when(liveMountEntityDao.selectById(any())).thenReturn(param);
        liveMountService.deactivateLiveMount(param.getId());
    }

    /**
     * test checkHasActive status not equals activated and strict false
     */
    @Test
    public void checkHasActiveNotEqualsAndStrictFalse() {
        Assert.assertFalse(liveMountService.checkHasActive(LiveMountEnableStatus.DISABLED.getName(), false));
    }

    /**
     * test checkHasActive status not equals activated and strict true
     */
    @Test
    public void checkHasActiveNotEqualsAndStrictTrue() {
        expectedException.expect(LegoCheckedException.class);
        expectedException.expectMessage("When livemount is disabled, it cannot be modified or updated.");
        liveMountService.checkHasActive(LiveMountEnableStatus.DISABLED.getName(), true);
    }

    /**
     * test checkHasActive status equals activated and strict true
     */
    @Test
    public void checkHasActiveEqualsAndStrictTrue() {
        Assert.assertTrue(liveMountService.checkHasActive(LiveMountEnableStatus.ACTIVATED.getName(), true));
    }

    /**
     * test checkHasActive status equals activated and strict false
     */
    @Test
    public void checkHasActiveEqualsAndStrictFalse() {
        Assert.assertFalse(liveMountService.checkHasActive(LiveMountEnableStatus.ACTIVATED.getName(), false));
    }

    /**
     * test checkTargetEnvironmentStatus(FAIL)
     *
     * @throws Exception Exception
     */
    @Test
    public void checkTargetEnvironmentStatus() throws Exception {
        environmentBasePage =
                LiveMountServiceImplTestData.getEnvironmentBasePage(LiveMountServiceImplTestData.getEnvironment());
        successEnvironmentRestApi();
        LiveMountInterceptorProvider liveMountInterceptorProvider = PowerMockito.mock(
                LiveMountInterceptorProvider.class);
        PowerMockito.when(providerManager.findProvider(any(), any(), any())).thenReturn(liveMountInterceptorProvider);
        PowerMockito.when(liveMountInterceptorProvider.isRefreshTargetEnvironment()).thenReturn(true);
        LiveMountEntity param = LiveMountServiceImplTestData.getLiveMountEntity();
        FileSetEntity fileSetEntity = LiveMountServiceImplTestData.getFileSetEntity();
        PowerMockito.when(resourceRestApi.queryResource(any())).thenReturn(fileSetEntity);
        liveMountService.checkTargetEnvironmentStatus(param);
        Mockito.verify(resourceRestApi, Mockito.times(1)).queryResource(any());
    }

    /**
     * successEnvironmentRestApi
     *
     * @throws Exception Exception
     */
    private void successEnvironmentRestApi() throws Exception {
        EnvironmentRestApi environmentApi =
                new EnvironmentRestApi() {
                    @Override
                    public BasePage<Environment> queryEnvironment(int page, int size, String cd, List<String> os) {
                        return environmentBasePage;
                    }
                };
        Class<?> clazz = liveMountService.getClass();
        Field environmentRestApiField = clazz.getDeclaredField("environmentRestApi");
        environmentRestApiField.setAccessible(true);
        environmentRestApiField.set(liveMountService, environmentApi);
    }

    /**
     * 查询挂载目标资源不存在
     */
    @Test
    public void test_query_target_resource_return_is_null() {
        List<String> resourceEntities = new ArrayList<>();
        resourceEntities.add("34343434");
        PowerMockito.when(resourceRestApi.queryResource(ArgumentMatchers.anyString())).thenReturn(null);
        List<ResourceEntity> results = liveMountService.queryResources(
                resourceEntities, ResourceSubTypeEnum.VM_BACKUP_AGENT.getType());
        Assertions.assertEquals(Collections.emptyList(), results);
    }

    /**
     * 取消执行挂载
     */
    @Test
    public void test_cancel_live_mount_success() {
        LiveMountEntity param = LiveMountServiceImplTestData.getLiveMountEntity();
        PowerMockito.when(liveMountEntityDao.selectById(any())).thenReturn(param);
        TableInfoHelper.initTableInfo(
                new MapperBuilderAssistant(new MybatisConfiguration(), ""), LiveMountEntity.class);
        liveMountService.cancelLiveMount(param.getId());
        Mockito.verify(liveMountEntityDao, Mockito.times(1)).selectById(any());
    }

    /**
     * createLiveMount but TargetResourceUuidList Size More Than Eight
     */
    @Test
    public void createLiveMountTargetResourceUuidListSizeMoreThanEight() {
        expectedException.expect(LegoCheckedException.class);
        expectedException.expectMessage("number of targets is exceeded the limit(8).");
        LiveMountObject liveMountObject = getLiveMountObject();
        liveMountObject.setTargetResourceUuidList(Arrays.asList("uuid1", "uuid2",
                "uuid3", "uuid4", "uuid5", "uuid6", "uuid7", "uuid8", "uuid9"));
        liveMountObject.setCopyId("12");
        CopiesEntity copiesEntity = CopiesEntity.builder().build();
        copiesEntity.setResourceSubType("aa");
        copiesEntity.setSlaProperties(
            "{\"policy_list\":[{\"ext_parameters\":{\"storage_info\": {\"storage_type\": \"storage_unit\", \"storage_id\": \"80ce06b8-5"
                + "0e0-4695-916e-d444aa7d347a\"}}}]}"
        );
        StorageUnitVo storageUnitVo = new StorageUnitVo();
        storageUnitVo.setDeviceType("OceanProtectX");

        when(copyMapper.selectById(liveMountObject.getCopyId())).thenReturn(copiesEntity);
        when(storageUnitService.getStorageUnitById(anyString())).thenReturn(Optional.of(storageUnitVo));
        when(deployTypeService.isCyberEngine()).thenReturn(true);
        liveMountService.createLiveMountCommon(liveMountObject);
    }

    /**
     * createLiveMount but FileSystemName is Chinese
     */
    @Test
    public void createLiveMountChineseFileSystemName() {
        expectedException.expect(LegoCheckedException.class);
        expectedException.expectMessage("Nas filesystem name is not right!");
        LiveMountObject liveMountObject = getLiveMountObject();
        liveMountObject.setTargetResourceUuidList(
                Arrays.asList("uuid1", "uuid2", "uuid3", "uuid4", "uuid5", "uuid6", "uuid7", "uuid8"));
        liveMountObject.setCopyId("12");
        LiveMountFileSystemShareInfo liveMountFileSystemShareInfo = new LiveMountFileSystemShareInfo();
        liveMountFileSystemShareInfo.setFileSystemName("中文");
        liveMountObject.setFileSystemShareInfoList(Lists.newArrayList(liveMountFileSystemShareInfo));
        CopiesEntity copiesEntity = CopiesEntity.builder().build();
        copiesEntity.setResourceSubType("aa");
        copiesEntity.setSlaProperties(
            "{\"policy_list\":[{\"ext_parameters\":{\"storage_info\": {\"storage_type\": \"storage_unit\", \"storage_id\": \"80ce06b8-5"
                + "0e0-4695-916e-d444aa7d347a\"}}}]}"
            );
        StorageUnitVo storageUnitVo = new StorageUnitVo();
        storageUnitVo.setDeviceType("OceanProtectX");

        when(copyMapper.selectById(liveMountObject.getCopyId())).thenReturn(copiesEntity);
        when(storageUnitService.getStorageUnitById(anyString())).thenReturn(Optional.of(storageUnitVo));
        when(deployTypeService.isCyberEngine()).thenReturn(true);
        liveMountService.createLiveMountCommon(liveMountObject);
    }

    /**
     * createLiveMount but FileSystemKeepTime is more than ninety-six
     */
    @Test
    public void createLiveMountFileSystemKeepTimeMoreThanNinetySix() {
        expectedException.expect(LegoCheckedException.class);
        expectedException.expectMessage("The fileSystemKeepTime must range from 1 to 96.");
        LiveMountObject liveMountObject = getLiveMountObject();
        liveMountObject.setTargetResourceUuidList(
                Arrays.asList("uuid1", "uuid2", "uuid3", "uuid4", "uuid5", "uuid6", "uuid7", "uuid8"));
        liveMountObject.setCopyId("12");
        LiveMountFileSystemShareInfo liveMountFileSystemShareInfo = new LiveMountFileSystemShareInfo();
        liveMountFileSystemShareInfo.setFileSystemName("FileSystemName");
        liveMountObject.setFileSystemShareInfoList(Lists.newArrayList(liveMountFileSystemShareInfo));
        CopiesEntity copiesEntity = CopiesEntity.builder().build();
        copiesEntity.setResourceSubType("aa");
        copiesEntity.setSlaProperties(
            "{\"policy_list\":[{\"ext_parameters\":{\"storage_info\": {\"storage_type\": \"storage_unit\", \"storage_id\": \"80ce06b8-5"
                + "0e0-4695-916e-d444aa7d347a\"}}}]}"
        );
        StorageUnitVo storageUnitVo = new StorageUnitVo();
        storageUnitVo.setDeviceType("OceanProtectX");

        when(copyMapper.selectById(liveMountObject.getCopyId())).thenReturn(copiesEntity);
        when(storageUnitService.getStorageUnitById(anyString())).thenReturn(Optional.of(storageUnitVo));
        when(deployTypeService.isCyberEngine()).thenReturn(true);
        Map<String, Object> parameters = new HashMap<>();
        Map<String, Object> params = new HashMap<>();
        params.put("fileSystemKeepTime", 97);
        parameters.put("performance", params);
        liveMountObject.setParameters(parameters);
        liveMountService.createLiveMountCommon(liveMountObject);
    }

    /**
     * createLiveMount but clientName is error
     */
    @Test
    public void createLiveMountErrorClientName() {
        expectedException.expect(LegoCheckedException.class);
        expectedException.expectMessage("The client name is error.");
        LiveMountObject liveMountObject = getLiveMountObject();
        liveMountObject.setTargetResourceUuidList(
                Arrays.asList("uuid1", "uuid2", "uuid3", "uuid4", "uuid5", "uuid6", "uuid7", "uuid8"));
        LiveMountFileSystemShareInfo liveMountFileSystemShareInfo = new LiveMountFileSystemShareInfo();
        liveMountFileSystemShareInfo.setFileSystemName("FileSystemName");
        liveMountFileSystemShareInfo.setAdvanceParams(ImmutableMap.of("clientName", "-"));
        liveMountObject.setCopyId("12");
        liveMountObject.setFileSystemShareInfoList(Lists.newArrayList(liveMountFileSystemShareInfo));
        CopiesEntity copiesEntity = CopiesEntity.builder().build();
        copiesEntity.setResourceSubType("aa");
        copiesEntity.setSlaProperties(
            "{\"policy_list\":[{\"ext_parameters\":{\"storage_info\": {\"storage_type\": \"storage_unit\", \"storage_id\": \"80ce06b8-5"
                + "0e0-4695-916e-d444aa7d347a\"}}}]}"
        );
        StorageUnitVo storageUnitVo = new StorageUnitVo();
        storageUnitVo.setDeviceType("OceanProtectX");

        when(copyMapper.selectById(liveMountObject.getCopyId())).thenReturn(copiesEntity);
        when(storageUnitService.getStorageUnitById(anyString())).thenReturn(Optional.of(storageUnitVo));
        when(deployTypeService.isCyberEngine()).thenReturn(true);
        when(deployTypeService.isCyberEngine()).thenReturn(true);
        Map<String, Object> parameters = new HashMap<>();
        Map<String, Object> params = new HashMap<>();
        params.put("fileSystemKeepTime", 95);
        parameters.put("performance", params);
        liveMountObject.setParameters(parameters);
        liveMountService.createLiveMountCommon(liveMountObject);
    }

    /**
     * createLiveMount but ShareName is Chinese
     */
    @Test
    public void createLiveMountChineseShareName() {
        expectedException.expect(LegoCheckedException.class);
        expectedException.expectMessage("Nas filesystem name is not right!");
        LiveMountObject liveMountObject = getLiveMountObject();
        liveMountObject.setTargetResourceUuidList(
                Arrays.asList("uuid1", "uuid2", "uuid3", "uuid4", "uuid5", "uuid6", "uuid7", "uuid8"));
        LiveMountFileSystemShareInfo liveMountFileSystemShareInfo = new LiveMountFileSystemShareInfo();
        liveMountFileSystemShareInfo.setFileSystemName("FileSystemName");
        liveMountFileSystemShareInfo.setAdvanceParams(ImmutableMap.of("shareName", "中文"));
        liveMountObject.setFileSystemShareInfoList(Lists.newArrayList(liveMountFileSystemShareInfo));
        liveMountObject.setCopyId("12");
        CopiesEntity copiesEntity = CopiesEntity.builder().build();
        copiesEntity.setResourceSubType("aa");
        copiesEntity.setSlaProperties(
            "{\"policy_list\":[{\"ext_parameters\":{\"storage_info\": {\"storage_type\": \"storage_unit\", \"storage_id\": \"80ce06b8-5"
                + "0e0-4695-916e-d444aa7d347a\"}}}]}"
        );
        StorageUnitVo storageUnitVo = new StorageUnitVo();
        storageUnitVo.setDeviceType("OceanProtectX");

        when(copyMapper.selectById(liveMountObject.getCopyId())).thenReturn(copiesEntity);
        when(storageUnitService.getStorageUnitById(anyString())).thenReturn(Optional.of(storageUnitVo));
        when(deployTypeService.isCyberEngine()).thenReturn(true);
        when(deployTypeService.isCyberEngine()).thenReturn(true);
        Map<String, Object> parameters = new HashMap<>();
        Map<String, Object> params = new HashMap<>();
        params.put("fileSystemKeepTime", 95);
        parameters.put("performance", params);
        liveMountObject.setParameters(parameters);
        liveMountService.createLiveMountCommon(liveMountObject);
    }

    /**
     * createLiveMount but count More than Total Limit
     */
    @Test
    public void createLiveMountCountMoreThanTotalLimit() {
        expectedException.expect(LegoCheckedException.class);
        expectedException.expectMessage("Exceeded the limit(20000000).");
        LiveMountObject liveMountObject = getLiveMountObject();
        liveMountObject.setCopyId("12");
        CopiesEntity copiesEntity = CopiesEntity.builder().build();
        copiesEntity.setResourceSubType("aa");
        copiesEntity.setSlaProperties(
            "{\"policy_list\":[{\"ext_parameters\":{\"storage_info\": {\"storage_type\": \"storage_unit\", \"storage_id\": \"80ce06b8-5"
                + "0e0-4695-916e-d444aa7d347a\"}}}]}"
        );
        StorageUnitVo storageUnitVo = new StorageUnitVo();
        storageUnitVo.setDeviceType("OceanProtectX");

        when(copyMapper.selectById(liveMountObject.getCopyId())).thenReturn(copiesEntity);
        when(storageUnitService.getStorageUnitById(anyString())).thenReturn(Optional.of(storageUnitVo));
        when(deployTypeService.isCyberEngine()).thenReturn(true);
        PowerMockito.when(liveMountEntityDao.selectCount(null)).thenReturn(20000001L);
        PowerMockito.when(systemSpecificationService.getClusterNodeCount()).thenReturn(2);
        liveMountService.createLiveMountCommon(liveMountObject);
    }

    /**
     * createLiveMount but policy id is not null and policy is null
     */
    @Test
    public void createLiveMountPolicyIdIsNotNull() {
        LiveMountObject liveMountObject = getLiveMountObject();
        liveMountObject.setPolicyId("policy_id_1");
        expectedException.expect(LegoCheckedException.class);
        expectedException.expectMessage("policy is not exist. id=" + liveMountObject.getPolicyId());
        liveMountObject.setCopyId("12");
        CopiesEntity copiesEntity = CopiesEntity.builder().build();
        copiesEntity.setResourceSubType("aa");
        copiesEntity.setSlaProperties(
            "{\"policy_list\":[{\"ext_parameters\":{\"storage_info\": {\"storage_type\": \"storage_unit\", \"storage_id\": \"80ce06b8-5"
                + "0e0-4695-916e-d444aa7d347a\"}}}]}"
        );
        StorageUnitVo storageUnitVo = new StorageUnitVo();
        storageUnitVo.setDeviceType("OceanProtectX");

        when(copyMapper.selectById(liveMountObject.getCopyId())).thenReturn(copiesEntity);
        when(storageUnitService.getStorageUnitById(anyString())).thenReturn(Optional.of(storageUnitVo));
        when(deployTypeService.isCyberEngine()).thenReturn(true);
        PowerMockito.when(liveMountEntityDao.selectCount(null)).thenReturn(1L);
        PowerMockito.when(systemSpecificationService.getClusterNodeCount()).thenReturn(2);
        liveMountService.createLiveMountCommon(liveMountObject);
    }

    /**
     * createLiveMount but policy id is not null and policy is not null
     */
    @Test(expected = NullPointerException.class)
    public void createLiveMountPolicyIdIsNotNullAndPolicyIsNotNull() {
        LiveMountObject liveMountObject = getLiveMountObject();
        liveMountObject.setPolicyId("policy_id_1");
        liveMountObject.setCopyId("12");
        CopiesEntity copiesEntity = CopiesEntity.builder().build();
        copiesEntity.setResourceSubType("aa");
        copiesEntity.setSlaProperties(
            "{\"policy_list\":[{\"ext_parameters\":{\"storage_info\": {\"storage_type\": \"storage_unit\", \"storage_id\": \"80ce06b8-5"
                + "0e0-4695-916e-d444aa7d347a\"}}}]}"
        );
        StorageUnitVo storageUnitVo = new StorageUnitVo();
        storageUnitVo.setDeviceType("OceanProtectX");

        when(copyMapper.selectById(liveMountObject.getCopyId())).thenReturn(copiesEntity);
        when(storageUnitService.getStorageUnitById(anyString())).thenReturn(Optional.of(storageUnitVo));
        when(deployTypeService.isCyberEngine()).thenReturn(true);
        PowerMockito.when(policyService.selectPolicyById(liveMountObject.getPolicyId()))
                .thenReturn(new LiveMountPolicyEntity());
        PowerMockito.when(liveMountEntityDao.selectCount(null)).thenReturn(1L);
        PowerMockito.when(systemSpecificationService.getClusterNodeCount()).thenReturn(2);
        liveMountService.createLiveMountCommon(liveMountObject);
    }

    /**
     * createLiveMount but copy status is not normal
     */
    @Test
    public void createLiveMountCopyStatusIsNotNormal() {
        expectedException.expect(LegoCheckedException.class);
        expectedException.expectMessage("Copy status is not normal");
        LiveMountObject liveMountObject = getLiveMountObject();
        Copy copy = getCopy();
        copy.setStatus(CopyStatus.INVALID.getValue());
        CopiesEntity copiesEntity = CopiesEntity.builder().build();
        liveMountObject.setCopyId("12");
        copiesEntity.setResourceSubType("aa");
        copiesEntity.setSlaProperties(
            "{\"policy_list\":[{\"ext_parameters\":{\"storage_info\": {\"storage_type\": \"storage_unit\", \"storage_id\": \"80ce06b8-5"
                + "0e0-4695-916e-d444aa7d347a\"}}}]}"
        );
        StorageUnitVo storageUnitVo = new StorageUnitVo();
        storageUnitVo.setDeviceType("OceanProtectX");

        when(copyMapper.selectById(liveMountObject.getCopyId())).thenReturn(copiesEntity);
        when(storageUnitService.getStorageUnitById(anyString())).thenReturn(Optional.of(storageUnitVo));
        when(deployTypeService.isCyberEngine()).thenReturn(true);
        Mockito.when(copyRestApi.queryCopyByID(anyString())).thenReturn(copy);
        PowerMockito.when(liveMountEntityDao.selectCount(null)).thenReturn(1L);
        PowerMockito.when(systemSpecificationService.getClusterNodeCount()).thenReturn(2);
        liveMountService.createLiveMountCommon(liveMountObject);
    }

    /**
     * createLiveMount copy is generation three
     */
    @Test
    public void createLiveMountCopyGenerationMoreThanTwo() {
        expectedException.expect(LegoCheckedException.class);
        expectedException.expectMessage("Copy(generation > 2) is not allowed");
        LiveMountObject liveMountObject = getLiveMountObject();
        Copy copy = getCopy();
        copy.setGeneration(IsmNumberConstant.THREE);
        liveMountObject.setCopyId("12");
        CopiesEntity copiesEntity = CopiesEntity.builder().build();
        copiesEntity.setResourceSubType("aa");
        copiesEntity.setSlaProperties(
            "{\"policy_list\":[{\"ext_parameters\":{\"storage_info\": {\"storage_type\": \"storage_unit\", \"storage_id\": \"80ce06b8-5"
                + "0e0-4695-916e-d444aa7d347a\"}}}]}"
        );
        StorageUnitVo storageUnitVo = new StorageUnitVo();
        storageUnitVo.setDeviceType("OceanProtectX");

        when(copyMapper.selectById(liveMountObject.getCopyId())).thenReturn(copiesEntity);
        when(storageUnitService.getStorageUnitById(anyString())).thenReturn(Optional.of(storageUnitVo));
        when(deployTypeService.isCyberEngine()).thenReturn(true);
        when(copyRestApi.queryCopyByID(anyString())).thenReturn(copy);
        PowerMockito.when(liveMountEntityDao.selectCount(null)).thenReturn(1L);
        PowerMockito.when(systemSpecificationService.getClusterNodeCount()).thenReturn(2);
        PowerMockito.doNothing().when(copyAuthVerifyService).checkCopyOperationAuth(Mockito.any(), Mockito.any());
        liveMountService.createLiveMountCommon(liveMountObject);
    }

    /**
     * createLiveMount copy is geneartion two and live is by replica
     */
    @Test
    public void createLiveMountCopyGenerationEqualTwo() {
        expectedException.expect(LegoCheckedException.class);
        expectedException.expectMessage("non-clone copy with generation 2 is not allowed");
        LiveMountObject liveMountObject = getLiveMountObject();
        Copy copy = getCopy();
        copy.setGeneration(IsmNumberConstant.TWO);
        copy.setGeneratedBy(CopyGeneratedByEnum.BY_REPLICATED.value());
        liveMountObject.setCopyId("12");
        CopiesEntity copiesEntity = CopiesEntity.builder().build();
        copiesEntity.setResourceSubType("aa");
        copiesEntity.setSlaProperties(
            "{\"policy_list\":[{\"ext_parameters\":{\"storage_info\": {\"storage_type\": \"storage_unit\", \"storage_id\": \"80ce06b8-5"
                + "0e0-4695-916e-d444aa7d347a\"}}}]}"
        );
        StorageUnitVo storageUnitVo = new StorageUnitVo();
        storageUnitVo.setDeviceType("OceanProtectX");

        when(copyMapper.selectById(liveMountObject.getCopyId())).thenReturn(copiesEntity);
        when(storageUnitService.getStorageUnitById(anyString())).thenReturn(Optional.of(storageUnitVo));
        when(deployTypeService.isCyberEngine()).thenReturn(true);
        PowerMockito.when(liveMountEntityDao.selectCount(null)).thenReturn(1L);
        when(copyRestApi.queryCopyByID(anyString())).thenReturn(copy);
        PowerMockito.when(systemSpecificationService.getClusterNodeCount()).thenReturn(2);
        liveMountService.createLiveMountCommon(liveMountObject);
    }

    /**
     * createLiveMount but Copy Generation More Than One
     */
    @Test(expected = NullPointerException.class)
    public void createLiveMountCopyGenerationEqualTwoGeneratedEqualLiveMount() {
        LiveMountObject liveMountObject = getLiveMountObject();
        Copy copy = getCopy();
        copy.setGeneration(IsmNumberConstant.TWO);
        copy.setGeneratedBy(CopyGeneratedByEnum.BY_LIVE_MOUNTE.value());
        liveMountObject.setCopyId("12");
        CopiesEntity copiesEntity = CopiesEntity.builder().build();
        copiesEntity.setResourceSubType("aa");
        copiesEntity.setSlaProperties(
            "{\"policy_list\":[{\"ext_parameters\":{\"storage_info\": {\"storage_type\": \"storage_unit\", \"storage_id\": \"80ce06b8-5"
                + "0e0-4695-916e-d444aa7d347a\"}}}]}"
        );
        StorageUnitVo storageUnitVo = new StorageUnitVo();
        storageUnitVo.setDeviceType("OceanProtectX");

        when(copyMapper.selectById(liveMountObject.getCopyId())).thenReturn(copiesEntity);
        when(storageUnitService.getStorageUnitById(anyString())).thenReturn(Optional.of(storageUnitVo));
        when(deployTypeService.isCyberEngine()).thenReturn(true);
        PowerMockito.when(liveMountEntityDao.selectCount(null)).thenReturn(1L);
        PowerMockito.when(systemSpecificationService.getClusterNodeCount()).thenReturn(2);
        CopyResourceSummary copyResourceSummary = Mockito.mock(CopyResourceSummary.class);
        when(copyService.queryCopyResourceSummary(anyString())).thenReturn(copyResourceSummary);
        liveMountService.createLiveMountCommon(liveMountObject);
    }

    /**
     * createLiveMount
     */
    @Test(expected = NullPointerException.class)
    public void createLiveMount() {
        PowerMockito.when(liveMountEntityDao.selectCount(null)).thenReturn(1L);
        PowerMockito.when(systemSpecificationService.getClusterNodeCount()).thenReturn(2);
        when(liveMountEntityDao.selectCount(any())).thenReturn(0L);
        Copy copy = getCopy();
        when(copyRestApi.queryCopyByID(any())).thenReturn(copy);
        CopyResourceSummary resourceSummary = new CopyResourceSummary();
        resourceSummary.setResourceSubType(ResourceSubTypeEnum.ORACLE.getType());
        resourceSummary.setResourceProperties("{\"name\":\"myResource\"}");
        when(copyService.queryCopyResourceSummary(any())).thenReturn(resourceSummary);
        when(liveMountEntityDao.selectOne(any())).thenReturn(null);
        ResourceEntity resourceEntity = new FileSetEntity();
        when(resourceRestApi.queryResource(any())).thenReturn((FileSetEntity) resourceEntity);
        LiveMountFlowService provider = PowerMockito.mock(LiveMountFlowService.class);
        when(providerRegistry.findProvider(any(), any())).thenReturn(provider);
        when(liveMountEntityDao.insert(any())).thenReturn(0);
        AtomicReference<LegoInternalEvent> reference = new AtomicReference<>();
        PowerMockito.doAnswer(invocation -> {
            LegoInternalEvent event = invocation.getArgument(0);
            reference.set(event);
            return null;
        }).when(operationLogService).sendEvent(any(LegoInternalEvent.class));
        LiveMountObject liveMountObject = getLiveMountObject();
        LiveMountServiceProvider serviceProvider = Mockito.mock(LiveMountServiceProvider.class);
        PowerMockito.when(providerManager.findProviderOrDefault(any(),
                isNull(), any())).thenReturn(serviceProvider);
        LiveMountServiceImpl spy = PowerMockito.spy(liveMountService);
        PowerMockito.doNothing().when(spy).executeLiveMount(isNull(),any(),any(),any(),anyBoolean());
        List<String> uuidList = spy.createLiveMountCommon(liveMountObject);
        String uuids = Joiner.on(" ").join(uuidList);
        LegoInternalEvent event = reference.get();
        Assert.assertNull(event);
//        Assert.assertArrayEquals(event.getEventParam(), new String[]{"user", "127.0.0.1", uuids});
    }

    private LiveMountEntity getLiveMountEntity() {
        LiveMountEntity entity = new LiveMountEntity();
        entity.setId("1");
        entity.setStatus(LiveMountStatus.AVAILABLE.getName());
        return entity;
    }

    /**
     * getLiveMountObject
     *
     * @return LiveMountObject
     */
    private LiveMountObject getLiveMountObject() {
        LiveMountObject liveMountObject = new LiveMountObject();
        liveMountObject.setTargetResourceUuidList(Arrays.asList("uuid1", "uuid2"));
        liveMountObject.setSourceResourceId("uuid0");
        liveMountObject.setCopyId("copyId");
        liveMountObject.setTargetLocation(LiveMountTargetLocation.ORIGINAL);
        Performance performance = new Performance();
        Map<String, Object> map = JSONObject.fromObject(performance).toMap(Object.class);
        liveMountObject.setParameters(map);
        return liveMountObject;
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
}
