/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package openbackup.system.base.common.aspect;

import openbackup.system.base.common.exception.LegoCheckedException;

import lombok.AllArgsConstructor;
import lombok.Data;
import lombok.NoArgsConstructor;

/**
 * 事件用到的参数
 *
 * @author w30042425
 * @since 2023-10-10
 */
@Data
@NoArgsConstructor
@AllArgsConstructor
public class LogParam {
    private String target;

    private String[] details;

    private boolean isSuccess;

    private LegoCheckedException legoCheckedException;
}

