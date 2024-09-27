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
package openbackup.data.access.framework.copy.mng.constant;

/**
 * 副本信息中资源属性的key定义
 *
 * @author: y00559272
 * @version: [OceanProtect X8000 1.2.1]
 * @since: 2022/8/3
 **/
public abstract class CopyResourcePropertiesConstant {
    /**
     * 扩展参数中的环境类型
     */
    public static final String ENVIRONMENT_TYPE = "environment_type";

    /**
     * 扩展参数中的环境子类型
     */
    public static final String ENVIRONMENT_SUB_TYPE = "environment_sub_type";

    /**
     * 扩展参数中的小文件聚合
     */
    public static final String SMALL_FILE_AGGREGATION = "small_file_aggregation";

    /**
     * 归档自动索引
     */
    public static final String ARCHIVE_RES_AUTO_INDEX = "archive_res_auto_index";

    /**
     * 备份自动索引
     */
    public static final String BACKUP_RES_AUTO_INDEX = "backup_res_auto_index";

    /**
     * 扩展参数中的聚合文件大小
     */
    public static final String AGGREGATION_FILE_SIZE = "aggregation_file_size";

    /**
     * 扩展参数中的待聚合文件大小最大值
     */
    public static final String AGGREGATION_FILE_MAX_SIZE = "aggregation_file_max_size";

    /**
     * 扩展参数中的聚合文件默认大小
     */
    public static final Integer DEFAULT_AGGREGATION_FILE_SIZE = 4096;

    /**
     * 扩展参数中的聚合文件默认大小
     */
    public static final Integer DEFAULT_AGGREGATION_FILE_MAX_SIZE = 1024;

    /**
     * 扩展参数中的环境uuid
     */
    public static final String ENVIRONMENT_UUID = "environment_uuid";

    /**
     * 副本所属保护对象的文件系统ID
     */
    public static final String PROTECTED_RESOURCE_FILESYSTEM_ID = "fileSystemId";

    /**
     * 副本所属保护对象的LunID
     */
    public static final String PROTECTED_RESOURCE_LUN_ID = "lunId";
}
