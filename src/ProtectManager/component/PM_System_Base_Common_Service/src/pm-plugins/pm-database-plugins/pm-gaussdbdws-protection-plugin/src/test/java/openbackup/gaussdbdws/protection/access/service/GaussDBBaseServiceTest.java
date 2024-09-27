package openbackup.gaussdbdws.protection.access.service;

import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyInt;

import openbackup.access.framework.resource.service.provider.UnifiedHealthCheckProvider;
import openbackup.data.access.framework.backup.constant.BackupConstant;
import openbackup.data.access.framework.core.common.util.EnvironmentLinkStatusHelper;
import openbackup.data.access.framework.core.manager.ProviderManager;
import openbackup.data.access.framework.protection.service.repository.TaskRepositoryManager;
import openbackup.data.access.framework.servitization.entity.VpcInfoEntity;
import openbackup.data.access.framework.servitization.service.IVpcService;
import openbackup.data.protection.access.provider.sdk.agent.AgentSelectParam;
import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.base.PageListResponse;
import openbackup.data.protection.access.provider.sdk.base.v2.StorageRepository;
import openbackup.data.protection.access.provider.sdk.resource.BrowseEnvironmentResourceConditions;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResourceChecker;
import openbackup.data.protection.access.provider.sdk.resource.ResourceConnectionCheckProvider;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.gaussdbdws.protection.access.constant.DwsConstant;
import openbackup.gaussdbdws.protection.access.interceptor.backup.MockInterceptorParameter;

import openbackup.system.base.common.constants.LegoNumberConstant;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.sdk.resource.enums.LinkStatusEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.service.hostagent.AgentQueryService;
import openbackup.system.base.service.hostagent.model.AgentInfo;
import openbackup.system.base.util.OpServiceUtil;

import com.alibaba.fastjson.JSON;

import org.junit.Assert;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mockito;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.api.support.membermodification.MemberModifier;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.PowerMockRunner;

import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Optional;

/**
 * DWS集群 基础类工具类
 *
 * @author swx1010572
 * @since 2023-09-13
 */
@RunWith(PowerMockRunner.class)
@PrepareForTest( {UnifiedHealthCheckProvider.class, EnvironmentLinkStatusHelper.class, OpServiceUtil.class})
public class GaussDBBaseServiceTest {
    private final ResourceService resourceService = Mockito.mock(ResourceService.class);

    private final ProtectedResourceChecker protectedResourceChecker = Mockito.mock(ProtectedResourceChecker.class);

    private final ProviderManager providerManager = Mockito.mock(ProviderManager.class);

    private final AgentQueryService agentQueryService = Mockito.mock(AgentQueryService.class);

    private final IVpcService iVpcService = Mockito.mock(IVpcService.class);

    private final ResourceConnectionCheckProvider resourceConnectionCheckProvider = Mockito.mock(
        ResourceConnectionCheckProvider.class);

    private final TaskRepositoryManager taskRepositoryManager = Mockito.mock(TaskRepositoryManager.class);

    private final GaussDBBaseService gaussDBBaseService = new GaussDBBaseService(resourceService,
        protectedResourceChecker, providerManager, resourceConnectionCheckProvider, taskRepositoryManager);

