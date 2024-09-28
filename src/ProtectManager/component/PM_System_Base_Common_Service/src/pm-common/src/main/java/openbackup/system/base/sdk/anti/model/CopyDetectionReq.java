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
package openbackup.system.base.sdk.anti.model;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Data;

import org.hibernate.validator.constraints.Length;

import javax.validation.constraints.NotBlank;

/**
 * 副本防勒索检测入参
 *
 */
@Data
public class CopyDetectionReq {
    /**
     * 副本ID
     */
    @NotBlank
    @Length(min = 1, max = 64)
    private String copyId;

    /**
     * 未感染快照设置安全快照
     */
    @JsonProperty("isSecuritySnap")
    private boolean isSecuritySnap;

    /**
     * 未感染快照设置安全快照
     */
    @JsonProperty("isBackupDetectEnable")
    private boolean isBackupDetectEnable;

    /**
     * 熵值侦测感染阈值
     */
    @JsonProperty("upperBound")
    private int upperBound;
}
