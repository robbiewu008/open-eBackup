/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package openbackup.system.base.ssh;

import lombok.AllArgsConstructor;
import lombok.Data;
import lombok.NoArgsConstructor;

/**
 * 功能描述
 *
 * @author w00504341
 * @since 2023-07-09
 */
@Data
@NoArgsConstructor
@AllArgsConstructor
public class CmdExecRes {
    /**
     * 错误码
     */
    private int code;

    /**
     * 错误信息
     */
    private String errInfo;
}
