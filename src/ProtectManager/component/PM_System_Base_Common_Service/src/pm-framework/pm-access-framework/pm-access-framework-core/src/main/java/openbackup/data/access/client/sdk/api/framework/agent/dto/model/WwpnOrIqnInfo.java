/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package openbackup.data.access.client.sdk.api.framework.agent.dto.model;

import lombok.Data;

/**
 * 调用agent查询到的AIX主机和SanClient主机WWPN或IQN信息类
 *
 * @author z00613137
 * @since 2023-06-27
 */
@Data
public class WwpnOrIqnInfo {
    /**
     * WWPN值或IQN值
     */
    private String configKey;

    /**
     * 在线状态 27在线，28离线
     */
    private String configValue;
}