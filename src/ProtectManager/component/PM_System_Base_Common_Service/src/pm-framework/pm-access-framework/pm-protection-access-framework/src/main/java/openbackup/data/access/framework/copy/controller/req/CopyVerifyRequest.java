/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package openbackup.data.access.framework.copy.controller.req;

import lombok.Getter;
import lombok.Setter;

import org.hibernate.validator.constraints.Length;

/**
 * 副本校验请求体
 *
 * @author c30016231
 * @version [OceanProtect DataBackup 1.3.0]
 * @since 2023-05-30
 */
@Getter
@Setter
public class CopyVerifyRequest {
    @Length(max = 256)
    private String userId;

    private String agents;
}
