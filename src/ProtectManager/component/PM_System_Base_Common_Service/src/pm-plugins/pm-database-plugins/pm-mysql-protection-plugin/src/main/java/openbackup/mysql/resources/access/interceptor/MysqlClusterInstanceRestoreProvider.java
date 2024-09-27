package openbackup.mysql.resources.access.interceptor;

import openbackup.data.protection.access.provider.sdk.base.Authentication;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.data.protection.access.provider.sdk.restore.v2.RestoreTask;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.database.base.plugin.enums.DatabaseDeployTypeEnum;
import openbackup.mysql.resources.access.common.MysqlConstants;
import openbackup.mysql.resources.access.common.MysqlErrorCode;
import openbackup.mysql.resources.access.enums.MysqlResourceSubTypeEnum;
import openbackup.mysql.resources.access.service.MysqlBaseService;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.copy.model.Copy;

import com.baomidou.mybatisplus.core.toolkit.CollectionUtils;

import lombok.extern.slf4j.Slf4j;

import org.apache.commons.lang3.StringUtils;
import org.springframework.stereotype.Component;

import java.util.ArrayList;
import java.util.List;
import java.util.Optional;
import java.util.Set;

/**
 *  mysql集群实例恢复任务下发provider
 *
 * @author fwx1022842
 * @version [OceanProtect X8000 1.2.0]
 * @since 2022/6/23
 */
@Slf4j
@Component
public class MysqlClusterInstanceRestoreProvider extends AbstractMysqlRestoreProvider {
    private final MysqlBaseService mysqlBaseService;

    private final ResourceService resourceService;

    private final CopyRestApi copyRestApi;

    /**
     * 构造方法
     *
     * @param mysqlBaseService mysqlBaseService
     * @param copyRestApi 副本rest api
     * @param resourceService resourceService
     */
    public MysqlClusterInstanceRestoreProvider(MysqlBaseService mysqlBaseService,
        ResourceService resourceService, CopyRestApi copyRestApi) {
        super(copyRestApi, mysqlBaseService);
        this.mysqlBaseService = mysqlBaseService;
        this.resourceService = resourceService;
        this.copyRestApi = copyRestApi;
    }

    /**
     * 数据库各自应用信息
     *
     * @param task RestoreTask
     * @return RestoreTask
     */
    @Override
    public RestoreTask supplyRestoreTask(RestoreTask task) {
        RestoreTask mysqlRestoreTask = super.supplyRestoreTask(task);
        // 获取集群实例或者是单实例的version，设置到保护对象的extendInfo里
        ProtectedResource clusterInstanceRes =
                mysqlBaseService.getResource(mysqlRestoreTask.getTargetObject().getUuid());
        mysqlRestoreTask.getTargetObject().setExtendInfo(
                mysqlBaseService.supplyExtendInfo(clusterInstanceRes.getVersion(),
                mysqlRestoreTask.getTargetObject().getExtendInfo()));
        mysqlRestoreTask.getTargetEnv().getExtendInfo()
                .put(DatabaseConstants.DEPLOY_TYPE, DatabaseDeployTypeEnum.AP.getType());
        mysqlRestoreTask.setAgents(new ArrayList<>());
        // 从dependency里，获取集群实例下面的所有子实例
        String uuid = mysqlRestoreTask.getTargetObject().getUuid();
        List<ProtectedResource> singleInstanceResources =
                mysqlBaseService.getSingleInstanceByClusterInstance(
                        mysqlRestoreTask.getTargetObject().getUuid());
        // 遍历子实例信息
        for (ProtectedResource singleInstanceResource : singleInstanceResources) {
            // 从子实例的dependency里，获取子实例对应的Agent主机，并设置到恢复对象中
            mysqlRestoreTask.getAgents().add(mysqlBaseService.getAgentEndpoint(
                    mysqlBaseService.getAgentBySingleInstanceUuid(singleInstanceResource.getUuid())));
        }
        // 将集群实例下子实例的mysql启动方式设置到extendInfo里
        String serviceName = singleInstanceResources.get(0).getExtendInfo()
                .get(MysqlConstants.MYSQL_SERVICE_NAME);
        mysqlRestoreTask.getTargetObject().getExtendInfo()
                .put(MysqlConstants.MYSQL_SERVICE_NAME, serviceName);
        String systemServiceName = singleInstanceResources.get(0).getExtendInfo()
                .get(MysqlConstants.MYSQL_SYSTEM_SERVICE_TYPE);
        mysqlRestoreTask.getTargetObject().getExtendInfo()
                .put(MysqlConstants.MYSQL_SYSTEM_SERVICE_TYPE, systemServiceName);
        // 设置node
        mysqlBaseService.supplyNodes(mysqlRestoreTask);
        // 设置node对应的mysql认证信息、dataDir信息、角色信息、MySQL服务的启动信息
        List<TaskEnvironment> nodes = mysqlRestoreTask.getTargetEnv().getNodes();
        mysqlBaseService.setNodesAuth(nodes, singleInstanceResources);
        // 检验集群信息
        String clusterType = mysqlRestoreTask.getTargetObject().getExtendInfo().get(DatabaseConstants.CLUSTER_TYPE);
        log.info("cluster_type:{}", clusterType);
        if (MysqlConstants.AP.equals(clusterType)) {
            checkAPClusterAuthentication(mysqlRestoreTask, singleInstanceResources);
        }
        // 设置node对应的mysql认证信息、dataDir信息、角色信息、MySQL服务的启动信息
        mysqlBaseService.setNodesAuth(nodes, singleInstanceResources);
        if (!MysqlConstants.EAPP.equals(clusterType)) {
            mysqlBaseService.checkClusterRole(nodes);
        }
        log.info("mysql restore object: {}, agents size: {}", mysqlRestoreTask.getTargetObject().getUuid(),
                mysqlRestoreTask.getAgents().size());
        return mysqlRestoreTask;
    }

