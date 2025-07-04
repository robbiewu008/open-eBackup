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
package openbackup.data.access.framework.protection.service.replication;

import com.huawei.oceanprotect.base.cluster.sdk.entity.TargetCluster;
import com.huawei.oceanprotect.base.cluster.sdk.service.ArrayTargetClusterService;
import com.huawei.oceanprotect.base.cluster.sdk.service.ClusterQueryService;

import lombok.extern.slf4j.Slf4j;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.sdk.anti.api.AntiRansomwareApi;
import openbackup.system.base.sdk.anti.model.AntiRansomwarePolicyRes;
import openbackup.system.base.sdk.anti.model.AntiRansomwareScheduleRes;
import openbackup.system.base.sdk.cluster.enums.ClusterEnum;
import openbackup.system.base.sdk.license.LicenseServiceApi;
import openbackup.system.base.sdk.license.enums.FunctionEnum;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Service;

import java.util.Date;

/**
 * 复制服务
 *
 */
@Service
@Slf4j
public class ReplicationService {
    private static final long SECONDS_MILLI = 1000L;

    /**
     * 9999-12-31 23:59:59 秒值
     */
    private static final long MAX_TIME_STAMP = 253402271999L;

    @Autowired
    private LicenseServiceApi licenseServiceApi;

    @Autowired
    private AntiRansomwareApi antiRansomwareApi;

    @Autowired
    private ClusterQueryService clusterQueryService;

    @Autowired
    private ArrayTargetClusterService arrayTargetClusterService;

    /**
     * 校验License
     *
     * @param isOriginCopyWorm 原副本是否是worm
     * @param schedule 防勒索&worm策略
     */
    public void checkDetectionLicense(boolean isOriginCopyWorm, AntiRansomwareScheduleRes schedule) {
        log.info("replication task check anti license, isOriginCopyWorm: {}, isTargetSetWorm:{}, isTargetNeedDetect:{}",
            isOriginCopyWorm, schedule.isSetWorm(), schedule.isNeedDetect());
        // 原副本设置worm或复制副本开启worm，同时开启防勒索检测，校验防勒索License，提示未设置worm原因
        if ((isOriginCopyWorm || schedule.isSetWorm()) && schedule.isNeedDetect()) {
            // 开启防勒索检测，需要校验licence
            licenseServiceApi.functionLicense(FunctionEnum.DATA_PROTECT_ANTI_MALWARE.name(), "");
        }
    }

    /**
     * 获取副本生成时间
     *
     * @param generatedTime 副本生成时间
     * @return 副本生成时间
     */
    public Date getGenerateTime(long generatedTime) {
        log.info("get generated time from param:{}", generatedTime);
        Date date;
        if (generatedTime > 0 && generatedTime < MAX_TIME_STAMP) {
            date = new Date(generatedTime * SECONDS_MILLI);
        } else {
            date = new Date(System.currentTimeMillis());
        }
        return date;
    }

    /**
     * 查询资源是否设置worm策略,如果开启防勒索，进行License校验
     *
     * @param resourceId 资源id
     * @return 防勒索&worm策略
     */
    public AntiRansomwareScheduleRes getAntiRansomwareSchedule(String resourceId) {
        AntiRansomwarePolicyRes antiRansomwarePolicyRes = antiRansomwareApi.getPolicyByResourceId(resourceId);
        AntiRansomwareScheduleRes schedule = antiRansomwarePolicyRes.getSchedule();
        if (schedule == null) {
            log.info("check worm resourceId id: {} policy is null.", resourceId);
            return new AntiRansomwareScheduleRes();
        }
        return schedule;
    }

    /**
     * 校验手动复制选的存储单元是否符合要求
     *
     * @param clusterId clusterId
     * @param storageType storageType
     * @param storageId storageId
     * @param resourceId resourceId
     */
    public void checkManualRep(Integer clusterId, String storageType, String storageId, String resourceId) {
        TargetCluster targetClusterById = clusterQueryService.getTargetClusterById(clusterId);
        if (ClusterEnum.StatusEnum.ONLINE.getStatus() != targetClusterById.getStatus()) {
            throw new LegoCheckedException(CommonErrorCode.REPLICATION_CLUSTER_AUTH_FAILED,
                "Replica Cluster is not Online.");
        }
        arrayTargetClusterService.checkBeforeManualReplication(targetClusterById, storageType, storageId, resourceId);
    }
}
