/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package openbackup.system.base.sdk.auth.model.request;

import static openbackup.system.base.common.constants.LegoNumberConstant.THIRTY_TWO;
import static openbackup.system.base.common.constants.LegoNumberConstant.THROUND_TWENTY_FOUR;
import static openbackup.system.base.common.constants.LegoNumberConstant.TWO_HUNDRED_FIFTY_SIX;

import lombok.Getter;
import lombok.Setter;

import org.hibernate.validator.constraints.Length;

import javax.validation.constraints.NotNull;

/**
 * 预置账号request
 *
 * @author y30021475
 * @since 2023-08-07
 */
@Getter
@Setter
public class PresetAccountRequest {
    @Length(max = TWO_HUNDRED_FIFTY_SIX)
    @NotNull
    private String userName;

    @Length(max = THROUND_TWENTY_FOUR)
    @NotNull
    private String userPwd;

    @Length(max = THIRTY_TWO)
    @NotNull
    private String sourceType;
}
