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
package openbackup.system.base.common.model.storage;

import lombok.Data;

import java.math.BigDecimal;

/**
 * 存储库完整信息回复
 *
 */
@Data
public class CompleteStorageInfoRes {
    private String storageName;

    private String endpoint;

    private String certName;

    private String certId;

    private int type;

    private int cloudType;

    private int port;

    private Integer connectType;

    private String ak;

    private String bucketName;

    private String indexBucketName;

    private boolean proxyEnable;

    private String proxyHostName;

    private String proxyUserName;

    private boolean useHttps;

    private boolean alarmEnable;

    private long alarmThreshold;

    private String noSensitiveSk;

    /**
     * 容量告警单位
     * 取值定义请见 @see com.huawei.oceanprotect.repository.common.enums.RepositoryThresholdAlarmUnit
     */
    private String alarmLimitValueUnit;

    private int status;

    private BigDecimal totalSize;

    private BigDecimal usedSize;

    private BigDecimal freeSize;
}
