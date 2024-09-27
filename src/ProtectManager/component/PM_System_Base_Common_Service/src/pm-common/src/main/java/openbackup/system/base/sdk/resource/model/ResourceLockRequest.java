/*
* This file is a part of the open-eBackup project.
* This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
* If a copy of the MPL was not distributed with this file, You can obtain one at
* http://mozilla.org/MPL/2.0/.
*
* Copyright (c) [2024] Huawei Technologies Co.,Ltd.
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
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
