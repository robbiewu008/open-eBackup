/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2024. All rights reserved.
 */

package openbackup.access.framework.resource.persistence.model;

import openbackup.system.base.query.PageQueryConfig;

import lombok.Data;

/**
 * 资源组查询参数
 *
 * @author l00853347
 * @version [OceanProtect DataBackup 1.7.0]
 * @since 2024-06-04
 */
@Data
@PageQueryConfig(conditions = {"resource_set_id"})
public class ResourceGroupExcludePo extends ResourceGroupPo {
    private String resourceSetId;
}