    /**
     * 用例场景：GaussDB(DWS) 获取schema集和表集的rootUuid集合(去重)
     * 前置条件：无
     * 检查点：获取成功符合要求
     */
    @Test
    public void check_get_all_table_and_schema_uuid_list_success() {
        PageListResponse<ProtectedResource> records = new PageListResponse<>();
        records.setTotalCount(1);
        List<ProtectedResource> protectedResourceList = new ArrayList<>();
        ProtectedResource protectedResource = new ProtectedResource();
        protectedResource.setRootUuid("123545678");
        ProtectedResource protectedResource1 = new ProtectedResource();
        protectedResource.setRootUuid("12354567");
        protectedResourceList.add(protectedResource);
        protectedResourceList.add(protectedResource1);
        records.setRecords(protectedResourceList);
        PowerMockito.when(resourceService.query(LegoNumberConstant.ZERO, LegoNumberConstant.ONE,
            Collections.singletonMap("subType", ResourceSubTypeEnum.GAUSSDB_DWS_SCHEMA.getType()))).thenReturn(records);
        PowerMockito.when(resourceService.query(LegoNumberConstant.ZERO, DwsConstant.QUERY_QUANTITY_SPECIFICATIONS,
            Collections.singletonMap("subType", ResourceSubTypeEnum.GAUSSDB_DWS_SCHEMA.getType()))).thenReturn(records);
        PowerMockito.when(resourceService.query(LegoNumberConstant.ZERO, LegoNumberConstant.ONE,
            Collections.singletonMap("subType", ResourceSubTypeEnum.GAUSSDB_DWS_TABLE.getType()))).thenReturn(records);
        PowerMockito.when(resourceService.query(LegoNumberConstant.ZERO, DwsConstant.QUERY_QUANTITY_SPECIFICATIONS,
            Collections.singletonMap("subType", ResourceSubTypeEnum.GAUSSDB_DWS_TABLE.getType()))).thenReturn(records);
        Assert.assertEquals(2, gaussDBBaseService.getAllTableAndSchemaUuidList().size());
    }

    /**
     * 用例场景：GaussDB(DWS) 获取下发agent参数对象
     * 前置条件：无
     * 检查点：获取成功符合要求
     */
    @Test
    public void check_get_list_resource_req_success() {
        gaussDBBaseService.getListResourceReq(new ProtectedEnvironment(), new ProtectedEnvironment(),
            new BrowseEnvironmentResourceConditions());
    }

    /**
     * 用例场景：GaussDB(DWS) 获取当前所有的subType 采用注册上来的资源
     * 前置条件：无
     * 检查点：获取成功符合要求
     */
    @Test
    public void check_get_existing_dws_resources_success() {
        PageListResponse<ProtectedResource> records = new PageListResponse<>();
        records.setTotalCount(1);
        List<ProtectedResource> protectedResourceList = new ArrayList<>();
        ProtectedEnvironment protectedResource = new ProtectedEnvironment();
        protectedResource.setRootUuid("123545678");
        protectedResourceList.add(protectedResource);
        records.setRecords(protectedResourceList);
        PowerMockito.when(resourceService.query(anyInt(), anyInt(), any())).thenReturn(records);
        Assert.assertEquals(1,
            gaussDBBaseService.getExistingDwsResources(ResourceSubTypeEnum.GAUSSDB_DWS_SCHEMA.getType()).size());
    }

    /**
     * 用例场景：GaussDB(DWS) 获取所有subType和sourceType方式rootUuid为传参的列表
     * 前置条件：无
     * 检查点：获取成功符合要求
     */
    @Test
    public void check_get_env_resource_list_success() {
        PageListResponse<ProtectedResource> records = new PageListResponse<>();
        records.setTotalCount(1);
        List<ProtectedResource> protectedResourceList = new ArrayList<>();
        ProtectedEnvironment protectedResource = new ProtectedEnvironment();
        protectedResource.setRootUuid("123545678");
        protectedResourceList.add(protectedResource);
        records.setRecords(protectedResourceList);
        PowerMockito.when(resourceService.query(anyInt(), anyInt(), any())).thenReturn(records);
        Assert.assertEquals(1,
            gaussDBBaseService.getEnvResourceList("uuid", ResourceSubTypeEnum.GAUSSDB_DWS.getType(), "autoscan")
                .size());
    }

    /**
     * 用例场景：查群集群资源状态为离线
     * 前置条件：集群资源状态为离线
     * 检查点：集群资源状态为离线，抛出异常
     */
    @Test
    public void should_throw_LegoCheckedException_if_check_link_status_is_offline() {
        ProtectedEnvironment protectedEnvironment = new ProtectedEnvironment();
        protectedEnvironment.setLinkStatus(LinkStatusEnum.OFFLINE.getStatus().toString());
        PowerMockito.when(resourceService.getResourceByIdIgnoreOwner("uuid")).thenReturn(Optional.of(protectedEnvironment));
        Assert.assertThrows(LegoCheckedException.class, () -> gaussDBBaseService.checkLinkStatus("uuid"));
    }

