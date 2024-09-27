/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2020. All rights reserved.
 */

package openbackup.system.base.sdk.livemount.model;

import lombok.Data;

/**
 * Refresh Target Resource Param
 *
 * @author l00272247
 * @since 2020-10-24
 */
@Data
public class RefreshTargetResourceParam {
    private String targetEnvironmentId;

    private String targetResourceName;
}
