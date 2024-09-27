/*
 *  Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

package openbackup.system.base.sdk.auth.model;

import lombok.Getter;
import lombok.Setter;

/**
 * 查询hcs manageOne管理员的token参数
 *
 * @author n30046257
 * @version [OceanProtect DataBackup 1.6.0]
 * @since 2024/7/30
 */
@Getter
@Setter
public class TokenParam {
    private String username;
    private String password;
    private String ip;
    private String domain;
    private int clusterId;
    private boolean isNeedNewToken = false;
}