    private void checkAPClusterAuthentication(RestoreTask task, List<ProtectedResource> targetResources) {
        String copyId = task.getCopyId();
        Copy copy = copyRestApi.queryCopyByID(copyId);
        String resourceId = copy.getResourceId();
        if (StringUtils.equals(resourceId, task.getTargetObject().getUuid())) {
            log.info("Restore origin target");
            return;
        }
        Optional<ProtectedResource> resource = resourceService.getResourceById(resourceId);
        if (!resource.isPresent()) {
            log.info("Restore origin source {} not exist", resourceId);
            return;
        }
        List<ProtectedResource> sourceResources = mysqlBaseService.getSingleInstanceByClusterInstance(resourceId);
        if (CollectionUtils.isEmpty(sourceResources)) {
            log.info("Restore origin source {} not exist", resourceId);
            return;
        }
        Authentication sourceAuth = sourceResources.get(0).getAuth();
        Authentication targetAuth = targetResources.get(0).getAuth();
        if (sourceAuth.getAuthType() == targetAuth.getAuthType() && StringUtils.equals(sourceAuth.getAuthKey(),
            targetAuth.getAuthKey()) && StringUtils.equals(sourceAuth.getAuthPwd(), targetAuth.getAuthPwd())) {
            return;
        }
        throw new LegoCheckedException(MysqlErrorCode.CHECK_AUTH_INFO_FAILED, "check auth info failed");
    }

    /**
     * 判断目标资源是否可以应用此拦截器
     *
     * @param subType 目标资源subType
     * @return 是否可以应用此拦截器
     */
    @Override
    public boolean applicable(String subType) {
        return MysqlResourceSubTypeEnum.MYSQL_CLUSTER_INSTANCE.getType().equals(subType);
    }

    /**
     * 查询集群实例下面的所有数据库资源，包含它本身，一起设置它们下一次备份必须是全量备份
     *
     * @param task 恢复对象
     * @return 这个对象恢复后，要设置哪些关联的对象下一次备份必须是全量备份
     */
    @Override
    protected List<String> findAssociatedResourcesToSetNextFull(RestoreTask task) {
        List<ProtectedResource> singleInstanceResources =
                mysqlBaseService.getSingleInstanceByClusterInstance(task.getTargetObject().getUuid());
        List<String> associatedResources = new ArrayList<>();
        for (ProtectedResource resource : singleInstanceResources) {
            Set<String> resourceUuids = resourceService.queryRelatedResourceUuids(resource.getUuid(), new String[]{});
            associatedResources.addAll(resourceUuids);
        }
        associatedResources.add(task.getTargetObject().getUuid());
        log.info("set mysql associated resources: {} next backup is full.", String.join(",", associatedResources));
        return associatedResources;
    }
}
