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
package openbackup.system.base.config.business.initialize;

import openbackup.system.base.config.business.initialize.beans.StorageVolumeBean;

import org.springframework.util.StringUtils;

import java.util.Arrays;
import java.util.HashMap;
import java.util.LinkedList;
import java.util.List;
import java.util.Locale;
import java.util.Map;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

/**
 * 存储卷配置
 *
 */
public class StorageVolumeConfig {
    /**
     * --------------------------------
     * 自备份卷类型
     */
    public static final int VOLUME_TYPE_SELF_BACKUP = 5;

    /**
     * 自备份卷数量
     */
    public static final int VOLUME_NUMBER_SELF_BACKUP = 1;

    /**
     * --------------------------------
     * 为所有卷使用的文件系统
     */
    public static final int VOLUME_TYPE_FILE_SYSTEM = 0;

    /**
     * 自备份卷名称前缀
     */
    public static final String VOLUME_NAME_PREFIX_SELF_BACKUP = "SELFBACKUP";

    /**
     * 自备份卷大小（每个）=100GB
     */
    public static final long VOLUME_SIZE_SELF_BACKUP = 100L * 1024L * 1024L * 1024L;

    /**
     * --------------------------------
     * 云备份索引卷类型
     */
    public static final int VOLUME_TYPE_COULD_BACKUP = 4;

    /**
     * 云备份索引卷数量
     */
    public static final int VOLUME_NUMBER_COULD_BACKUP = 1;

    /**
     * 云备份索引卷名称前缀
     */
    public static final String VOLUME_NAME_PREFIX_COULD_BACKUP = "CLOUDINDEXBACKUP";

    /**
     * 云备份索引卷大小（每个）=7.5TB
     */
    public static final long VOLUME_SIZE_COULD_BACKUP = 15L * 512L * 1024L * 1024L * 1024L;

    /**
     * --------------------------------
     * 元数据卷类型
     */
    public static final int VOLUME_TYPE_METADATA_BACKUP = 3;

    /**
     * 元数据卷数量
     */
    public static final int VOLUME_NUMBER_METADATA_BACKUP = 1;

    /**
     * 元数据卷名称前缀
     */
    public static final String VOLUME_NAME_PREFIX_METADATA_BACKUP = "METADATABACKUP";

    /**
     * 云备份索引卷大小（每个）=7.5TB
     */
    public static final long VOLUME_SIZE_METADATA_BACKUP = 15L * 512L * 1024L * 1024L * 1024L;

    /**
     * --------------------------------
     * 标准备份卷类型
     */
    public static final int VOLUME_TYPE_STANDARD_BACKUP = 1;

    /**
     * 标准备份卷数量
     */
    public static final int VOLUME_NUMBER_STANDARD_BACKUP = 1;

    /**
     * 标准备份卷名称前缀
     */
    public static final String VOLUME_NAME_PREFIX_STANDARD_BACKUP = "STANDARDBACKUP";

    /**
     * 标准备份卷大小（每个）单位，16B
     */
    public static final long VOLUME_SIZE = 16L;

    /**
     * 标准备份卷大小（每个），16PB
     */
    public static final long VOLUME_SIZE_STANDARD_BACKUP = VOLUME_SIZE * 1024L * 1024L * 1024L * 1024L * 1024L;

    /**
     * --------------------------------
     * 挂载卷的域名列表
     */
    public static final List<String> MOUNT_NAS_DOMAIN_PARAMS =
            Arrays.asList(
                    "nas.storage.protectengine_a.host",
                    "nas.storage.protectengine_a.host.1",
                    "nas.storage.protectengine_a.host.2",
                    "nas.storage.protectengine_a.host.3");

    /**
     * --------------------------------
     * 文件系统路径分隔符号
     */
    public static final String FILE_SYSTEM_PATH_SEPARATOR = "/";

    /**
     * NAS挂载路径的一个通配符
     */
    public static final String DPA_MOUNT_PATH_TAIL_REG = "?";

    /**
     * 路径分隔符正则
     */
    public static final String DPA_MOUNT_PATH_SEPARATOR_REG = FILE_SYSTEM_PATH_SEPARATOR + DPA_MOUNT_PATH_TAIL_REG;

    /**
     * NAS存储的挂载路径
     */
    public static final String DPA_MOUNT_PATH_HEAD = "/dpa";

    /**
     * NAS存储的挂载路径
     */
    public static final String DPA_MOUNT_PATH = DPA_MOUNT_PATH_HEAD + FILE_SYSTEM_PATH_SEPARATOR;

    /**
     * NAS挂载路径正则表达式
     */
    public static final String DPA_MOUNT_PATH_REG = "(/dpa)?/?";

    /**
     * 存储类型
     */
    public static final String DPA_MOUNT_PATH_STORAGE_TYPES_REG = "[0-9]";

    /**
     * 存储分片号
     */
    public static final String DPA_MOUNT_PATH_STORAGE_INDEX_REG = "[0-9]{2}";