    /**
     * 用例场景：GaussDB(DWS) 校验应用集群是否在线;
     * 前置条件：无
     * 检查点：不抛错
     */
    @Test
    public void check_check_link_status_success() {
        ProtectedEnvironment protectedEnvironment = new ProtectedEnvironment();
        protectedEnvironment.setLinkStatus(LinkStatusEnum.ONLINE.getStatus().toString());
        PowerMockito.when(resourceService.getResourceByIdIgnoreOwner("uuid")).thenReturn(Optional.of(protectedEnvironment));
        gaussDBBaseService.checkLinkStatus("uuid", ResourceSubTypeEnum.GAUSSDB_DWS.getType());
        PowerMockito.mockStatic(EnvironmentLinkStatusHelper.class);
        PowerMockito.when(EnvironmentLinkStatusHelper.getLinkStatusAdaptMultiCluster(any())).thenReturn("1");
        gaussDBBaseService.checkLinkStatus("uuid", ResourceSubTypeEnum.GAUSSDB_DWS_SCHEMA.getType());
        PowerMockito.mockStatic(EnvironmentLinkStatusHelper.class);
        PowerMockito.when(EnvironmentLinkStatusHelper.getLinkStatusAdaptMultiCluster(any())).thenReturn("0");
        Assert.assertThrows(LegoCheckedException.class,
            () -> gaussDBBaseService.checkLinkStatus("uuid", ResourceSubTypeEnum.GAUSSDB_DWS_SCHEMA.getType()));
    }

    /**
     * 用例场景：GaussDB(DWS) 增加本地存储的容器阈值成功
     * 前置条件：无
     * 检查点：不抛错
     */
    @Test
    public void check_add_repository_capacity_success() {
        gaussDBBaseService.addRepositoryCapacity(new StorageRepository(), "storageId",
            BackupConstant.BACKUP_EXT_PARAM_STORAGE_UNIT_GROUP_VALUE);
    }

    /**
     * 用例场景：GaussDB(DWS) 取第一个在线agent 环境
     * 前置条件：无
     * 检查点：不抛错
     */
    @Test
    public void check_get_cluster_uuid_success() {
        List<ProtectedResource> clusterResources = new ArrayList<>();
        ProtectedResource protectedResource = new ProtectedResource();
        protectedResource.setUuid("uuid");
        clusterResources.add(protectedResource);
        ProtectedEnvironment protectedEnvironment = new ProtectedEnvironment();
        protectedEnvironment.setLinkStatus(LinkStatusEnum.ONLINE.getStatus().toString());
        PowerMockito.when(resourceService.getResourceByIdIgnoreOwner("uuid")).thenReturn(Optional.of(protectedEnvironment));
        gaussDBBaseService.getClusterUuid(clusterResources);
    }

    /**
     * 用例场景：GaussDB(DWS) 填充agent信息 成功
     * 前置条件：无
     * 检查点：不抛错
     */
    @Test
    public void check_supply_agent_success() {
        ProtectedResource protectedResource = new ProtectedResource();
        protectedResource.setRootUuid("rootUuid");
        AgentSelectParam agentSelectParam = AgentSelectParam.builder().resource(protectedResource).build();
        PowerMockito.when(resourceService.getResourceByIdIgnoreOwner("rootUuid")).thenReturn(Optional.of(protectedResource));
        ProtectedResourceChecker checker = Mockito.mock(ProtectedResourceChecker.class);
        PowerMockito.when(providerManager.findProviderOrDefault(ProtectedResourceChecker.class, protectedResource,
            protectedResourceChecker)).thenReturn(checker);
        Map<ProtectedResource, List<ProtectedEnvironment>> protectedResourceMap = new HashMap<>();
        List<ProtectedEnvironment> list = new ArrayList<>();
        ProtectedEnvironment protectedEnvironment = new ProtectedEnvironment();
        protectedEnvironment.setUuid("rootUuid");
        protectedEnvironment.setEndpoint("rootUuid");
        protectedEnvironment.setPort(27017);
        list.add(protectedEnvironment);
        protectedResourceMap.put(protectedResource, list);
        PowerMockito.when(checker.collectConnectableResources(protectedResource)).thenReturn(protectedResourceMap);
        Assert.assertEquals(1, gaussDBBaseService.supplyAgent(agentSelectParam).size());
    }

