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
package openbackup.system.base.sdk.resource.custommodel;

import openbackup.system.base.common.utils.VerifyUtil;

import lombok.extern.slf4j.Slf4j;

import org.apache.logging.log4j.util.Strings;

/**
 * 资源子类型与任务日志的步骤label（log_info）的枚举类
 *
 */
@Slf4j
public enum ResourceSubTypeJobLabelEnum {
    // 文件集
    FILESET_BACKUP_DATA("Fileset", "file_plugin_host_backup_data_completed_label"),

    // NAS共享
    NAS_SHARE_BACKUP_DATA("NasShare", "nas_plugin_hetro_backup_data_completed_label");

    /**
     * 资源子类型
     */
    private final String subType;

    /**
     * 任务日志的步骤label（log_info）
     */
    private final String label;

    ResourceSubTypeJobLabelEnum(String subType, String label) {
        this.subType = subType;
        this.label = label;
    }

    /**
     * 根据资源子类型查询对应类型步骤的任务日志label
     *
     * @param subType 资源子类型
     * @return 任务日志label
     */
    public static String getLabelBySubType(String subType) {
        if (VerifyUtil.isEmpty(subType)) {
            return Strings.EMPTY;
        }

        for (ResourceSubTypeJobLabelEnum labelEnum : ResourceSubTypeJobLabelEnum.values()) {
            if (labelEnum.getSubType().equals(subType)) {
                return labelEnum.getLabel();
            }
        }
        return Strings.EMPTY;
    }

    /**
     * 根据对应类型步骤的任务日志label查询资源子类型
     *
     * @param label 任务日志label
     * @return 资源子类型
     */
    public static String getSubTypeByLabel(String label) {
        if (VerifyUtil.isEmpty(label)) {
            return Strings.EMPTY;
        }

        for (ResourceSubTypeJobLabelEnum labelEnum : ResourceSubTypeJobLabelEnum.values()) {
            if (labelEnum.getLabel().equals(label)) {
                return labelEnum.getSubType();
            }
        }
        return Strings.EMPTY;
    }

    public String getLabel() {
        return label;
    }

    public String getSubType() {
        return subType;
    }

    /**
     * 和字符串对比，是否相同
     *
     * @param otherSubType 其他子类型
     * @return 是否相同
     */
    public boolean equalsSubType(String otherSubType) {
        return subType.equals(otherSubType);
    }
}
