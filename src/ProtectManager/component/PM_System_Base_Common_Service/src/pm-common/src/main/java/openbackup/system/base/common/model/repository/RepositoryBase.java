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
package openbackup.system.base.common.model.repository;

import openbackup.system.base.common.model.storage.S3StorageStatusResponse;

import lombok.Data;

import java.math.BigDecimal;
import java.util.List;

/**
 * 存储库基类
 *
 * @author y00413474
 * @version [BCManager 8.0.0]
 * @since 2020-06-19
 */
@Data
public class RepositoryBase {
    private String repositoryId;

    private String name;

    private BigDecimal totalSize;

    private BigDecimal usedSize;

    private BigDecimal freeSize;

    /**
     * 存储库类型
     * 具体定义请参考{@link RepositoryType}
     */
    private Integer type;

    private Integer status;

    private Float alarmThreashold;

    private int cloudType;

    private List<S3StorageStatusResponse> s3StorageStatusResponses;
}
