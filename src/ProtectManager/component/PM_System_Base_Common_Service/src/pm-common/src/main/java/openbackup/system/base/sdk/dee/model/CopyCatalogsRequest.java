/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

package openbackup.system.base.sdk.dee.model;

import lombok.Data;

/**
 * 副本相关的信息
 *
 * @author jwx701567
 * @version [OceanProtect A8000 1.1.0]
 * @since 2021-12-25
 */
@Data
public class CopyCatalogsRequest {
    private FinegrainedRestoreCopy copyInfo;

    private String parentPath;

    private String name;

    private Integer pageSize;

    private Integer pageNum;

    private String conditions;
}