    /**
     * 用例场景：GaussDB(DWS) 增加agent列表
     * 前置条件：无
     * 检查点：不抛错
     */
    @Test
    public void check_add_supply_agent_success() throws Exception {
        MemberModifier.field(GaussDBBaseService.class, "agentQueryService").set(gaussDBBaseService, agentQueryService);
        PowerMockito.mockStatic(OpServiceUtil.class);
        PowerMockito.doReturn(false).when(OpServiceUtil.class, "isHcsService");
        List<Endpoint> objects = new ArrayList<>();
        PowerMockito.when(resourceService.getResourceByIdIgnoreOwner("uuid")).thenReturn(Optional.of(MockInterceptorParameter.getProtectedEnvironment()));
        gaussDBBaseService.addSupplyAgent(objects, "uuid", "DWS-clusterPlugin");
        Assert.assertEquals(objects.size(), 0);
        PowerMockito.doReturn(true).when(OpServiceUtil.class, "isHcsService");
        openbackup.system.base.common.model.PageListResponse<AgentInfo> records
            = new openbackup.system.base.common.model.PageListResponse<>();
        records.setTotalCount(1);
        List<AgentInfo> protectedResourceList = new ArrayList<>();
        AgentInfo protectedResource = new AgentInfo();
        protectedResource.setUuid("123545678");
        protectedResource.setEndpoint("123545678");
        protectedResource.setPort(5525);
        protectedResourceList.add(protectedResource);
        records.setRecords(protectedResourceList);
        PowerMockito.when(agentQueryService.querySharedAgents(any())).thenReturn(records);
        gaussDBBaseService.addSupplyAgent(objects, "uuid","DWS-clusterPlugin");
        Assert.assertEquals(objects.size(), 0);
    }

    /**
     * 用例场景：GaussDB(DWS) 增加agent列表, 代理列表是空
     * 前置条件：无
     * 检查点：不抛错
     */
    @Test
    public void check_add_supply_agent_with_no_agent() throws Exception {
        MemberModifier.field(GaussDBBaseService.class, "agentQueryService").set(gaussDBBaseService, agentQueryService);
        PowerMockito.mockStatic(OpServiceUtil.class);
        PowerMockito.doReturn(false).when(OpServiceUtil.class, "isHcsService");
        List<Endpoint> objects = new ArrayList<>();
        PowerMockito.when(resourceService.getResourceByIdIgnoreOwner("uuid")).thenReturn(Optional.of(MockInterceptorParameter.getProtectedEnvironment()));
        gaussDBBaseService.addSupplyAgent(objects, "uuid", "DWS-clusterPlugin");
        Assert.assertEquals(objects.size(), 0);
    }

    /**
     * 用例场景：GaussDB(DWS) 增加agent列表
     * 前置条件：无
     * 检查点：不抛错
     */
    @Test
    public void check_modify_advance_params_success() throws Exception {
        Map<String, String> advanceParams = new HashMap<>();
        advanceParams.put("vpc_info", "uuid");
        PowerMockito.mockStatic(OpServiceUtil.class);
        PowerMockito.doReturn(false).when(OpServiceUtil.class, "isHcsService");
        ProtectedEnvironment protectedEnvironment = new ProtectedEnvironment();
        protectedEnvironment.setUserId("userid");
        PowerMockito.when(resourceService.getResourceByIdIgnoreOwner("uuid")).thenReturn(Optional.of(protectedEnvironment));
        gaussDBBaseService.modifyAdvanceParams(advanceParams, "uuid");
        Assert.assertEquals(advanceParams.get("vpc_info"), "uuid");
        PowerMockito.doReturn(true).when(OpServiceUtil.class, "isHcsService");
        List<VpcInfoEntity> vpcInfoEntities = new ArrayList<>();
        MemberModifier.field(GaussDBBaseService.class, "iVpcService").set(gaussDBBaseService, iVpcService);
        PowerMockito.when(iVpcService.getVpcInfoEntityByProjectId("userid")).thenReturn(vpcInfoEntities);
        gaussDBBaseService.modifyAdvanceParams(advanceParams, "uuid");
        Assert.assertEquals(advanceParams.get("vpc_info"), JSON.toJSONString(vpcInfoEntities));
    }
}
