/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

package openbackup.system.base.sdk.auth.model.request;

import lombok.Getter;
import lombok.Setter;

/**
 * HCS云备份密码
 *
 * @author l30044826
 * @since 2023-07-07
 */
@Getter
@Setter
public class Password {
    private User user;
}
