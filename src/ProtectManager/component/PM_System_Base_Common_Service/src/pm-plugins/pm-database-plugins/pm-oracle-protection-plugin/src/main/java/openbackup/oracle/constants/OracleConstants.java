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
package openbackup.oracle.constants;

/**
 * oracle常量类
 *
 */
public class OracleConstants {
    /**
     * Oracle环境变量信息
     */
    public static final String ORACLE_HOME = "oracle_home";

    /**
     * Oracle主目录
     */
    public static final String ORACLE_BASE = "oracle_base";

    /**
     * 是否是ASM数据库
     */
    public static final String IS_ASM_INST = "is_asm_inst";

    /**
     * 数据库实例角色
     */
    public static final String DB_ROLE = "db_role";

    /**
     * ASM信息
     */
    public static final String ASM_INFO = "asm_info";

    /**
     * 查询类型
     */
    public static final String QUERY_TYPE = "queryType";

    /**
     * oracle数据库认证状态
     */
    public static final String VERIFY_STATUS = "verify_status";

    /**
     * oracle实例名称
     */
    public static final String INST_NAME = "inst_name";

    /**
     * oracle集群名称
     */
    public static final String CLUSTER_NAME = "cluster_name";

    /**
     * oracle集群ip
     */
    public static final String CLUSTER_IP = "cluster_ip";

    /**
     * oracle ip信息
     */
    public static final String ORACLE_IP_INFOS = "oracle_ip_infos";

    /**
     * 服务运行状态
     */
    public static final String SERVICE_RUNNING = "0";

    /**
     * 运行状态
     */
    public static final String STATUS = "status";

    /**
     * 即时挂载扩展参数 channels
     */
    public static final String CHANNELS = "channels";

    /**
     * 即时挂载扩展参数- 恢复目标位置key
     */
    public static final String RECOVER_TARGET = "recoverTarget";

    /**
     * 即时挂载目标位置 0：恢复到原机原实例，2：恢复到新机
     */
    public static final String OTHER_HOST = "2";

    /**
     * 即时挂载数据仓协议
     */
    public static final int NAS_SHARE_PROTOCOL = 1;

    /**
     * 恢复的目标位置
     */
    public static final String TARGET_LOCATION = "targetLocation";

    /**
     * 集群数据库的实例
     */
    public static final String INSTANCES = "instances";

    /**
     * 即时挂载前置脚本
     */
    public static final String PRE_SCRIPT = "preScript";

    /**
     * 即时挂载后置脚本
     */
    public static final String POST_SCRIPT = "postScript";

    /**
     * 即时失败后置脚本
     */
    public static final String FAIL_POST_SCRIPT = "failPostScript";

    /**
     * 成功-返回码
     */
    public static final String SUCCESS = "0";

    /**
     * Oracle用户组
     */
    public static final String ORACLE_GROUP = "oracle_group";

    /**
     * PM下发给agent的扩展字段，用于判断oracle日志备份是否需要转全量
     */
    public static final String IS_CHECK_BACKUP_JOB_TYPE = "isCheckBackupJobType";

    /**
     * Oracle 版本号 12.1
     */
    public static final String VERSION_12_1 = "12.1";

    /**
     * 四字节对齐标志
     */
    public static final String FILE_HANDLE_BYTE_ALIGNMENT_SWITCH = "fileHandleByteAlignmentSwitch";

    /**
     * 是否需要删除Dtree
     */
    public static final String NEED_DELETE_DTREE = "needDeleteDtree";

    /**
     * 安全模式
     */
    public static final String SECURITY_STYLE = "securityStyle";

    /**
     * 下发存储仓区分Windows操作系统
     */
    public static final String WINDOWS = "WINDOWS";

    /**
     * 集群Oracle存储最大数量
     */
    public static final int ORACLE_CLUSTER_STORAGE_MAN_NUM = 2;

    /**
     * 单机Oracle存储最大数量
     */
    public static final int ORACLE_SINGLE_STORAGE_MAN_NUM = 2;

    /**
     * 存储最大数量
     */
    public static final int MAX_PORT = 65535;

    /**
     * 吊销证书最大5KB
     */
    public static final int CRL_MAX_BYTE_SIZE = 5 * 1024;

    /**
     * 证书最大1MB
     */
    public static final int CERT_MAX_BYTE_SIZE = 1024 * 1024;

    /**
     * 证书开启
     */
    public static final String ENABLE = "1";

    /**
     * 启动存储快照标志位
     */
    public static final String STORAGE_SNAPSHOT_FLAG = "storage_snapshot_flag";

    /**
     * agents分隔符
     */
    public static final String AGENTS_SEPARATOR = ";";

    /**
     * 存储快照agent标志位
     */
    public static final String STORAGE_SNAPSHOT_AGENT_FLAG = "storage_snapshot_agent_flag";

    /**
     * 副本传输模式
     */
    public static final String COPY_TRANSPORT_MODE = "transportMode";

    /**
     * 存储快照模式
     */
    public static final String STORAGE_LAYER = "storageLayer";

    /**
     * 存储快照恢复选择的代理主机
     */
    public static final String PROXY_HOST = "proxyHost";

    /**
     * oracle删除保护组失败告警ID
     */
    public static final String REMOVE_PROTECT_GROUP_FAILED_ID = "0x6403350001";
}
