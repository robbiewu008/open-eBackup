/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package openbackup.data.protection.access.provider.sdk.resource;

import lombok.Getter;
import lombok.Setter;

/**
 * 资源脱敏对象
 *
 * @author c30016231
 * @version [OceanProtect DataBackup 1.3.0]
 * @since 2023-04-08
 */
@Getter
@Setter
public class ResourceDesesitization {
    /**
     * 脱敏状态
     */
    private String desesitizationStatus;

    /**
     * 识别状态
     */
    private String identificationStatus;

    /**
     * 脱敏任务id
     */
    private String desesitizationJobId;

    /**
     * 识别任务id
     */
    private String identificationJobId;

    /**
     * 脱敏策略id
     */
    private String desesitizationPolicyId;

    /**
     * 脱敏策略名称
     */
    private String desesitizationPolicyName;
}
