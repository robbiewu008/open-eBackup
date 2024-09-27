/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2020. All rights reserved.
 */

package openbackup.system.base.sdk.kmc.model;

import lombok.Data;

import javax.validation.constraints.NotEmpty;

/**
 * 功能描述
 *
 * @author y00413474
 * @version [BCManager 8.0.0]
 * @since 2020-05-30
 */
@Data
public class CiphertextVo {
    @NotEmpty(message = "ciphertext cannot be empty.")
    private String ciphertext;
}
