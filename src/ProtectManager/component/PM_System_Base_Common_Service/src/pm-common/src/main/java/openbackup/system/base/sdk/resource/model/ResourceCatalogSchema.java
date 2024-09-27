package openbackup.system.base.sdk.resource.model;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Data;

import java.util.List;

/**
 * 应用目录类型
 *
 * @author l00347293
 * @since 2021-01-04
 */
@Data
public class ResourceCatalogSchema {
    // 目录ID
    @JsonProperty("catalog_id")
    private String catalogId;

    // 目录名称
    @JsonProperty("catalog_name")
    private String catalogName;

    // 显示顺序
    @JsonProperty("display_order")
    private int displayOrder;

    // 是否隐藏
    @JsonProperty("show")
    private boolean isShow;

    // 父目录ID
    @JsonProperty("parent_id")
    private String parentId;

    // 子目录列表
    @JsonProperty("children")
    private List<ResourceCatalogSchema> children;

    // 标签
    @JsonProperty("label")
    private String label;

    // 资源目录对应的url
    @JsonProperty("link")
    private String link;
}
