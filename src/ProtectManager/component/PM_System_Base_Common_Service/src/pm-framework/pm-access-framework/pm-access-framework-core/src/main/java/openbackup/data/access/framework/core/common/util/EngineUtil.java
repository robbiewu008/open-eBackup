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

import openbackup.system.base.sdk.job.model.JobTypeEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import java.util.Arrays;
import java.util.List;

/**
 * 功能描述
 *
 * @author y00280557
 * @since 2020-09-05
 */
public class EngineUtil {
    /**
     * DataMover引擎
     */
    public static final String ENGINE_DATAMOVER = "Engine_DataMover";

    /**
     * DataMover 支持的保护对象类型列表
     */
    private static final List<String> DATAMOVER_SUPPORT_SOURCE_TYPE_LIST =
            Arrays.asList(
                    ResourceSubTypeEnum.VMWARE.getType(),
                    ResourceSubTypeEnum.CLOUD_BACKUP_FILE_SYSTEM.getType());

    private EngineUtil() {}

    /**
     * 根据资源类型获取对应的引擎
     *
     * @param resourceSubType 资源类型
     * @param jobTypeEnum 任务类型
     * @return String 备份引擎任务类型key
     */
    public static String getEngineTaskTypeKey(String resourceSubType, JobTypeEnum jobTypeEnum) {
        if (DATAMOVER_SUPPORT_SOURCE_TYPE_LIST.contains(resourceSubType)) {
            // 老框架的自研应用
            return ENGINE_DATAMOVER + "_" + resourceSubType + "_" + jobTypeEnum.getValue();
        } else {
            // 新框架不区分应用，根据任务类型区分，直接返回任务类型
            return jobTypeEnum.getValue();
        }
    }
}
