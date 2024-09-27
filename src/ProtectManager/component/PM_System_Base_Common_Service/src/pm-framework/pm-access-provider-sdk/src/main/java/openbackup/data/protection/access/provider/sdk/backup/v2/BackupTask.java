/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

package openbackup.data.protection.access.provider.sdk.backup.v2;

import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.base.Qos;
import openbackup.data.protection.access.provider.sdk.base.v2.StorageRepository;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskEnvironment;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskResource;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskResourceFilter;

import com.fasterxml.jackson.annotation.JsonIgnore;

import lombok.Data;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

/**
 * 通用备份框架备份参数对象，备份相关参数通过该对象承载
 *
 * @author j00364432
 * @version [OceanProtect A8000 1.1.0]
 * @since 2021-11-01
 */
@Data
public class BackupTask {
    // 备份请求ID
    private String requestId;

    // 备份任务ID，ProtectManager中保存的备份任务ID
    private String taskId;

    // 副本ID
    private String copyId;

    // 备份方式
    private String backupType;

    // 备份对象，即具体的受保护资源是什么
    private TaskResource protectObject;

    // 备份子对象，用于存放细粒度备份的对象，如备份数据库实例时，可选择只备份该实例下的某几个数据库
    private List<TaskResource> protectSubObjects;

    // 备份的环境列表，即具体的受保护资源在那些受保护环境中
    private TaskEnvironment protectEnv;

    // 备份代理列表
    private List<Endpoint> agents;

    // 可对某些资源进行过滤，比如VMware可设置一些过滤规则将某些磁盘过滤掉
    private List<TaskResourceFilter> filters;

    // Qos
    private Qos qos;

    // 数据布局：重删，压缩，加密
    private DataLayout dataLayout;

    // 备份脚本
    private BackupScript scripts;

    // 高级备份参数，key/value键值对存放
    private Map<String, String> advanceParams;

    // 副本格式
    private int copyFormat;

    private List<StorageRepository> repositories;

    // 本次备份相对于上一次备份发生变换的参数，
    // key为SLA中Policy设置的备份参数， value为一个长度为2的数字，value[0]为上一次的参数的值，value[1]为本次备份的值
    // 如value[0]为null则代表参数是本次增加的。如value[1]null，表示该参数在备份任务中被删除
    @JsonIgnore
    private Map<String, String[]> changedBackupParameters = new HashMap<>();

    /**
     * 添加备份参数
     *
     * @param key 参数名称
     * @param value 参数值
     */
    public void addParameter(String key, String value) {
        if (this.advanceParams == null) {
            this.advanceParams = new HashMap<>();
        }
        this.advanceParams.put(key, value);
    }

    /**
     * 添加高级参数
     *
     * @param param 待新增的高级参数map
     */
    public void addParameters(Map<String, String> param) {
        if (this.advanceParams == null) {
            this.advanceParams = new HashMap<>();
        }
        this.advanceParams.putAll(param);
    }

    /**
     * 增加存储库
     *
     * @param repository 存储库
     */
    public void addRepository(StorageRepository repository) {
        if (repositories == null) {
            repositories = new ArrayList<>();
        }
        repositories.add(repository);
    }

    /**
     * 增加存储库
     *
     * @param repositories 存储库列表
     */
    public void addAllRepository(List<StorageRepository> repositories) {
        if (this.repositories == null) {
            this.repositories = new ArrayList<>();
        }
        this.repositories.addAll(repositories);
    }

    /**
     * 检查某个参数的值是否发生改变
     *
     * @param parameterName 参数的名称
     * @return 改变返回true，否则返回false
     */
    public boolean isParameterChanged(String parameterName) {
        return changedBackupParameters.containsKey(parameterName);
    }

    /**
     * 检查某个备份参数在本次备份任务中是否被删除
     *
     * @param parameterName 参数名称
     * @return 被删除则返回true，否则返回false
     */
    public boolean isParameterDeleted(String parameterName) {
        if (!changedBackupParameters.containsKey(parameterName)) {
            return false;
        }
        String[] values = changedBackupParameters.get(parameterName);
        return values[0] != null && values[1] == null;
    }

    /**
     * 检查某个备份参数在备份任务是否为新增加的
     *
     * @param parameterName 参数名称
     * @return 是则返回true，否则返回false
     */
    public boolean isParameterAdded(String parameterName) {
        if (!changedBackupParameters.containsKey(parameterName)) {
            return false;
        }
        String[] values = changedBackupParameters.get(parameterName);
        return values[0] == null && values[1] != null;
    }

    /**
     * 添加发生变化的参数
     *
     * @param parameterName 参数名称
     * @param values 参数值values[0]为上一次备份的参数，values[1]为本次备份的参数
     */
    public void addChangedParameter(String parameterName, String[] values) {
        this.changedBackupParameters.put(parameterName, values);
    }

    /**
     * 内置Agent，不支持设置kerberos，屏蔽备份链路加密开关
     */
    public void shieldBackupLinkEncryption() {
        this.dataLayout.setLinkEncryption(false);
    }

    /**
     * 外置Agent，支持设置kerberos，备份链路加密开关设置为True
     */
    public void openBackupLinkEncryption() {
        this.dataLayout.setLinkEncryption(true);
    }
}