    /**
     * 文件系统名称分隔符号
     */
    public static final String FILE_SYSTEM_NAME_SEPARATOR = "_";

    /**
     * 文件系统前缀
     */
    public static final String FILE_SYSTEM_NAME_PREFIX =
            "dpa" + FILE_SYSTEM_NAME_SEPARATOR + "fs" + FILE_SYSTEM_NAME_SEPARATOR;

    /**
     * 文件系统名称随机长度
     */
    public static final int DEFAULT_FILE_SYSTEM_NAME_RANDOM_SIZE = 24;

    /**
     * 随机码正则表达
     */
    public static final String FILE_SYSTEM_CODE_REG =
            // 下划线
            FILE_SYSTEM_NAME_SEPARATOR
                    // 字符集
                    + "[0-9a-zA-Z]{"
                    // 长度
                    + DEFAULT_FILE_SYSTEM_NAME_RANDOM_SIZE
                    // 长度
                    + "}";

    /**
     * 文件系统名称正则
     */
    public static final String FILE_SYSTEM_NAME_REG =
            // 前缀
            FILE_SYSTEM_NAME_PREFIX
                    // 类型编号:存储类型
                    + DPA_MOUNT_PATH_STORAGE_TYPES_REG
                    // 分隔符号
                    + FILE_SYSTEM_NAME_SEPARATOR
                    // 数量编号：01/02/03
                    + DPA_MOUNT_PATH_STORAGE_INDEX_REG
                    // 随机编号:_随机编码（24）
                    + FILE_SYSTEM_CODE_REG;

    /**
     * 共享路径正则
     */
    public static final String FILE_SYSTEM_NAS_PATH_REG =
            // 路径开头
            DPA_MOUNT_PATH_REG
                    // 文件名称表达式
                    + FILE_SYSTEM_NAME_REG
                    // 路径结尾
                    + DPA_MOUNT_PATH_SEPARATOR_REG;

    /**
     * 共享路径正则
     */
    public static final String FILE_SYSTEM_NAS_PATH_STORAGE_1_REG =
            // 路径开头
            DPA_MOUNT_PATH
                    // 文件名称表达式
                    // 前缀
                    + FILE_SYSTEM_NAME_PREFIX
                    // 类型编号:存储类型
                    + "1"
                    // 分隔符号
                    + FILE_SYSTEM_NAME_SEPARATOR
                    // 数量编号：01/02/03
                    + DPA_MOUNT_PATH_STORAGE_INDEX_REG
                    // 随机编号:_随机编码（24）
                    + FILE_SYSTEM_CODE_REG
                    // 路径结尾
                    + DPA_MOUNT_PATH_SEPARATOR_REG;

    /**
     * 共享路径正则
     */
    public static final String FILE_SYSTEM_NAS_PATH_STORAGE_3_REG =
            // 路径开头
            DPA_MOUNT_PATH
                    // 文件名称表达式
                    // 前缀
                    + FILE_SYSTEM_NAME_PREFIX
                    // 类型编号:存储类型
                    + "3"
                    // 分隔符号
                    + FILE_SYSTEM_NAME_SEPARATOR
                    // 数量编号：01/02/03
                    + DPA_MOUNT_PATH_STORAGE_INDEX_REG
                    // 随机编号:_随机编码（24）
                    + FILE_SYSTEM_CODE_REG
                    // 路径结尾
                    + DPA_MOUNT_PATH_SEPARATOR_REG;

    /**
     * 共享路径正则
     */
    public static final String FILE_SYSTEM_NAS_PATH_STORAGE_4_REG =
            // 路径开头
            DPA_MOUNT_PATH
                    // 文件名称表达式
                    // 前缀
                    + FILE_SYSTEM_NAME_PREFIX
                    // 类型编号:存储类型
                    + "4"
                    // 分隔符号
                    + FILE_SYSTEM_NAME_SEPARATOR
                    // 数量编号：01/02/03
                    + DPA_MOUNT_PATH_STORAGE_INDEX_REG
                    // 随机编号:_随机编码（24）
                    + FILE_SYSTEM_CODE_REG
                    // 路径结尾
                    + DPA_MOUNT_PATH_SEPARATOR_REG;

    /**
     * 共享路径正则
     */
    public static final String FILE_SYSTEM_NAS_PATH_STORAGE_5_REG =
            // 路径开头
            DPA_MOUNT_PATH
                    // 文件名称表达式
                    // 前缀
                    + FILE_SYSTEM_NAME_PREFIX
                    // 类型编号:存储类型
                    + "5"
                    // 分隔符号
                    + FILE_SYSTEM_NAME_SEPARATOR
                    // 数量编号：01/02/03
                    + DPA_MOUNT_PATH_STORAGE_INDEX_REG
                    // 随机编号:_随机编码（24）
                    + FILE_SYSTEM_CODE_REG
                    // 路径结尾
                    + DPA_MOUNT_PATH_SEPARATOR_REG;

