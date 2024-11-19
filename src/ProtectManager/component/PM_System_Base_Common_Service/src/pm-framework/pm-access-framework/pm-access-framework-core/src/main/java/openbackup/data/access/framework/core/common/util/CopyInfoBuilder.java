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
package openbackup.data.access.framework.core.common.util;

import lombok.extern.slf4j.Slf4j;
import openbackup.data.access.framework.core.common.constants.CopyInfoConstants;
import openbackup.data.protection.access.provider.sdk.copy.CopyInfoBo;
import openbackup.data.protection.access.provider.sdk.enums.CopyFeatureEnum;
import openbackup.data.protection.access.provider.sdk.resource.Resource;
import openbackup.system.base.common.enums.RetentionTypeEnum;
import openbackup.system.base.common.enums.TimeUnitEnum;
import openbackup.system.base.common.enums.WormValidityTypeEnum;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.common.utils.json.JsonUtil;
import openbackup.system.base.sdk.protection.model.PolicyBo;
import openbackup.system.base.sdk.protection.model.RetentionBo;
import openbackup.system.base.sdk.protection.model.SlaBo;

import org.apache.commons.lang3.StringUtils;

import java.util.Calendar;
import java.util.Date;
import java.util.List;
import java.util.Map;

/**
 * 副本信息构造器
 *
 */
@Slf4j
public class CopyInfoBuilder {
    private static final int END_INDEX = 36;

    private static final long SECONDS_MILLI = 1000L;

    private CopyInfoBo copy = new CopyInfoBo();

    private CopyInfoBuilder() {
    }

    /**
     * 获取构造器
     *
     * @return CopyInfoBuilder
     */
    public static CopyInfoBuilder builder() {
        return new CopyInfoBuilder();
    }

    /**
     * build Retention Info
     *
     * @param copy copy
     * @param policyBo policy bo
     * @param timeStamp time stamp
     */
    public static void buildRetentionInfo(CopyInfoBo copy, PolicyBo policyBo, long timeStamp) {
        RetentionBo retention = policyBo.getRetention();
        copy.setRetentionType(RetentionTypeEnum.TEMPORARY.getType());
        copy.setRetentionDuration(retention.getRetentionDuration());
        copy.setDurationUnit(retention.getDurationUnit());
        copy.setExpirationTime(computeExpirationTime(timeStamp, TimeUnitEnum.getByUnit(retention.getDurationUnit()),
            retention.getRetentionDuration()));
    }

    /**
     * build worm Retention Info
     *
     * @param copy 副本
     * @param policyBo 策略数据
     * @param timeStamp time stamp
     */
    public static void buildWormRetentionInfo(CopyInfoBo copy, PolicyBo policyBo, long timeStamp) {
            Integer wormValidityType = policyBo.getWormValidityType();
            if (!VerifyUtil.isEmpty(wormValidityType)) {
                copy.setWormValidityType(wormValidityType);
            }
            if (wormValidityType == null || WormValidityTypeEnum.WORM_NOT_OPEN.getType().equals(wormValidityType)) {
                return;
            }
            RetentionBo retention = policyBo.getRetention();
            if (WormValidityTypeEnum.CUSTOM_RETENTION_TIME.getType().equals(wormValidityType)) {
                copy.setWormRetentionDuration(retention.getWormRetentionDuration());
                copy.setWormDurationUnit(retention.getWormDurationUnit());
            } else {
                copy.setWormDurationUnit(retention.getDurationUnit());
                if (!VerifyUtil.isEmpty(retention.getRetentionDuration())) {
                    copy.setWormRetentionDuration(retention.getRetentionDuration());
                }
            }
        if (!VerifyUtil.isEmpty(copy.getWormDurationUnit())) {
            copy.setWormExpirationTime(
                computeExpirationTime(timeStamp, TimeUnitEnum.getByUnit(copy.getWormDurationUnit()),
                    copy.getWormRetentionDuration()));
        }
    }

    /**
     * 计算副本过期时间
     *
     * @param time date
     * @param durationUnit 保留时间单位
     * @param duration 保留时间
     * @return Date 副本过期日期
     */
    public static Date computeExpirationTime(long time, TimeUnitEnum durationUnit, Integer duration) {
        Calendar calendar = Calendar.getInstance();
        calendar.setTimeInMillis(time);
        switch (durationUnit) {
            case MINUTES:
                calendar.add(Calendar.MINUTE, duration);
                break;
            case HOURS:
                calendar.add(Calendar.HOUR, duration);
                break;
            case DAYS:
                calendar.add(Calendar.HOUR, duration * 24);
                break;
            case WEEKS:
                calendar.add(Calendar.HOUR, duration * 7 * 24);
                break;
            case MONTHS:
                calendar.add(Calendar.MONTH, duration);
                break;
            case YEARS:
                calendar.add(Calendar.YEAR, duration);
                break;
            default:
        }
        return calendar.getTime();
    }

