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
package openbackup.system.base.service;

import com.google.common.collect.ImmutableList;

import lombok.extern.slf4j.Slf4j;
import openbackup.system.base.common.enums.DeployTypeEnum;
import openbackup.system.base.config.configmap.ConfigMapService;

import org.springframework.stereotype.Component;

import java.util.Arrays;
import java.util.List;
import java.util.Objects;

/**
 * 提供获取产品部署类型的能力
 *
 */
@Slf4j
@Component
public class DeployTypeService {
    /**
     * 备份软件白牌化涉及的产品型号
     */
    public static final ImmutableList<String> WHITE_BOX_DEPLOY_TYPES = ImmutableList.of(DeployTypeEnum.X9000.getValue(),
        DeployTypeEnum.A8000.getValue(), DeployTypeEnum.X8000.getValue(), DeployTypeEnum.X6000.getValue(),
        DeployTypeEnum.X3000.getValue(), DeployTypeEnum.HYPER_DETECT.getValue(),
        DeployTypeEnum.OPEN_SOURCE.getValue());

    /**
     * x系列存储的设备枚举值
     */
    public static final List<DeployTypeEnum> X_SERIES = Arrays.asList(DeployTypeEnum.X3000, DeployTypeEnum.X6000,
            DeployTypeEnum.X8000, DeployTypeEnum.X9000, DeployTypeEnum.OPEN_SOURCE);

    /**
     * E系列存储的设备枚举值
     */
    public static final ImmutableList<DeployTypeEnum> E_SERIES =
        ImmutableList.of(DeployTypeEnum.E1000, DeployTypeEnum.E6000);

    /**
     * 业务认证初始化设备类型
     */
    public static final ImmutableList<DeployTypeEnum> BUSINESS_AUTH_INIT_DEPLOY_TYPES = ImmutableList
            .of(DeployTypeEnum.CLOUD_BACKUP, DeployTypeEnum.CYBER_ENGINE, DeployTypeEnum.HYPER_DETECT);

    /**
     * 不支持RBAC部署形态
     */
    public static final ImmutableList<DeployTypeEnum> NOT_SUPPORT_RBAC_DEPLOY_TYPES = ImmutableList.of(
            DeployTypeEnum.CLOUD_BACKUP_OLD, DeployTypeEnum.CLOUD_BACKUP, DeployTypeEnum.CYBER_ENGINE,
            DeployTypeEnum.HYPER_DETECT);

    /**
     * 支持通过lld初始化的部署形态
     */
    public static final ImmutableList<DeployTypeEnum> SUPPORT_INIT_BY_LLD = ImmutableList.of(
        DeployTypeEnum.X9000, DeployTypeEnum.X8000, DeployTypeEnum.X6000,
        DeployTypeEnum.X3000, DeployTypeEnum.E6000, DeployTypeEnum.E1000,
        DeployTypeEnum.OPEN_SOURCE, DeployTypeEnum.OPEN_SERVER);

    private DeployTypeEnum deployType;

    private final ConfigMapService configMapService;

    public DeployTypeService(ConfigMapService configMapService) {
        this.configMapService = configMapService;
    }

    /**
     * 获取应用部署类型
     *
     * @return deployType-部署类型
     */
    public DeployTypeEnum getDeployType() {
        if (Objects.nonNull(deployType)) {
            return deployType;
        }
        String productModel = System.getenv("DEPLOY_TYPE");
        if (productModel == null) {
            productModel = configMapService.getDeployType();
        }
        deployType = DeployTypeEnum.getByValue(productModel);
        log.info("The system deploy type is :{}", deployType);
        return deployType;
    }

    /**
     * 判断是否为X9000
     *
     * @return true-是，false-不是
     */
    public boolean isX9000() {
        if (Objects.isNull(deployType)) {
            getDeployType();
        }
        return DeployTypeEnum.X9000.equals(deployType);
    }

