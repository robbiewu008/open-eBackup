/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

package openbackup.system.base.sdk.protection.model;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Getter;
import lombok.Setter;

/**
 * 更新SLA返回值Bo
 *
 * @author z30006621
 * @since 2021-08-27
 */
@Getter
@Setter
public class SlaUpdateBo {
    /**
     * SLA UUID
     */
    @JsonProperty("uuid")
    private String slaId;
}
