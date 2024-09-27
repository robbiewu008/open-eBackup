/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package openbackup.data.access.client.sdk.api.framework.dme.replicate.model;

import lombok.AllArgsConstructor;
import lombok.Data;
import lombok.NoArgsConstructor;

import java.util.List;

/**
 * 接收复制任务操作返回信息对象
 *
 * @author y30037959
 * @since 2023-01-31
 */
@Data
@NoArgsConstructor
@AllArgsConstructor
public class ReceiveJobActionInfo {
    /**
     * 执行中的复制任务id集合
     */
    private List<String> jobIds;
}