    /**
     * 判断是否为云备份类型
     *
     * @return true-是，false-不是
     */
    public boolean isCloudBackup() {
        if (Objects.isNull(deployType)) {
            getDeployType();
        }
        return DeployTypeEnum.CLOUD_BACKUP.equals(deployType)
            || DeployTypeEnum.CLOUD_BACKUP_OLD.equals(deployType);
    }

    /**
     * 判断是否为防勒索类型
     *
     * @return true-是，false-不是
     */
    public boolean isHyperDetectDeployType() {
        if (Objects.isNull(deployType)) {
            getDeployType();
        }
        return DeployTypeEnum.HYPER_DETECT.equals(deployType);
    }

    /**
     * 判断是否为安全一体机类型
     *
     * @return true-是，false-不是
     */
    public boolean isCyberEngine() {
        if (Objects.isNull(deployType)) {
            getDeployType();
        }
        return DeployTypeEnum.CYBER_ENGINE.equals(deployType);
    }

    /**
     * 判断是否为E1000类型
     *
     * @return true-是，false-不是
     */
    public boolean isE1000() {
        if (Objects.isNull(deployType)) {
            getDeployType();
        }

        return DeployTypeEnum.E1000.equals(deployType)
            || DeployTypeEnum.OPEN_SERVER.equals(deployType);
    }

    /**
     * 判断是否为openSource类型
     *
     * @return true-是，false-不是
     */
    public boolean isOpenSource() {
        if (Objects.isNull(deployType)) {
            getDeployType();
        }
        return DeployTypeEnum.OPEN_SOURCE.equals(deployType);
    }

    /**
     * 判断是否是不支持多集群部署类型
     *
     * @return true-是，false-不是
     */
    public boolean isNotSupportMemberCluster() {
        if (Objects.isNull(deployType)) {
            getDeployType();
        }
        return DeployTypeEnum.CLOUD_BACKUP.equals(deployType) || DeployTypeEnum.CLOUD_BACKUP_OLD.equals(deployType)
            || DeployTypeEnum.HYPER_DETECT.equals(deployType) || DeployTypeEnum.CYBER_ENGINE.equals(deployType)
            || DeployTypeEnum.E1000.equals(deployType) || DeployTypeEnum.E6000.equals(deployType)
            || DeployTypeEnum.OPEN_SERVER.equals(deployType);
    }

    /**
     * 判断是否是分布式
     *
     * @return true-是，false-不是
     */
    public boolean isPacific() {
        if (Objects.isNull(deployType)) {
            getDeployType();
        }
        return DeployTypeEnum.E6000.equals(deployType);
    }

    /**
     * 判断是否是X系列存储
     *
     * @return true-是，false-不是
     */
    public boolean isXSeries() {
        if (Objects.isNull(deployType)) {
            getDeployType();
        }
        return X_SERIES.contains(deployType);
    }

    /**
     * 判断是否是E系列存储
     *
     * @return true-是，false-不是
     */
    public boolean isESeries() {
        if (Objects.isNull(deployType)) {
            getDeployType();
        }
        return E_SERIES.contains(deployType);
    }

    /**
     * 判断是否是业务认证初始化设备类型
     *
     * @return true-是，false-不是
     */
    public boolean isBusinessAuthInitType() {
        if (Objects.isNull(deployType)) {
            getDeployType();
        }
        return BUSINESS_AUTH_INIT_DEPLOY_TYPES.contains(deployType);
    }

    /**
     * 判断是否不支持RBAC
     *
     * @return true-是(不支持)，false-不是(支持)
     */
    public boolean isNotSupportRBACType() {
        if (Objects.isNull(deployType)) {
            getDeployType();
        }
        return NOT_SUPPORT_RBAC_DEPLOY_TYPES.contains(deployType);
    }

    /**
     * 判断是否支持通过lld初始化
     *
     * @return true-是，false-不是
     */
    public boolean isSupportInitByLLD() {
        if (Objects.isNull(deployType)) {
            getDeployType();
        }
        return SUPPORT_INIT_BY_LLD.contains(deployType);
    }
}
