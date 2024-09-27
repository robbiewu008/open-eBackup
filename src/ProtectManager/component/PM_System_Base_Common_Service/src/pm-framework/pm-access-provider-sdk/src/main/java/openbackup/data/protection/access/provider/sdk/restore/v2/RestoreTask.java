package openbackup.data.protection.access.provider.sdk.restore.v2;

import openbackup.data.protection.access.provider.sdk.base.v2.BaseTask;
import openbackup.data.protection.access.provider.sdk.base.v2.StorageRepository;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskEnvironment;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskResource;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskResourceFilter;
import openbackup.data.protection.access.provider.sdk.enums.RestoreLocationEnum;
import openbackup.system.base.common.utils.JSONObject;

import org.apache.commons.collections.CollectionUtils;

import java.util.HashMap;
import java.util.List;
import java.util.Map;

/**
 * 副本恢复参数对象，恢复时所需要的参数由该对象承载
 *
 * @author j00364432
 * @version [OceanProtect X8000 1.2.1]
 * @since 2021-11-01
 */
public class RestoreTask extends BaseTask {
    /**
     * 恢复类型枚举类
     */
    private String restoreType;

    /**
     * 恢复模式
     */
    private String restoreMode;

    /**
     * 恢复目标(DME接口不需要，序列化时忽略该字段)
     */
    private RestoreLocationEnum targetLocation;

    /**
     * 恢复的子对象，可用于细粒度恢复
     */
    private List<TaskResource> subObjects;

    /**
     * 资源过滤器列表
     */
    private List<TaskResourceFilter> filters;

    /**
     * 恢复任务的高级参数
     */
    private Map<String, String> advanceParams;

    /**
     * 在Agent上执行的相关恢复脚本
     */
    private RestoreScript scripts;

    public RestoreScript getScripts() {
        return scripts;
    }

    public void setScripts(RestoreScript scripts) {
        this.scripts = scripts;
    }

    public String getRestoreType() {
        return restoreType;
    }

    public void setRestoreType(String restoreType) {
        this.restoreType = restoreType;
    }

    public List<TaskResource> getSubObjects() {
        return subObjects;
    }

    public void setSubObjects(List<TaskResource> subObjects) {
        this.subObjects = subObjects;
    }

    public List<TaskResourceFilter> getFilters() {
        return filters;
    }

    public void setFilters(List<TaskResourceFilter> filters) {
        this.filters = filters;
    }

    public Map<String, String> getAdvanceParams() {
        if (this.advanceParams == null) {
            this.advanceParams = new HashMap<>();
        }
        return advanceParams;
    }

    public void setAdvanceParams(Map<String, String> advanceParams) {
        this.advanceParams = advanceParams;
    }

    public RestoreLocationEnum getTargetLocation() {
        return targetLocation;
    }

    public void setTargetLocation(RestoreLocationEnum targetLocation) {
        this.targetLocation = targetLocation;
    }

    public String getRestoreMode() {
        return restoreMode;
    }

    public void setRestoreMode(String restoreMode) {
        this.restoreMode = restoreMode;
    }

    /**
     * 加密信息解密
     *
     * @return 解密加密信息之后的恢复任务
     */
    public RestoreTask decryptRestoreTask() {
        // 解密target Env auth password
        getTargetEnv().decryptPassword();
        // 解密target Obj auth password
        getTargetObject().decryptPassword();
        // 解密target Env extendInfo
        getTargetEnv().decryptExtendInfo();
        // 解密target Obj extendInfo
        getTargetObject().decryptExtendInfo();
        // 解密target Env auth extendInfo
        getTargetEnv().decryptAuthExtendInfo();
        // 解密target obj auth extendInfo
        getTargetObject().decryptAuthExtendInfo();
        // 解密repository auth password
        if (CollectionUtils.isEmpty(getRepositories())) {
            return this;
        }
        getRepositories().forEach(StorageRepository::decryptPassword);
        // 解密targetEnv nodes列表中的 auth password
        if (CollectionUtils.isEmpty(getTargetEnv().getNodes())) {
            return this;
        }
        getTargetEnv().getNodes().forEach(TaskEnvironment::decryptPassword);
        return this;
    }

    /**
     * 加密信息
     *
     * @return 加密信息之后的恢复任务
     */
    public RestoreTask encryptRestoreTask() {
        // 加密target Env auth password
        getTargetObject().encryptPassword();
        // 加密target Obj auth password
        getTargetEnv().encryptPassword();
        // 加密target Env extendInfo
        getTargetObject().encryptExtendInfo();
        // 加密target Obj extendInfo
        getTargetEnv().encryptExtendInfo();
        // 加密target Env auth extendInfo
        getTargetObject().encryptAuthExtendInfo();
        // 加密target obj auth extendInfo
        getTargetEnv().encryptAuthExtendInfo();
        if (CollectionUtils.isEmpty(getTargetEnv().getNodes())) {
            return this;
        }
        // 加密targetEnv nodes列表中的 auth password
        getTargetEnv().getNodes().forEach(TaskEnvironment::encryptPassword);
        return this;
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
     * 整对象深克隆，新对象与当前对象没有任何引用关系
     *
     * @return 克隆后的新对象
     */
    public RestoreTask deepClone() {
        return JSONObject.fromObject(this).toBean(RestoreTask.class);
    }
}
