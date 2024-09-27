/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2020. All rights reserved.
 */

package openbackup.system.base.common.model.storage;

import openbackup.system.base.common.constants.IsmNumberConstant;
import openbackup.system.base.common.enums.StorageConnectTypeEnum;
import openbackup.system.base.common.enums.StorageTypeEnum;

import lombok.Data;

import org.hibernate.validator.constraints.Length;

import javax.validation.constraints.Max;
import javax.validation.constraints.Min;

/**
 * 功能描述
 *
 * @author w00504341
 * @since 2020-12-18
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
