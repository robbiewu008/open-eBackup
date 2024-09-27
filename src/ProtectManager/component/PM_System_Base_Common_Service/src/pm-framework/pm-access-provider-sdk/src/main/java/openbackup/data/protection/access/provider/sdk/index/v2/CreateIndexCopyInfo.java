package openbackup.data.protection.access.provider.sdk.index.v2;

import com.fasterxml.jackson.annotation.JsonProperty;

/**
 * 创建索引的副本信息
 *
 * @author lWX776769
 * @version [OceanProtect A8000 1.1.0]
 * @since 2021-12-29
 */
public class CreateIndexCopyInfo {
    // 副本ID
    private String uuid;

    // 副本生产方式
    private String generatedBy;

    // 副本是否聚合
    @JsonProperty("isAggregation")
    private boolean isAggregation;

    // 副本是否索引
    @JsonProperty("isIndexed")
    private boolean isIndexed;

    // 资源ID
    private String resourceId;

    // 资源名称
    private String resourceName;

    // 副本资源子类型
    private String resourceSubType;

    // 副本gn号
    private int gn;

    // 备份链ID
    private String chainId;

    // 副本用户ID
    private String userId;

    // 副本时间
    private String displayTimestamp;

    // 副本资源平台类型
    private String resourcePlatform;

    // 设备esn
    private String esn;

    public String getUuid() {
        return uuid;
    }

    public void setUuid(String uuid) {
        this.uuid = uuid;
    }

    public String getGeneratedBy() {
        return generatedBy;
    }

    public void setGeneratedBy(String generatedBy) {
        this.generatedBy = generatedBy;
    }

    public boolean isAggregation() {
        return isAggregation;
    }

    public void setAggregation(boolean isAggregation) {
        this.isAggregation = isAggregation;
    }

    public boolean isIndexed() {
        return isIndexed;
    }

    public void setIndexed(boolean isIndexed) {
        this.isIndexed = isIndexed;
    }

    public String getResourceId() {
        return resourceId;
    }

    public void setResourceId(String resourceId) {
        this.resourceId = resourceId;
    }

    public String getResourceName() {
        return resourceName;
    }

    public void setResourceName(String resourceName) {
        this.resourceName = resourceName;
    }

    public String getResourceSubType() {
        return resourceSubType;
    }

    public void setResourceSubType(String resourceSubType) {
        this.resourceSubType = resourceSubType;
    }

    public int getGn() {
        return gn;
    }

    public void setGn(int gn) {
        this.gn = gn;
    }

    public String getChainId() {
        return chainId;
    }

    public void setChainId(String chainId) {
        this.chainId = chainId;
    }

    public String getUserId() {
        return userId;
    }

    public void setUserId(String userId) {
        this.userId = userId;
    }

    public String getDisplayTimestamp() {
        return displayTimestamp;
    }

    public void setDisplayTimestamp(String displayTimestamp) {
        this.displayTimestamp = displayTimestamp;
    }

    public String getResourcePlatform() {
        return resourcePlatform;
    }

    public void setResourcePlatform(String resourcePlatform) {
        this.resourcePlatform = resourcePlatform;
    }

    public void setEsn(String esn) {
        this.esn = esn;
    }

    public String getEsn() {
        return esn;
    }
}
