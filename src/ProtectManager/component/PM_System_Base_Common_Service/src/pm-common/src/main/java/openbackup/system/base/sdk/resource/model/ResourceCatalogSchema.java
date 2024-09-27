/*
* This file is a part of the open-eBackup project.
* This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
* If a copy of the MPL was not distributed with this file, You can obtain one at
* http://mozilla.org/MPL/2.0/.
*
* Copyright (c) [2024] Huawei Technologies Co.,Ltd.
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*/
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