    /**
     * 存储类型NAS地址路径正则表达式
     */
    public static final Map<Integer, Pattern> FILE_SYSTEM_NAS_PATH_REG_MAP = new HashMap<>();

    /**
     * 共享路径正则匹配
     */
    public static final Pattern FILE_SYSTEM_NAS_PATH_PATTERN = Pattern.compile(FILE_SYSTEM_NAS_PATH_REG);

    /**
     * 文件系统类型控制
     */
    public static final int FILE_SYSTEM_STORAGE_TYPE_INDEX = 2;

    /**
     * --------------------------------
     * 系统默认的租户
     */
    public static final String DEFAULT_SYSTEM_VSTORE_ID = "0";

    /**
     * --------------------------------
     * 参数列表
     */
    public static final Map<Integer, StorageVolumeBean> AUTO_EXPAND_ACTION_PARM_MAP = new HashMap<>();

    /**
     * 存储类型列表
     */
    public static final List<Integer> VOLUME_TYPE_LIST = new LinkedList<>();

    static {
        FILE_SYSTEM_NAS_PATH_REG_MAP.put(
                VOLUME_TYPE_STANDARD_BACKUP, Pattern.compile(FILE_SYSTEM_NAS_PATH_STORAGE_1_REG));

        FILE_SYSTEM_NAS_PATH_REG_MAP.put(
                VOLUME_TYPE_METADATA_BACKUP, Pattern.compile(FILE_SYSTEM_NAS_PATH_STORAGE_3_REG));

        FILE_SYSTEM_NAS_PATH_REG_MAP.put(VOLUME_TYPE_COULD_BACKUP, Pattern.compile(FILE_SYSTEM_NAS_PATH_STORAGE_4_REG));

        FILE_SYSTEM_NAS_PATH_REG_MAP.put(VOLUME_TYPE_SELF_BACKUP, Pattern.compile(FILE_SYSTEM_NAS_PATH_STORAGE_5_REG));
    }

    static {
        // 自备份
        AUTO_EXPAND_ACTION_PARM_MAP.put(
                VOLUME_TYPE_SELF_BACKUP,
                new StorageVolumeBean(
                        VOLUME_TYPE_SELF_BACKUP,
                        VOLUME_NUMBER_SELF_BACKUP,
                        VOLUME_NAME_PREFIX_SELF_BACKUP,
                        VOLUME_SIZE_SELF_BACKUP));

        // 云备份
        AUTO_EXPAND_ACTION_PARM_MAP.put(
                VOLUME_TYPE_COULD_BACKUP,
                new StorageVolumeBean(
                        VOLUME_TYPE_COULD_BACKUP,
                        VOLUME_NUMBER_COULD_BACKUP,
                        VOLUME_NAME_PREFIX_COULD_BACKUP,
                        VOLUME_SIZE_COULD_BACKUP));

        // 元数据
        AUTO_EXPAND_ACTION_PARM_MAP.put(
                VOLUME_TYPE_METADATA_BACKUP,
                new StorageVolumeBean(
                        VOLUME_TYPE_METADATA_BACKUP,
                        VOLUME_NUMBER_METADATA_BACKUP,
                        VOLUME_NAME_PREFIX_METADATA_BACKUP,
                        VOLUME_SIZE_METADATA_BACKUP));

        // 标准备份
        AUTO_EXPAND_ACTION_PARM_MAP.put(
                VOLUME_TYPE_STANDARD_BACKUP,
                new StorageVolumeBean(
                        VOLUME_TYPE_STANDARD_BACKUP,
                        VOLUME_NUMBER_STANDARD_BACKUP,
                        VOLUME_NAME_PREFIX_STANDARD_BACKUP,
                        VOLUME_SIZE_STANDARD_BACKUP));
    }

    static {
        AUTO_EXPAND_ACTION_PARM_MAP.keySet().stream().sorted().forEach(VOLUME_TYPE_LIST::add);
    }

    /**
     * 私有构造函数
     */
    private StorageVolumeConfig() {}

    /**
     * 从文件系统路径中获得存储类型
     *
     * @param fileSystemPath 文件系统路径
     * @return 存储类型
     */
    public static int getStorageTypeFromFileSystemPath(String fileSystemPath) {
        if (!StringUtils.isEmpty(fileSystemPath)) {
            Matcher matcher = FILE_SYSTEM_NAS_PATH_PATTERN.matcher(fileSystemPath);
            if (matcher.find()) {
                return Integer.parseInt(
                        matcher.group().split(FILE_SYSTEM_NAME_SEPARATOR)[FILE_SYSTEM_STORAGE_TYPE_INDEX]);
            }
        }
        return Integer.MIN_VALUE;
    }

    /**
     * 生成卷的名称
     *
     * @param volumePrefix 文件挂载路径
     * @param volumeFlag 文件挂载路径
     * @param index 卷索引编号
     * @return 卷名称
     */
    public static String getVolumeName(String volumePrefix, String volumeFlag, int index) {
        return volumePrefix + volumeFlag + String.format(Locale.ENGLISH, "%02d", index);
    }
}
