/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package openbackup.system.base.sdk.job.model.request;

import openbackup.system.base.common.validator.constants.RegexpConstants;

import lombok.Getter;
import lombok.Setter;

import java.util.List;
import java.util.Map;

import javax.validation.constraints.NotEmpty;
import javax.validation.constraints.Pattern;
import javax.validation.constraints.Size;

/**
 * 高级配置，k8s恢复才有
 *
 * @author y30046482
 * @since 2023-10-18
 */
@Setter
@Getter
public class AdvancedConfigReq {
    // 工作负载类型
    @NotEmpty
    @Size(min = 1, max = 50)
    private String workLoadType;

    // 工作负载名称
    @NotEmpty
    @Size(min = 1, max = 50)
    @Pattern(regexp = RegexpConstants.NAME_STR)
    private String workLoadName;

    // 容器名称
    @NotEmpty
    @Size(min = 1, max = 50)
    @Pattern(regexp = RegexpConstants.NAME_STR)
    private String containerName;

    // 环境变量
    @Size(min = 1, max = 10)
    @NotEmpty
    private List<Map<String, String>> envMap;
}
