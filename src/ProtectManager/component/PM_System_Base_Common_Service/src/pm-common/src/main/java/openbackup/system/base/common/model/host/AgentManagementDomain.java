/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

package openbackup.system.base.common.model.host;

import lombok.AllArgsConstructor;
import lombok.Data;
import lombok.NoArgsConstructor;

/**
 * Agent当前控制器所在pod的信息
 *
 * @author l30044826
 * @since 2023-05-29
 */
@AllArgsConstructor
@NoArgsConstructor
@Data
public class AgentManagementDomain {
    /**
     * 拼接后的域名
     */
    private String domain;

    /**
     * 端口
     */
    private Integer port;
}
