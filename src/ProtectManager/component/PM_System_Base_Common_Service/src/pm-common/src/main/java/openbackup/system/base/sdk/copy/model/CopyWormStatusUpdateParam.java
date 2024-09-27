/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

package openbackup.system.base.sdk.copy.model;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Data;

/**
 * 修改副本WORM状态请求体
 *
 * @author w00616953
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-10-20
 */
@Data
public class CopyWormStatusUpdateParam {
    @JsonProperty("worm_status")
    private int wormStatus;

    public int getWormStatus() {
        return wormStatus;
    }

    public void setWormStatus(int wormStatus) {
        this.wormStatus = wormStatus;
    }
}
