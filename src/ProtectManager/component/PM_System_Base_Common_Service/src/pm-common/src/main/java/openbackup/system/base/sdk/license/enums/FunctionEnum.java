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
package openbackup.system.base.sdk.license.enums;

import lombok.Getter;
import openbackup.system.base.sdk.license.constants.LicenseSbomConstants;

/**
 * function item
 *
 **/
@Getter
public enum FunctionEnum {
    // ---------------------- Basic Function --------------------------
    BACKUP(LicenseSbomConstants.SOFT_CAPACITY_PERMISSION_SBOMCODE), // 备份

    RECOVERY(LicenseSbomConstants.SOFT_CAPACITY_PERMISSION_SBOMCODE), // 恢复

    INSTANT_RECOVERY(LicenseSbomConstants.SOFT_CAPACITY_PERMISSION_SBOMCODE), // 即时恢复

    FINE_GRAINED_RECOVERY(LicenseSbomConstants.SOFT_CAPACITY_PERMISSION_SBOMCODE), // 细粒度恢复

    CROSS_DOMAIN_REPLICATION(LicenseSbomConstants.SOFT_CAPACITY_PERMISSION_SBOMCODE), // 跨域复制

    BAND_CONTROL(LicenseSbomConstants.SOFT_CAPACITY_PERMISSION_SBOMCODE), // 带宽控制

    EXPIRE_COPY_DELETE(LicenseSbomConstants.SOFT_CAPACITY_PERMISSION_SBOMCODE), // 副本过期删除

    LIVE_MOUNT(LicenseSbomConstants.SOFT_CAPACITY_PERMISSION_SBOMCODE), // LiveMount

    ARCHIVE(LicenseSbomConstants.SOFT_CAPACITY_PERMISSION_SBOMCODE), // 归档流程

    REPLICATION(LicenseSbomConstants.SOFT_CAPACITY_PERMISSION_SBOMCODE), // 复制

    COPY(LicenseSbomConstants.SOFT_CAPACITY_PERMISSION_SBOMCODE), // 副本

    // -------------------- Advance Function ---------------------------
    // 云数据保护：云容灾
    CLOUD_DATA_PROTECT_CLOUD_DISASTER_TOLERANCE(LicenseSbomConstants.SOFT_USE_PERMISSION_SBOMCODE),

    // 云数据保护：公有云上数据保护
    CLOUD_DATA_PROTECT_DATA_PROTECTION_ON_PUBLIC_CLOUD(
        LicenseSbomConstants.SOFT_USE_PERMISSION_SBOMCODE),

    // 数据管理：全局检索
    DATA_MANANGE_GLOBALE_SEARCH(LicenseSbomConstants.SOFT_USE_PERMISSION_SBOMCODE),

    // 数据管理：非结构化数据管理
    DATA_MANAGE_UNSTRUCTURED_DATA_MANAGEMENT(LicenseSbomConstants.SOFT_USE_PERMISSION_SBOMCODE),

    // 数据保护：数据脱敏
    DATA_PROTECT_DATA_DESENSITIZATION(LicenseSbomConstants.SOFT_USE_PERMISSION_SBOMCODE),

    // 数据保护：防恶意软件
    DATA_PROTECT_ANTI_MALWARE(LicenseSbomConstants.SOFT_USE_PERMISSION_SBOMCODE),

    // 数据保护：个人数据法规遵从
    DATA_PROTECT_PERSONAL_DATA_COMPLIANCE(LicenseSbomConstants.SOFT_USE_PERMISSION_SBOMCODE),

    // 多集群管理
    MULTI_CLUSTER_MANAGEMENT(LicenseSbomConstants.SOFT_USE_PERMISSION_SBOMCODE),

    // DWS，数据仓库服务
    DATA_WAREHOUSE_SERVICE(LicenseSbomConstants.SOFT_USE_PERMISSION_SBOMCODE);
    private final String sbom;

    FunctionEnum(String sbom) {
        this.sbom = sbom;
    }

    public String getSbom() {
        return sbom;
    }
}
