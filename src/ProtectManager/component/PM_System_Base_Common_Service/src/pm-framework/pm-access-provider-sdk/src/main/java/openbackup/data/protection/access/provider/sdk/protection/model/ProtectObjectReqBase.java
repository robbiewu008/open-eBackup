/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package openbackup.data.protection.access.provider.sdk.protection.model;

import lombok.Getter;
import lombok.Setter;

import org.hibernate.validator.constraints.Length;

import java.util.Map;

/**
 * 保护请求体base类
 *
 * @author h30027154
 * @version [OceanProtect DataBackup 1.6.0]
 * @since 2023/11/10
 */
@Getter
@Setter
public class ProtectObjectReqBase {
    @Length(max = 256)
    private String slaId;
    private Map<String, Object> extParameters;
    @Length(max = 256)
    private String jobId;
    @Length(max = 256)
    private String originSlaId;
}
