/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

package openbackup.system.base.common.constants;

import lombok.AllArgsConstructor;
import lombok.Data;
import lombok.NoArgsConstructor;

/**
 * ClientCertGenResult
 *
 * @author wx1011919
 * @since 2021-08-16
 */
@Data
@AllArgsConstructor
@NoArgsConstructor
public class ClientCertGenResult {
    private int status;
    private String uuid;
}
