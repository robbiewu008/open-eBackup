/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

package openbackup.data.access.framework.livemount.common.model;

import openbackup.system.base.common.model.livemount.LiveMountEntity;
import openbackup.system.base.sdk.copy.model.Copy;

import lombok.Data;

/**
 * Live Mount Clone Request
 *
 * @author l00272247
 * @since 2021-04-12
 */
@Data
public class LiveMountCloneRequest {
    private Copy sourceCopy;
    private String targetCopyUuid;
    private LiveMountEntity liveMountEntity;
}
