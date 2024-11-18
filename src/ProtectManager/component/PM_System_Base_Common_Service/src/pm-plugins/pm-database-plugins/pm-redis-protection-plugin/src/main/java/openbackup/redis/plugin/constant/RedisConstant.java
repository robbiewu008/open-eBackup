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
package openbackup.redis.plugin.constant;

import java.util.regex.Pattern;

/**
 * 功能描述: Redis模块的通用常量
 *
 */
public class RedisConstant {
    /**
     * 支持的Redis版本
     */
    public static final int REDIS_SUPPORT_VERSION = 5;

    /**
     * 资源uuid
     */
    public static final String UUID = "uuid";

    /**
     * 支持的Redis版本
     */
    public static final int ABNORMAL_CODE = 8;

    /**
     * 扩展信息 extendInfo 中 version 的 Key 名称
     */
    public static final String EXTEND_INFO_KEY_VERSION = "version";

    /**
     * 扩展信息 extendInfo 中 业务IP 的 Key 名称
     */
    public static final String IP = "ip";

    /**
     * 扩展信息 extendInfo 中 端口 的 Key 名称
     */
    public static final String PORT = "port";

    /**
     * 扩展信息 extendInfo 中 槽 的 Key 名称
     */
    public static final String SLOT = "slot";

    /**
     * 扩展信息 extendInfo 中 部署模式
     */
    public static final String ROLE = "role";

    /**
     * 名字
     */
    public static final String NAME = "name";

    /**
     * 类型
     */
    public static final String TYPE = "resourceType";

    /**
     * 客户端安装路径
     */
    public static final String CLIENT_PATH = "clientPath";

    /**
     * 节点扩展字段里面存放agent id
     */
    public static final String AGENT_ID = "agentId";

    /**
     * rdb存储位置
     */
    public static final String RDB_DIR = "dir";

    /**
     * rdb文件名
     */
    public static final String RDB_DB_FILENAME = "dbfilename";

    /**
     * AOF持久化使能
     */
    public static final String AOF_ENABLED = "aofEnabled";

    /**
     * name最大长度
     */
    public static final int NMAE_MAX_LENGTH = 64;

    /**
     * Linux合法路径的正则表达式
     */
    public static final Pattern LINUX_PATH_PATTERN = Pattern.compile(
        "(/([a-zA-Z0-9][a-zA-Z0-9_\\-]{0,255}/)*([a-zA-Z0-9][a-zA-Z0-9_\\-]{0,255})|/)");

    private RedisConstant() {
    }
}