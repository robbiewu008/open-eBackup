/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 */

package openbackup.system.base.sdk.cluster.model.storage;

import static openbackup.system.base.common.constants.IsmNumberConstant.FOUR;
import static openbackup.system.base.common.constants.IsmNumberConstant.SIXTY_FOUR;

import openbackup.system.base.common.validator.constants.RegexpConstants;

import lombok.AllArgsConstructor;
import lombok.Getter;
import lombok.NoArgsConstructor;
import lombok.Setter;

import org.hibernate.validator.constraints.Length;

import javax.validation.constraints.Pattern;

/**
 * 存储单元接口请求类
 *
 * @author w00639094
 * @version [OceanProtect DataBackup 1.6.0]
 * @since 2024-01-06
 */
@Getter
@Setter
@NoArgsConstructor
@AllArgsConstructor
public class StorageUnitRequest {
    @Length(max = SIXTY_FOUR, min = FOUR, message = "The length of storage unit name is 4-64 characters")
    @Pattern(regexp = RegexpConstants.NAME_STR, message = "The storage unit name is invalid")
    private String name;

    @Length(max = 1024)
    private String deviceId;

    @Length(max = 256)
    private String poolId;

    @Length(max = 64)
    private String deviceType;

    private Boolean isAutoAdded;

    @Length(max = 64)
    private String threshold;
}