    /**
     * 设置副本信息基础参数
     *
     * @return CopyInfoBuilder
     */
    public CopyInfoBuilder setBaseCopyInfo() {
        copy.setLocation(CopyInfoConstants.COPY_INIT_LOCATION);
        copy.setIndexed(CopyInfoConstants.COPY_INIT_INDEXED);
        copy.setStatus(CopyInfoConstants.COPY_INIT_STATUS);
        return this;
    }

    /**
     * 设置副本位置
     *
     * @param location 位置
     * @return CopyInfoBuilder
     */
    public CopyInfoBuilder setLocation(String location) {
        copy.setLocation(location);
        return this;
    }

    /**
     * 设置副本信息是否可删除
     *
     * @param isCopyDeleteEnable 副本是否可删除
     * @return CopyInfoBuilder
     */
    public CopyInfoBuilder setDeletable(boolean isCopyDeleteEnable) {
        copy.setDeletable(isCopyDeleteEnable);
        return this;
    }

    /**
     * 设置副本uuid为backupId
     *
     * @param copyId 副本id
     * @return CopyInfoBuilder
     */
    public CopyInfoBuilder setCopyId(String copyId) {
        copy.setUuid(copyId);
        return this;
    }

    /**
     * 设置副本originCopyId为backupId
     *
     * @param originBackupId 原始副本备份id多集群场景复制场景使用
     * @return CopyInfoBuilder
     */
    public CopyInfoBuilder setOriginBackupId(String originBackupId) {
        copy.setOriginBackupId(originBackupId);
        return this;
    }

    /**
     * 设置副本生成的备份类型
     *
     * @param backupType 备份类型，1：完全备份 2：增量备份 3：差异备份 4：日志备份 5：永久增量备份
     * @return CopyInfoBuilder
     */
    public CopyInfoBuilder setBackupType(int backupType) {
        copy.setBackupType(backupType);
        return this;
    }

    /**
     * 设置用户id
     *
     * @param userId 用户Id
     * @return CopyInfoBuilder
     */
    public CopyInfoBuilder setUserId(String userId) {
        copy.setUserId(userId);
        return this;
    }

    /**
     * 设置eBackup时间戳
     *
     * @param timePoint 副本时间戳
     * @return builder
     */
    public CopyInfoBuilder setEBackupTimestamp(Long timePoint) {
        copy.setTimestamp(String.valueOf(timePoint * SECONDS_MILLI * SECONDS_MILLI));
        copy.setDisplayTimestamp(String.valueOf(timePoint));
        copy.setOriginCopyTimeStamp(String.valueOf(timePoint));
        return this;
    }

    /**
     * 设置副本支持的特性
     *
     * @param features 特性枚举类列表
     * @return CopyInfoBuilder
     */
    public CopyInfoBuilder setCopyFeature(List<CopyFeatureEnum> features) {
        Integer feature = CopyFeatureEnum.setAndGetFeatures(features);
        copy.setFeatures(feature);
        return this;
    }

    /**
     * 设置资源信息
     *
     * @param resource 资源信息
     * @return CopyInfoBuilder
     */
    public CopyInfoBuilder setResourceInfo(Resource resource) {
        copy.setResourceStatus("EXIST");
        copy.setResourceId(resource.getUuid());
        copy.setResourceName(resource.getName());
        copy.setResourceType(resource.getType());
        copy.setResourceSubType(resource.getSubType());
        String path = resource.getPath();
        copy.setResourceLocation(StringUtils.isBlank(path) ? resource.getEnvironmentName() : path);
        return this;
    }

    /**
     * 设置资源信息
     *
     * @param generatedBy generatedBy
     * @return CopyInfoBuilder
     */
    public CopyInfoBuilder setGeneratedBy(String generatedBy) {
        copy.setGeneratedBy(generatedBy);
        return this;
    }

    /**
     * 设置索引信息
     *
     * @param indexStatus indexStatus
     * @return CopyInfoBuilder
     */
    public CopyInfoBuilder setIndexed(String indexStatus) {
        copy.setIndexed(indexStatus);
        return this;
    }

