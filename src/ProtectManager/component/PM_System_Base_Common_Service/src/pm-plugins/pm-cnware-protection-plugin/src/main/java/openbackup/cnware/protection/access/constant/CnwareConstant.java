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
package openbackup.cnware.protection.access.constant;

import java.util.regex.Pattern;

/**
 * CNware插件常量
 *
 */
public class CnwareConstant {
    /**
     * 单机
     */
    public static final String SINGLE = "1";

    /**
     * 端口号最大值
     */
    public static final int MAX_PORT = 65535;

    /**
     * 证书最大1MB
     */
    public static final int CERT_MAX_BYTE_SIZE = 1024 * 1024;

    /**
     * 吊销列表最大5KB
     */
    public static final int CRL_MAX_BYTE_SIZE = 5 * 1024;

    /**
     * 查询资源分页大小
     */
    public static final int PAGE_SIZE = 200;

    /**
     * -1
     */
    public static final int NEGATIVE_ONE = -1;

    /**
     * 1
     */
    public static final int ONE = 1;

    /**
     * 0
     */
    public static final int ZERO = 0;

    /**
     * CNware域名最大长度
     */
    public static final int CNWARE_DOMAINNAME_MAX_LENGTH = 128;

    /**
     * 环境中CNware注册最大数量上限
     */
    public static final int CNWARE_MAX_COUNT = 8;

    /**
     * 资源扫描频率 默认1小时
     */
    public static final int SCAN_INTERVAL_DEFAULT = 3600;

    /**
     * 资源扫描频率 最大值72小时
     */
    public static final int SCAN_INTERVAL_MAX = 72 * 3600;

    /**
     * 资源扫描频率 最小值1小时
     */
    public static final int SCAN_INTERVAL_MIN = 3600;

    /**
     * 是否开启证书校验  “0”或“1”
     */
    public static final String ENABLE_CERT = "enableCert";

    /**
     * 开启证书
     */
    public static final String ENABLE = "1";

    /**
     * 证书文件内容
     */
    public static final String CERTIFICATION = "certification";

    /**
     * 证书文件名称
     */
    public static final String CERT_NAME = "certName";

    /**
     * 证书文件大小
     */
    public static final String CERT_SIZE = "certSize";

    /**
     * 吊销列表内容
     */
    public static final String REVOCATION_LIST = "revocationList";

    /**
     * 吊销列表文件名称
     */
    public static final String CRL_NAME = "crlName";

    /**
     * 吊销列表文件大小
     */
    public static final String CRL_SIZE = "crlSize";

    /**
     * 框架 environment 中 dependencies 的 agent 名称
     */
    public static final String AGENTS = "agents";

    /**
     * CNware ip
     */
    public static final String IP = "ip";

    /**
     * CNware endpoint
     */
    public static final String ENDPOINT = "endpoint";

    /**
     * CNware ip
     */
    public static final String PORT = "port";

    /**
     * CNware Version
     */
    public static final String PRODUCT_VERSION = "productVersion";

    /**
     * Agent 返回的成功错误码
     */
    public static final String SUCCESS = "0";

    /**
     * 是否是树形结构
     */
    public static final String IS_TREE = "isTree";

    /**
     * 扫描时的资源类型key
     */
    public static final String RESOURCE_TYPE_KEY = "resourceType";

    /**
     * 扫描时的marker标记key
     */
    public static final String SCAN_MARKER_KEY = "marker";

    /**
     * 扫描时的资源类型
     */
    public static final String RES_TYPE_CNWARE = "CNware";

    /**
     * 扫描时的资源类型
     */
    public static final String RES_TYPE_CNWARE_HOST_POOL = "CNwareHostPool";

    /**
     * 扫描时的资源类型
     */
    public static final String RES_TYPE_CNWARE_CLUSTER = "CNwareCluster";

    /**
     * 扫描时的资源类型
     */
    public static final String RES_TYPE_CNWARE_HOST = "CNwareHost";

    /**
     * 扫描时的资源类型
     */
    public static final String RES_TYPE_CNWARE_VM = "CNwareVm";

    /**
     * 扫描时的资源类型
     */
    public static final String RES_TYPE_CNWARE_DISK = "CNwareDisk";

    /**
     * 扫描时的资源类型
     */
    public static final String RES_TYPE_TAG = "Tag";

    /**
     * tags
     */
    public static final String TAGS = "tags";

    /**
     * 集群、主机下的虚拟机数量
     */
    public static final String VM_NUMBER = "vmNumber";

    /**
     * 环境资源子类型key
     */
    public static final String SUBTYPE = "subType";

    /**
     * 备份下发的是否全部磁盘参数key
     */
    public static final String ALL_DISK = "all_disk";

    /**
     * 上下电状态 key 0:下电 1:上电
     */
    public static final String POWER_STATE = "powerState";

    /**
     * 上电状态：上电
     */
    public static final String POWER_ON = "1";

    /**
     * 上电状态：下电
     */
    public static final String POWER_OFF = "0";

    /**
     * 恢复的目标位置 key
     */
    public static final String RESTORE_LOCATION = "restoreLocation";

    /**
     * 是否删除原虚拟机
     */
    public static final String IS_DELETE_ORIGINAL_VM = "isDeleteOriginalVM";

    /**
     * 生产环境的domain id key
     */
    public static final String DOMAIN_ID_KEY = "domainId";

    /**
     * 保护对象中包含的磁盘列表key
     */
    public static final String DISK_INFO = "disk_info";

    /**
     * 恢复类型: "0" - 虚拟机恢复  "1" - 磁盘恢复
     */
    public static final String RESTORE_LEVEL = "restoreLevel";

    /**
     * 恢复类型: "0" - 虚拟机恢复
     */
    public static final String RESTORE_LEVEL_ZERO = "0";

    /**
     * 恢复类型: "1" - 磁盘恢复
     */
    public static final String RESTORE_LEVEL_ONE = "1";

    /**
     * 恢复类型: "2" - 即时恢复
     */
    public static final String RESTORE_LEVEL_TWO = "2";

    /**
     * 覆盖原机: "0" - 不覆盖  "1" - 覆盖
     */
    public static final String CLEAN_ORIGIN_VM = "cleanOriginVM";

    /**
     * 覆盖原机: "0" - 不覆盖
     */
    public static final String CLEAN_ORIGIN_VM_ZERO = "0";

    /**
     * 覆盖原机: "1" - 覆盖
     */
    public static final String CLEAN_ORIGIN_VM_ONE = "1";

    /**
     * 部署类型
     */
    public static final String DEPLOY_TYPE = "deployType";

    /**
     * 目标位置
     */
    public static final String TARGET_LOCATION = "targetLocation";

    /**
     * 资源扫描频率
     */
    public static final String RESCAN_INTERVAL_IN_SEC = "rescanIntervalInSec";

    /**
     * 恢复虚拟机加锁
     */
    public static final String RESOURCE_LOCK_ID = "resourceLockId";

    /**
     * 域名正则
     */
    public static final String DOMAIN_REGEX = "^[a-zA-Z0-9][-a-zA-Z0-9]{0,62}(\\.[a-zA-Z0-9][-a-zA-Z0-9]{0,62})+$";

    /**
     * Unicode 转义序列解码正则
     */
    public static final Pattern UNICODE_REGEX = Pattern.compile("\\\\u([0-9a-fA-F]{4})");

    /**
     * 生产存储剩余容量阈值
     */
    public static final String AVAILABLE_CAPACITY_THRESHOLD = "available_capacity_threshold";
}
