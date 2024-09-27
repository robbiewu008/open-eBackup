/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package openbackup.system.base.sdk.anti.model;

import lombok.AllArgsConstructor;
import lombok.Data;
import lombok.NoArgsConstructor;

import org.hibernate.validator.constraints.Length;

import java.util.List;

import javax.validation.constraints.NotNull;

/**
 * 批量查询防勒索策略请求
 *
 * @author j00619968
 * @since 2023-09-05
 */
@Data
@AllArgsConstructor
@NoArgsConstructor
public class BatchQueryAntiRansomwarePolicyReq {
    /**
     * 资源ID列表
     */
    @NotNull
    List<@Length(min = 1, max = 64) String> resourceIds;
}
