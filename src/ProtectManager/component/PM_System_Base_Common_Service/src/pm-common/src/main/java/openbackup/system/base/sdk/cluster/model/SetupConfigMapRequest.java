/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package openbackup.system.base.sdk.cluster.model;

import openbackup.system.base.config.configmap.ConfigMapReq;

import lombok.Getter;
import lombok.Setter;

import javax.validation.constraints.NotNull;
import javax.validation.constraints.Size;

/**
 * 修改configMap请求体
 *
 * @author x30046484
 * @since 2023-05-18
 */
@Getter
@Setter
public class SetupConfigMapRequest {
    @NotNull
    private ConfigMapReq configMapReq;

    @NotNull
    @Size(max = 256)
    private String operationType;
}
