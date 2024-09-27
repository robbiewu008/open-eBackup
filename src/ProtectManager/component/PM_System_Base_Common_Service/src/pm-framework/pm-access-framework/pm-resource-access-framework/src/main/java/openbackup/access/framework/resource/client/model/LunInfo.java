/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

package openbackup.access.framework.resource.client.model;

import lombok.AllArgsConstructor;
import lombok.Data;
import lombok.NoArgsConstructor;

/**
 * Lun信息
 *
 * @author c30058517
 * @since 2024-06-28
 */
@Data
@AllArgsConstructor
@NoArgsConstructor
public class LunInfo {
    /**
     * LunID
     */
    private String lunId;

    /**
     * lun名称
     */
    private String lunName;

    /**
     * 租户ID
     */
    private String vstoreId;

    /**
     * 租户名称
     */
    private String vstoreName;
}
