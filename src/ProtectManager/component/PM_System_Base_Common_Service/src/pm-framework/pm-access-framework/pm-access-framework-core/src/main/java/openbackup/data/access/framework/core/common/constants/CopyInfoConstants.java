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
package openbackup.data.access.framework.core.common.constants;

import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import java.util.Arrays;
import java.util.Collections;
import java.util.List;

/**
 * 副本信息常量定义类
 *
 */
public final class CopyInfoConstants {
    /**
     * default value of task's indexed
     */
    public static final String COPY_INIT_INDEXED = "Unindexed";

    /**
     * default value of backup's copy information's place
     */
    public static final String COPY_INIT_LOCATION = "Local";

    /**
     * default value of backup's status
     */
    public static final String COPY_INIT_STATUS = "Normal";

    /**
     * backup info
     */
    public static final String COPY_GENERATE = "Backup";

    /**
     * 归档、级联复制、反向复制副本不支持索引的应用类型
     */
    public static final List<String> UNSUPPORTED_INDEX_TYPES = Collections.unmodifiableList(
        Arrays.asList(ResourceSubTypeEnum.FUSION_COMPUTE.getType(), ResourceSubTypeEnum.HCS_CLOUD_HOST.getType()));

    /**
     * copy info constants
     */
    private CopyInfoConstants() {}
}
