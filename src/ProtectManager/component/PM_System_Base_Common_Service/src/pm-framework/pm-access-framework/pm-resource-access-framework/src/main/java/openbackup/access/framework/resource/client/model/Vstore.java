/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

package openbackup.access.framework.resource.client.model;

import lombok.AllArgsConstructor;
import lombok.Builder;
import lombok.Data;
import lombok.NoArgsConstructor;

/**
 * 租户信息
 *
 * @author s30031954
 * @since 2022-12-29
 */
@Data
@Builder
@NoArgsConstructor
@AllArgsConstructor
public class Vstore {
    /**
     * 租户Id
     */
    private String vstoreId;

    /**
     * 租户名称
     */
    private String vstoreName;
}