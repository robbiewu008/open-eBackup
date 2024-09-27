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
 * 本信息中扩展属性的key常量定义
 * <p>
 * 常量定义规则如下：以KEY_xxx开头，后面拼接json对象中对应层级的KEY，不同层级之间用_分割
 * 示例：
 * 假如properties中的json结构为:
 * {
 *     "resource": {
 *         "uuid": "1111",
 *         "env": {
 *             "uuid": "2222",
 *             "linkStatus": 1
 *         }
 *     }
 * }
 * <p>
 * 则有如下key与之对应：
 * 1. KEY_RESOURCES
 * 2. KEY_RESOURCES_UUID
 * 3. KEY_RESOURCES_ENV
 * 4. KEY_RESOURCES_ENV_UUID
 * 5. KEY_RESOURCES_ENV_LINK_STATUS
 *
 * @author y00559272
 * @version [OceanProtect A8000 1.1.0]
 * @since 2021-12-03
 **/
public abstract class CopyPropertiesKeyConstant {
    /**
     * 副本中扩扩展参数中存储库列表的KEY
     * 格式：[{"id":"xx", "type": "0", "protocol":""}]
     * type: 0-META_REPOSITORY，1-DATA_REPOSITORY，2-CACHE_REPOSITORY
     * protocol参考： openbackup.data.protection.access.provider.enums.sdk.RepositoryProtocolEnum
     */
    public static final String KEY_REPOSITORIES = "repositories";

    /**
     * 存储副本关联的快照信息
     */
    public static final String SNAPSHOTS = "snapshots";

    /**
     * 存储副本的逻辑大小/缩减前数据量
     */
    public static final String SIZE = "size";

    /**
     * 归档副本id
     */
    public static final String KEY_ARCHIVE_COPY_ID = "archive_id";

    /**
     * 归档存储库id
     */
    public static final String KEY_ARCHIVE_REPOSITORY_ID = "storage_id";

    /**
     * 备份存储库id
     */
    public static final String KEY_BACKUP_REPOSITORY_ID = "storage_id";

    /**
     * sla
     */
    public static final String KEY_BACKUP_SLA = "sla";

    /**
     * policy 备份策略
     */
    public static final String KEY_BACKUP_POLICY = "policy";

    /**
     * chain_id
     */
    public static final String KEY_BACKUP_CHAIN_ID = "chain_id";

    /**
     * 备份副本关联的资源
     */
    public static final String KEY_BACKUP_RESOURCE = "resource";

    /**
     * 备份对象的子资源
     */
    public static final String KEY_PROTECT_SUB_OBJECT = "sub_object";

    /**
     * 目标集群备份存储库id
     */
    public static final String KEY_REPLICATE_EXTERNAL_REPOSITORY_ID = "external_storage_id";

    /**
     * 副本校验文件是否存在
     */
    public static final String KEY_COPY_VERIFY_FILE = "copyVerifyFile";

    /**
     * 副本校验状态
     */
    public static final String KEY_VERIFY_STATUS = "verifyStatus";

    /**
     * 副本最后校验时间
     */
    public static final String KEY_LAST_VERIFY_TIME = "verifyTime";

    /**
     * 副本格式
     */
    public static final String KEY_FORMAT = "format";

    /**
     * 扩展信息
     */
    public static final String KEY_EXTEND_INFO = "extendInfo";

    /**
     * 复制层级
     */
    public static final String KEY_REPLICATE_COUNT = "replicate_count";

    /**
     * Oracle文件系统名前缀
     */
    public static final String ORACLE_FILE_SYSTEM_PREFIX = "D";


    /**
     * 副本id
     */
    public static final String KEY_PROPERTIES_SNAPSHOT_ID = "snapshotId";

    /**
     * 副本名称
     */
    public static final String KEY_PROPERTIES_SNAPSHOT_NAME = "snapshotName";

    /**
     * 文件系统id
     */
    public static final String KEY_PROPERTIES_FILESYSTEM_ID = "filesystemId";

    /**
     * 文件系统名称
     */
    public static final String KEY_PROPERTIES_FILESYSTEM_NAME = "filesystemName";

    /**
     * 租户id
     */
    public static final String KEY_PROPERTIES_TENANT_ID = "tenantId";

    /**
     * 租户id
     */
    public static final String KEY_PROPERTIES_TENANT_NAME = "tenantName";

    /**
     * 存储设备id
     */
    public static final String KEY_RESOURCE_PROPERTIES_ROOT_UUID = "root_uuid";

    /**
     * 存储设备名称
     */
    public static final String KEY_RESOURCE_PROPERTIES_ENVIRONMENT_NAME = "environment_name";

    /**
     * 克隆文件系统共享信息
     */
    public static final String KEY_RESOURCE_PROPERTIES_FILESYSTEM_SHARE_INFO = "fileSystemShareInfo";
}
