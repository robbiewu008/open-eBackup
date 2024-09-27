/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

package openbackup.data.protection.access.provider.sdk.resource;

import org.hibernate.validator.constraints.Length;

/**
 * 资源基类
 *
 * @author j00364432
 * @version [OceanProtect A8000 1.1.0]
 * @since 2021-10-22
 */
public class ResourceBase {
    /**
     * 资源UUID
     */
    @Length(max = 64)
    private String uuid;

    /**
     * 资源名称
     */
    @Length(max = 512)
    private String name;

    /**
     * 资源类型（主类）
     */
    @Length(max = 64)
    private String type;

    /**
     * 资源子类
     */
    @Length(max = 64)
    private String subType;

    /**
     * 资源路径
     */
    @Length(max = 1024)
    private String path;

    /**
     * 创建时间
     */
    private String createdTime;

    /**
     * 父资源名称
     */
    @Length(max = 256)
    private String parentName;

    /**
     * 父资源uuid
     */
    @Length(max = 64)
    private String parentUuid;

    /**
     * 受保护环境uuid
     */
    @Length(max = 64)
    private String rootUuid;

    /**
     * 资源的来源: restore、livemount、autoscan、register
     */
    @Length(max = 16)
    private String sourceType;

    /**
     * 资源的版本信息
     */
    @Length(max = 64)
    private String version;

    public String getUuid() {
        return uuid;
    }

    public void setUuid(String uuid) {
        this.uuid = uuid;
    }

    public String getName() {
        return name;
    }

    public void setName(String name) {
        this.name = name;
    }

    public String getType() {
        return type;
    }

    public void setType(String type) {
        this.type = type;
    }

    public String getSubType() {
        return subType;
    }

    public void setSubType(String subType) {
        this.subType = subType;
    }

    public String getPath() {
        return path;
    }

    public void setPath(String path) {
        this.path = path;
    }

    public String getCreatedTime() {
        return createdTime;
    }

    public void setCreatedTime(String createdTime) {
        this.createdTime = createdTime;
    }

    public String getParentName() {
        return parentName;
    }

    public void setParentName(String parentName) {
        this.parentName = parentName;
    }

    public String getParentUuid() {
        return parentUuid;
    }

    public void setParentUuid(String parentUuid) {
        this.parentUuid = parentUuid;
    }

    public String getRootUuid() {
        return rootUuid;
    }

    public void setRootUuid(String rootUuid) {
        this.rootUuid = rootUuid;
    }

    public String getSourceType() {
        return sourceType;
    }

    public void setSourceType(String sourceType) {
        this.sourceType = sourceType;
    }

    public String getVersion() {
        return version;
    }

    public void setVersion(String version) {
        this.version = version;
    }
}
