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
import openbackup.system.base.common.constants.IsmNumberConstant;
import openbackup.system.base.common.enums.CloudCheckTypeEnum;
import openbackup.system.base.common.enums.StorageConnectTypeEnum;
import openbackup.system.base.common.enums.StorageTypeEnum;

import org.hibernate.validator.constraints.Length;

import javax.validation.constraints.Max;
import javax.validation.constraints.Min;

/**
 * 功能描述
 *
 */
@Data
public class StorageRequest {
    @Length(max = IsmNumberConstant.THROUND_TWENTY_FOUR, message = "Storage id is longer than 1024.")
    private String storageId;

    @Length(max = IsmNumberConstant.THROUND_TWENTY_FOUR, message = "cert id is longer than 1024.")
    private String certId;

    private String certName;

    private String storageName;

    private String endpoint;

    private int type;

    /**
     * S3存储类型：</br>
     * 0 - OceanStor Pacific </br>
     * 1 - FusionStorage OBS </br>
     * 3 - 华为云 OBS </br>
     * 4 - AWS S3 </br>
     */
    @Min(value = IsmNumberConstant.ZERO)
    @Max(value = IsmNumberConstant.FIVE)
    private int cloudType;

    private String ak;

    private String sk;

    private String bucketName;

    private String indexBucketName;

    private boolean proxyEnable;

    private String proxyHostName;

    private String proxyUserName;

    private String proxyUserPwd;

    private boolean useHttps;

    private boolean alarmEnable;

    private long alarmThreshold;

    private String alarmLimitValueUnit;

    /**
     * 端口号
     */
    private Integer port;

    /**
     * 连接类型，可以为空
     * 0 - 标准模式
     * 1 - 连接字符串模式
     * 兼容以前的设计，这个字段可以为空
     */
    private Integer connectType;

    /**
     * 检查类型，默认为0
     * 0 - 定时任务检查对象存储连通性
     * 1 - 添加或修改时检查对象存储连通性
     */
    private Integer cloudCheckType = CloudCheckTypeEnum.CLOUD_STORAGE_SCHEDULE_CHECK.getType();

    /**
     * 是否是Azure Blob类型
     *
     * @return 是 or 否
     */
    public boolean isAzureBlob() {
        return cloudType == StorageTypeEnum.AZURE_BLOB.getStorageType();
    }

    /**
     * 是否是连接字符串模式
     *
     * @return 是 or 否
     */
    public boolean isConnectInfo() {
        if (connectType == null) {
            return false;
        }
        return connectType == StorageConnectTypeEnum.CONNECT_INFO.getConnectType();
    }
}
