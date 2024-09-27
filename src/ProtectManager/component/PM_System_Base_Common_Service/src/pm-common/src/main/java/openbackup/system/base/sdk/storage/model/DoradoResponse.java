/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 */

package openbackup.system.base.sdk.storage.model;

import lombok.Data;

/**
 * 功能描述
 *
 * @param <T> 数据类型
 * @author y00413474
 * @since 2020-07-01
 */
@Data
public class DoradoResponse<T> {
    /**
     * 数据
     */
    private T data;

    /**
     * 操作结果
     */
    private Result result;
}