package openbackup.data.protection.access.provider.sdk.base.v2;

import com.fasterxml.jackson.annotation.JsonIgnore;

/**
 * ResourceAppResource
 *
 * @description: 恢复任务中的应用资源信息
 * @author: y00559272
 * @version: [OceanProtect A8000 1.1.0]
 * @since: 2021/11/30
 **/
public class TaskResource extends TaskCommonResource {
    /**
     * 父资源的uuid。如备份数据库，这个是数据库实例的uuid
     */
    private String parentUuid;

    /**
     * 父资源的名称
     */
    private String parentName;

    @JsonIgnore
    private String targetLocation;

    public String getTargetLocation() {
        return targetLocation;
    }

    public void setTargetLocation(String targetLocation) {
        this.targetLocation = targetLocation;
    }

    public String getParentUuid() {
        return parentUuid;
    }

    public void setParentUuid(String parentUuid) {
        this.parentUuid = parentUuid;
    }

    public String getParentName() {
        return parentName;
    }

    public void setParentName(String parentName) {
        this.parentName = parentName;
    }
}
