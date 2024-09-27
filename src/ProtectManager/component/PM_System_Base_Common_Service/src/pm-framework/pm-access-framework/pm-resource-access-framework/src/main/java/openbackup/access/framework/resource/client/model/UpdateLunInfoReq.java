/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

package openbackup.access.framework.resource.client.model;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.AllArgsConstructor;
import lombok.Data;
import lombok.NoArgsConstructor;

import java.util.List;

/**
 * 功能描述
 *
 * @author c30058517
 * @since 2024-06-28
 */
@Data
@AllArgsConstructor
@NoArgsConstructor
public class UpdateLunInfoReq {
    /**
     * LUN信息列表
     */
    @JsonProperty("lun_info")
    private List<LunInfo> lunInfoList;
}
