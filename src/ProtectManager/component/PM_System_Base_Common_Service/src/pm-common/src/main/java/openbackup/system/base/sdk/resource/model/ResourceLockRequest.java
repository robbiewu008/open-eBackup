/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package openbackup.system.base.sdk.resource.model;

import lombok.AllArgsConstructor;
import lombok.Builder;
import lombok.Data;
import lombok.NoArgsConstructor;

import org.hibernate.validator.constraints.Length;
import org.springframework.validation.annotation.Validated;

import java.util.List;

import javax.validation.constraints.Size;

/**
 * 资源Redis冗余锁
 *
 * @author w30044259
 * @version [OceanProtect DataBackup 1.5.0]
 * @since 2023-08-14
 */
@Data
@NoArgsConstructor
@AllArgsConstructor
@Builder
@Validated
public class ResourceLockRequest {
    @Size(max = 4200)
    private List<ResourceLockEntity> resources;

    @Length(max = 256)
    private String lockId;
}
