/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

package openbackup.data.access.framework.livemount.controller.livemount.model;

import lombok.Data;

import javax.validation.constraints.Size;

/**
 * VMWare 迁移请求参数
 *
 * @author h30003246
 * @since 2020-12-31
 */
@Data
public class Datastore {
    @Size(max = 128)
    private String moId;
}