    /**
     * 设置资源环境信息
     *
     * @param envEndPoint envEndPoint
     * @param envName envEndpoint
     * @return CopyInfoBuilder
     */
    public CopyInfoBuilder setEnvironmentInfo(String envName, String envEndPoint) {
        copy.setResourceEnvironmentName(envName);
        copy.setResourceEnvironmentIp(envEndPoint);
        return this;
    }

    /**
     * 设置副本保留信息
     *
     * @param sla sla
     * @param policyBo sla policy
     * @param timeStamp 副本生成时间
     * @return CopyInfoBuilder
     * */
    public CopyInfoBuilder setRetentionInfo(SlaBo sla, PolicyBo policyBo, long timeStamp) {
        copy.setSlaName(sla.getName());
        if (policyBo == null || RetentionTypeEnum.getByType(policyBo.getRetention().getRetentionType())
            .equals(RetentionTypeEnum.PERMANENT)) {
            // 执行的备份类型在SLA中未开启或者保留策略为永久保留
            copy.setRetentionType(RetentionTypeEnum.PERMANENT.getType());
        } else if (RetentionTypeEnum.getByType(policyBo.getRetention().getRetentionType())
            .equals(RetentionTypeEnum.TEMPORARY)) {
            // 执行的备份类型在SLA中开启，根据SLA计算副本有效期
            buildRetentionInfo(copy, policyBo, timeStamp);
        } else {
            // 按数量保留副本，保留时间与永久保留一致
            copy.setRetentionType(RetentionTypeEnum.QUANTITY.getType());
        }
        return this;
    }

    /**
     * 设置副本WORM保留信息
     *
     * @param sla sla
     * @param policyBo sla policy
     * @param timeStamp 副本生成时间
     * @return CopyInfoBuilder
     * */
    public CopyInfoBuilder setWormRetentionInfo(SlaBo sla, PolicyBo policyBo, long timeStamp) {
        // 执行的备份类型在SLA中开启，根据SLA计算副本有效期
        buildWormRetentionInfo(copy, policyBo, timeStamp);
        return this;
    }

    /**
     * 设置扩展参数
     *
     * @param properties 扩展参数
     * @return CopyInfoBuilder
     */
    public CopyInfoBuilder setProperties(Map<String, Object> properties) {
        copy.setProperties(JsonUtil.json(properties));
        return this;
    }

    /**
     * 设置其它信息
     *
     * @param chainId 副本链id
     * @param resourceStr 资源参数
     * @param slaStr sla 参数
     * @return CopyInfoBuilder
     */
    public CopyInfoBuilder setOtherInfo(String chainId, String resourceStr, String slaStr) {
        copy.setChainId(chainId);
        copy.setResourceProperties(resourceStr);
        copy.setSlaProperties(slaStr);
        return this;
    }

    /**
     * 设置副本名
     *
     * @param name 副本名
     * @return copyInfo 构造器
     */
    public CopyInfoBuilder setName(String name) {
        copy.setName(name);
        return this;
    }

    /**
     * 构造副本信息
     *
     * @return CopyInfo 副本信息对象
     */
    public CopyInfoBo build() {
        return copy;
    }

    /**
     * 设置所属存储库名称
     *
     * @param storageId 副本所属存储库
     * @return copyInfo 构造器
     */
    public CopyInfoBuilder setStorageId(String storageId) {
        copy.setStorageId(storageId);
        return this;
    }

    /**
     * 设置所属存储库名称
     *
     * @param sourceCopyType 副本所属存储库
     * @return copyInfo 构造器
     */
    public CopyInfoBuilder setSourceCopyType(int sourceCopyType) {
        copy.setSourceCopyType(sourceCopyType);
        return this;
    }

    /**
     * 设置副本所在设备esn
     *
     * @param deviceEsn 设备esn
     * @return 构造器
     */
    public CopyInfoBuilder setDeviceEsn(String deviceEsn) {
        copy.setDeviceEsn(deviceEsn);
        return this;
    }

    /**
     * 设置副本存储池id
     *
     * @param poolId 存储池id
     * @return 构造器
     */
    public CopyInfoBuilder setPoolId(String poolId) {
        copy.setPoolId(poolId);
        return this;
    }

    /**
     * 设置副本storageUnitId
     *
     * @param storageUnitId String
     * @return 构造器
     */
    public CopyInfoBuilder setStorageUnitId(String storageUnitId) {
        copy.setStorageUnitId(storageUnitId);
        return this;
    }
}
