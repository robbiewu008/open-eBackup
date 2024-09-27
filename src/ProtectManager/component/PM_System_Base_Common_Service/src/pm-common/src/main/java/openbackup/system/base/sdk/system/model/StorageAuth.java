/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

package openbackup.system.base.sdk.system.model;

import lombok.AllArgsConstructor;
import lombok.Data;
import lombok.NoArgsConstructor;

import org.hibernate.validator.constraints.Length;

import javax.validation.constraints.NotEmpty;

/**
 * 功能描述
 *
 * @author y00413474
 * @since 2020-07-14
 */
@Data
@AllArgsConstructor
@NoArgsConstructor
public class StorageAuth {
    @NotEmpty
    @Length(max = 32, min = 1, message = "The length of description is 1 ~ 32")
    private String username;

    @NotEmpty
    @Length(max = 16, min = 1, message = "The length of description is 1 ~ 16")
    private String password;
}
