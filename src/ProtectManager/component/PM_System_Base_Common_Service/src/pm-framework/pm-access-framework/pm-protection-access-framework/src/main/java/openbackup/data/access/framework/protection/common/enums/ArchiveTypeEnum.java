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
package openbackup.data.access.framework.protection.common.enums;

import openbackup.data.access.framework.copy.mng.enums.CopyTypeEnum;
import openbackup.data.access.framework.core.common.util.CopyInfoBuilder;
import openbackup.data.access.framework.protection.common.constants.ArchivePolicyKeyConstant;
import openbackup.data.protection.access.provider.sdk.enums.RepositoryProtocolEnum;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.enums.RetentionTypeEnum;
import openbackup.system.base.common.enums.TimeUnitEnum;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.sdk.copy.model.CopyInfo;
import openbackup.system.base.sdk.protection.model.PolicyBo;
import openbackup.system.base.sdk.protection.model.RetentionBo;

import com.fasterxml.jackson.databind.JsonNode;

import lombok.Getter;
import lombok.extern.slf4j.Slf4j;

import java.util.ArrayList;
import java.util.Date;
import java.util.List;

/**
 * 归档副本类型
 *
 * @author z30006621
 * @since 2021-09-02
 */
@Slf4j
@Getter
public enum ArchiveTypeEnum {
    /**
     * 归档所有副本
     */
    ALL_COPY(1) {
        /**
         * 处理归档所有副本
         *
         * @param copy 副本信息
         * @param policyBo 归档策略
         * @return 处理后的副本信息
         */
        @Override
        public CopyInfo handleCopy(CopyInfo copy, PolicyBo policyBo) {
            log.info("[ARCHIVE_TASK] Archive all copies.");
            // 执行的备份类型在SLA中开启，根据SLA计算副本有效期
            RetentionBo retention = policyBo.getRetention();
            copy.setRetentionType(RetentionTypeEnum.TEMPORARY.getType());

            // 归档所有
            copy.setRetentionDuration(retention.getRetentionDuration());
            copy.setDurationUnit(retention.getDurationUnit());

            copy.setExpirationTime(CopyInfoBuilder.computeExpirationTime(new Date().getTime(),
                TimeUnitEnum.getByUnit(retention.getDurationUnit()), retention.getRetentionDuration()));
            return copy;
        }
    },

    /**
     * 归档指定副本
     */
    SPECIFIED_COPY(2) {
        /**
         * 处理指定副本
         *
         * @param copy 副本信息
         * @param policyBo 归档策略
         * @return 处理后的副本信息
         */
        @Override
        public CopyInfo handleCopy(CopyInfo copy, PolicyBo policyBo) {
            log.info("[ARCHIVE_TASK] Get archive tape policy copy={}.", policyBo.getUuid());
            if (policyBo.getExtParameters().get(ArchivePolicyKeyConstant.PROTOCOL_KEY).asInt()
                == RepositoryProtocolEnum.TAPE.getProtocol()) {
                copy.setRetentionType(RetentionTypeEnum.PERMANENT.getType());
                copy.setExpirationTime(null);
                return copy;
            }
            log.info("[ARCHIVE_TASK] Archive specify date copies.");
            copy.setRetentionType(RetentionTypeEnum.TEMPORARY.getType());
            List<SpecifiedScope> specifiedScopes = new ArrayList<>();
            for (JsonNode jsonNode : policyBo.getExtParameters().get("specified_scope")) {
                String copyType = jsonNode.get("copy_type").asText();
                int retentionDuration = jsonNode.get("retention_duration").asInt();
                specifiedScopes.add(new SpecifiedScope(CopyTypeEnum.getByType(copyType), retentionDuration));
            }

            if (specifiedScopes.isEmpty()) {
                throw new LegoCheckedException(CommonErrorCode.ERR_PARAM, "Can not find retention policy");
            }
            log.info("[ARCHIVE_TASK] Get specified scopes(nums: {}) success.", specifiedScopes.size());

            specifiedScopes.sort((o1, o2) -> o2.getDays() - o1.getDays());
            SpecifiedScope specifiedScope = specifiedScopes.get(0);
            copy.setDurationUnit(specifiedScope.getTimeUnit().getUnit());
            copy.setRetentionDuration(specifiedScope.getRetentionDuration());
            log.info("[ARCHIVE_TASK] Set retention duration success.");
            copy.setExpirationTime(CopyInfoBuilder.computeExpirationTime(new Date().getTime(),
                TimeUnitEnum.getByUnit(copy.getDurationUnit()), copy.getRetentionDuration()));
            return copy;
        }
    };

    private final Integer archiveType;

    ArchiveTypeEnum(Integer archiveType) {
        this.archiveType = archiveType;
    }

    /**
     * 处理归档副本
     *
     * @param copyInfo 副本信息
     * @param policyBo 归档策略
     * @return 副本信息
     */
    public abstract CopyInfo handleCopy(CopyInfo copyInfo, PolicyBo policyBo);

    /**
     * 获取副本类型
     *
     * @return archiveType
     */
    public Integer getType() {
        return archiveType;
    }

    /**
     * 匹配获取归档类型
     *
     * @param archiveType 归档类型
     * @return 匹配到的归档类型
     */
    public static ArchiveTypeEnum getArchiveTypeEnum(int archiveType) {
        for (ArchiveTypeEnum archiveTypeEnum : values()) {
            if (archiveTypeEnum.getArchiveType() == archiveType) {
                return archiveTypeEnum;
            }
        }
        throw new LegoCheckedException(CommonErrorCode.ERR_PARAM);
    }
}
