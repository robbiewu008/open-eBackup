/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

package openbackup.mysql.resources.access.interceptor;

import openbackup.data.protection.access.provider.sdk.base.Authentication;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.restore.v2.RestoreTask;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.database.base.plugin.enums.DatabaseDeployTypeEnum;
import com.huawei.oceanprotect.kms.sdk.EncryptorService;
import openbackup.mysql.resources.access.common.MysqlConstants;
import openbackup.mysql.resources.access.enums.MysqlResourceSubTypeEnum;
import openbackup.mysql.resources.access.service.MysqlBaseService;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.util.BeanTools;

import lombok.extern.slf4j.Slf4j;

import org.springframework.stereotype.Component;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;
import java.util.Map;

/**
 * mysql数据库恢复任务下发provider
 *
 * @author fwx1022842
 * @version [OceanProtect X8000 1.2.0]
 * @since 2022/6/23
 */
@Slf4j
@Component
public class MysqlDatabaseRestoreProvider extends AbstractMysqlRestoreProvider {
    /**
     * 数据库资源对应的单实例或者集群实例的uuid
     */
    private static final String MYSQL_INSTANCE_UUID = "mysqlInstanceUuid";

    private final MysqlBaseService mysqlBaseService;

    private final EncryptorService encryptorService;

    /**
     * 构造方法
     *
     * @param mysqlBaseService mysqlBaseService
     * @param copyRestApi 副本rest api
     * @param encryptorService encryptorService
     */
    public MysqlDatabaseRestoreProvider(MysqlBaseService mysqlBaseService, CopyRestApi copyRestApi,
        EncryptorService encryptorService) {
        super(copyRestApi, mysqlBaseService);
        this.mysqlBaseService = mysqlBaseService;
        this.encryptorService = encryptorService;
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

        mysqlRestoreTask.setAgents(new ArrayList<>());
        // 获取数据库资源对应的父资源单实例的auth信息并设置
        ProtectedResource singleInstanceRes =
            mysqlBaseService.getResource(mysqlRestoreTask.getTargetObject().getParentUuid());
        Authentication authentication = new Authentication();
        mysqlRestoreTask.getTargetObject().setAuth(
                BeanTools.copy(singleInstanceRes.getAuth(), authentication));

        // 获取数据库资源对应的父资源单实例对应的父资源信息，此时这个资源可能是主机，可能是MySQL集群实例
        ProtectedResource grantParentResource = mysqlBaseService.getResource(singleInstanceRes.getParentUuid());

        // 汇总这个数据库恢复需要单实例信息，因为如果是集群实例下的数据库恢复，需要恢复到集群实例下所有子实例
        // 并且设置好这个数据库资源对应的单实例或者集群实例的uuid，在PM插件恢复的后置操作里需要使用
        List<ProtectedResource> singleInstanceResList = new ArrayList<>();
        if (grantParentResource instanceof ProtectedEnvironment) {
            singleInstanceResList.add(singleInstanceRes);
            mysqlRestoreTask.getTargetEnv().getExtendInfo().put(
                DatabaseConstants.DEPLOY_TYPE, DatabaseDeployTypeEnum.SINGLE.getType());
            task.getAdvanceParams().put(MYSQL_INSTANCE_UUID, singleInstanceRes.getUuid());
        } else {
            // 获取集群实例下的所有单实例
            singleInstanceResList.addAll(
                mysqlBaseService.getSingleInstanceByClusterInstance(grantParentResource.getUuid()));
            mysqlRestoreTask.getTargetEnv().getExtendInfo()
                .put(DatabaseConstants.DEPLOY_TYPE, DatabaseDeployTypeEnum.AP.getType());
            task.getAdvanceParams().put(MYSQL_INSTANCE_UUID, grantParentResource.getUuid());
        }

        // 根据恢复需要的单实例信息，找到这些单实例信息对应的Agent信息
        for (ProtectedResource restoreNeedSingleInstanceRes : singleInstanceResList) {
            mysqlRestoreTask.getAgents().add(mysqlBaseService.getAgentEndpoint(
                mysqlBaseService.getAgentBySingleInstanceUuid(restoreNeedSingleInstanceRes.getUuid())));
        }

        // 取任意一个子实例的extendInfo里的信息下发下去，因为数据库恢复需要用户这个数据库对应的实例是否哪种集群类型
        Map<String, String> dbResExtendInfo = mysqlRestoreTask.getTargetObject().getExtendInfo();
        Map<String, String> insResExtendInfo = singleInstanceResList.get(0).getExtendInfo();
        if (dbResExtendInfo == null) {
            mysqlRestoreTask.getTargetObject().setExtendInfo(insResExtendInfo);
        } else {
            for (String key : insResExtendInfo.keySet()) {
                dbResExtendInfo.put(key, insResExtendInfo.get(key));
            }
        }

        // 解密dataDir参数
        mysqlRestoreTask.getTargetObject().getExtendInfo().put(DatabaseConstants.DATA_DIR,
            encryptorService.decrypt(mysqlRestoreTask.getTargetObject()
                .getExtendInfo().get(DatabaseConstants.DATA_DIR)));

        // 解密logbin日志文件路径参数
        mysqlRestoreTask.getTargetObject().getExtendInfo().put(MysqlConstants.LOG_BIN_INDEX_PATH,
                encryptorService.decrypt(mysqlRestoreTask.getTargetObject()
                        .getExtendInfo().get(MysqlConstants.LOG_BIN_INDEX_PATH)));

        // 根据Agents，设置node，DME会用于任务拆分
        mysqlBaseService.supplyNodes(mysqlRestoreTask);

        // 根据node以及Agents，设置node对应的mysql认证信息，底层需要根据节点获取到mysql认证信息
        mysqlBaseService.setNodesAuth(mysqlRestoreTask.getTargetEnv().getNodes(), singleInstanceResList);

        // 检验集群信息
        if (DatabaseDeployTypeEnum.AP.getType().equals(
                mysqlRestoreTask.getTargetEnv().getExtendInfo().get(DatabaseConstants.DEPLOY_TYPE))) {
            mysqlBaseService.checkClusterRole(mysqlRestoreTask.getTargetEnv().getNodes());
        }

        log.info("mysql restore object: {}, agents size: {}", mysqlRestoreTask.getTargetObject().getUuid(),
                mysqlRestoreTask.getAgents().size());
        return mysqlRestoreTask;
    }

    /**
     * 判断目标资源是否可以应用此拦截器
     *
     * @param subType 目标资源subType
     * @return 是否可以应用此拦截器
     */
    @Override
    public boolean applicable(String subType) {
        return MysqlResourceSubTypeEnum.MYSQL_DATABASE.getType().equals(subType);
    }

    /**
     * 查询数据库资源的实例资源，包含它本身，一起设置它们下一次备份必须是全量备份
     *
     * @param task 恢复对象
     * @return 这个对象恢复后，要设置哪些关联的对象下一次备份必须是全量备份
     */
    @Override
    protected List<String> findAssociatedResourcesToSetNextFull(RestoreTask task) {
        // 区分单实例的数据库资源，以及集群实例下的数据库资源
        List<String> associatedResources = Arrays.asList(task.getTargetObject().getUuid(),
                task.getAdvanceParams().get(MYSQL_INSTANCE_UUID));
        log.info("set mysql associated resources: {} next backup is full.", String.join(",", associatedResources));
        return associatedResources;
    }
}
