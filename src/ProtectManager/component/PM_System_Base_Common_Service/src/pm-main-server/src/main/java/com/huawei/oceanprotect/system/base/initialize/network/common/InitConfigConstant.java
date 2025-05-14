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
package com.huawei.oceanprotect.system.base.initialize.network.common;

import static openbackup.system.base.common.constants.Constants.ERROR_CODE_OK;

import openbackup.system.base.common.enums.ServiceType;

import java.util.HashMap;
import java.util.Map;
import java.util.concurrent.TimeUnit;

/**
 * 初始化配置错误码信息
 *
 * @since 2020-12-08
 */
public final class InitConfigConstant {
    /**
     * 运行中
     */
    public static final int ERROR_CODE_RUNNING = 1;

    /**
     * 修改中
     */
    public static final int MODIFY_NETWORK_RUNNING = 9527;

    /**
     * 修改完成
     */
    public static final int MODIFY_NETWORK_FINISH = 2233;

    /**
     * 修改出错
     */
    public static final int MODIFY_NETWORK_ERROR = 45;

    /**
     * 需要初始化
     */
    public static final int ERROR_CODE_YES = 2;

    /**
     * 不需要初始化
     */
    public static final int ERROR_CODE_NO = 3;

    /**
     * 备份网络配置或修改失败
     */
    public static final int ERROR_CODE_FAILED = 5;

    /**
     * 标准备份服务启动失败状态
     */
    public static final int ERROR_CODE_STANDARD_FAILED = 6;

    /**
     * 标准备份服务启动回滚中状态
     */
    public static final int ERROR_CODE_STANDARD_ROLLBACKING = 7;

    /**
     * 标准备份服务启动回滚失败
     */
    public static final int ERROR_CODE_STANDARD_ROLLBACK_FAILED = 8;

    /**
     * 扩容失败
     */
    public static final int ERROR_EXPANSION_CODE_FAILED = -1;

    /**
     * 初始化不可回退的状态码
     */
    public static final int ERROR_CODE_UNRECOVERABLE_FAILED = 6;

    /**
     * 初始化不可回退的状态码
     */
    public static final long WAIT_DATABASE_UPDATE_TIME = 10000L;

    /**
     * installation language flag
     */
    public static final String INSTALLATION_LANGUAGE_FLAG = "installationLanguage";

    /**
     * backup network flag
     */
    public static final String BACKUP_NETWORK_FLAG = "backupNetwork";

    /**
     * archive network flag
     */
    public static final String ARCHIVE_NETWORK_FLAG = "archiveNetwork";

    /**
     * replication network flag
     */
    public static final String REPLICATION_NETWORK_FLAG = "replicationNetwork";

    /**
     * application flag
     */
    public static final String CONTROLLER_SIZE_NAMES = "controllerNames";

    /**
     * ---------------------------------------------
     * infrastructure
     * 基础设施返回状态
     */
    public static final String SUCCESS = "success";

    /**
     * 安装结果中文
     */
    public static final String ZH_CN = "Chinese";

    /**
     * 安装结果英文
     */
    public static final String EN_US = "English";

    /**
     * VIP_NAMESPACE
     */
    public static final String VIP_NAMESPACE = "dpa";

    /**
     * COMMON_CONF
     */
    public static final String COMMON_CONF = "common-conf";

    /**
     * LANGUAGE
     */
    public static final String LANGUAGE = "language";

    /**
     * FLOAT_IP
     */
    public static final String FLOAT_IP = "vip.address";

    /**
     * ----------------------------------------------------------------
     * 系统初始化状态-关键字
     */

    /**
     * error status flag
     */
    public static final String INIT_PROGRESS_CODE = "initializeProgressCode";

    /**
     * error status flag
     */
    public static final String INIT_PROGRESS_DESC = "initializeProgressDesc";

    /**
     * error status flag
     */
    public static final String INIT_PROGRESS_RATE = "initializeProgressRate";

    /**
     * 挂载参数 字段 列表
     */
    public static final String INIT_ERROR_CODE_PARAM = "errorCodeParams";

    /**
     * 挂载参数 字段 列表
     */
    public static final String INIT_PROGRESS_PARAM = "initializeParam";

    /**
     * ----------------------------------------------------------------
     * 修改备份网络状态-关键字
     */

    /**
     * backup progress error status flag
     */
    public static final String MODIFY_BACK_PROGRESS_STATUS = "modifyBackProgressStatus";

    /**
     * error status flag
     */
    public static final String MODIFY_BACK_PROGRESS_CODE = "modifyBackProgressCode";

    /**
     * error status flag
     */
    public static final String MODIFY_BACK_PROGRESS_DESC = "modifyBackProgressDesc";

    /**
     * error status flag
     */
    public static final String MODIFY_BACK_PROGRESS_RATE = "modifyBackProgressRate";

    /**
     * ----------------------------------------------------------------
     * 修改归档网络状态-关键字
     */

    /**
     * archive progress error status flag
     */
    public static final String MODIFY_ARCH_PROGRESS_STATUS = "modifyArchProgressStatus";

    /**
     * error status flag
     */
    public static final String MODIFY_ARCH_PROGRESS_CODE = "modifyArchProgressCode";

    /**
     * error status flag
     */
    public static final String MODIFY_ARCH_PROGRESS_DESC = "modifyArchProgressDesc";

    /**
     * error status flag
     */
    public static final String MODIFY_ARCH_PROGRESS_RATE = "modifyArchProgressRate";

    /**
     * ----------------------------------------------------------------
     * 标准备份服务拉起网络状态-关键字
     */

    /**
     * sftp progress error status flag
     */
    public static final String INIT_SFTP_PROGRESS_STATUS = "initSftpProgressStatus";

    /**
     * standard progress error status flag
     */
    public static final String INIT_STANDARD_PROGRESS_STATUS = "initStandardProgressStatus";

    /**
     * error status flag
     */
    public static final String INIT_STANDARD_PROGRESS_CODE = "initStandardProgressCode";

    /**
     * sftp service error status flag
     */
    public static final String INIT_SFTP_PROGRESS_CODE = "initSftpProgressCode";

    /**
     * error status flag
     */
    public static final String INIT_STANDARD_PROGRESS_DESC = "initStandardProgressDesc";

    /**
     * sftp service error desc
     */
    public static final String INIT_SFTP_PROGRESS_DESC = "initSftpProgressDesc";

    /**
     * error status flag
     */
    public static final String INIT_STANDARD_PROGRESS_RATE = "initStandardProgressRate";

    /**
     * sftp service error status flag
     */
    public static final String INIT_SFTP_PROGRESS_RATE = "initSftpProgressRate";

    /**
     * float vip flag
     */
    public static final String STANDARD_REPLICATION_SERVICE_FLAG = "standardReplicationServiceIp";

    /**
     * 标准备份服务进度错误码参数列表
     */
    public static final String INIT_STANDARD_ERROR_CODE_PARAM = "initStandardErrorCodeParams";

    /**
     * SFTP服务进度错误码参数列表
     */
    public static final String INIT_SFTP_ERROR_CODE_PARAM = "initSftpErrorCodeParams";

    /**
     * ----------------------------------------------------------------
     * IP类型
     */

    /**
     * ipv6 flag
     */
    public static final String IPV6_TYPE_FLAG = "IPV6";

    /**
     * ipv4 flag
     */
    public static final String IPV4_TYPE_FLAG = "IPV4";

    /**
     * ----------------------------------------------------------------
     * 进度值
     */

    /**
     * 进度0
     */
    public static final int PROGRESS_RATE_00 = 0;

    /**
     * 进度5
     */
    public static final int PROGRESS_RATE_05 = 05;

    /**
     * 进度10
     */
    public static final int PROGRESS_RATE_10 = 10;

    /**
     * 进度15
     */
    public static final int PROGRESS_RATE_15 = 15;

    /**
     * 进度25
     */
    public static final int PROGRESS_RATE_20 = 20;

    /**
     * 进度25
     */
    public static final int PROGRESS_RATE_25 = 25;

    /**
     * 进度30
     */
    public static final int PROGRESS_RATE_30 = 30;

    /**
     * 进度25
     */
    public static final int PROGRESS_RATE_40 = 40;

    /**
     * 进度50
     */
    public static final int PROGRESS_RATE_50 = 50;

    /**
     * 进度60
     */
    public static final int PROGRESS_RATE_60 = 60;

    /**
     * 进度75
     */
    public static final int PROGRESS_RATE_75 = 75;

    /**
     * 进度80
     */
    public static final int PROGRESS_RATE_80 = 80;

    /**
     * 进度90
     */
    public static final int PROGRESS_RATE_90 = 90;

    /**
     * 进度100
     */
    public static final int PROGRESS_RATE_OK = 100;

    /**
     * ----------------------------------------------------------------
     * 初始化的Redis状态信息
     */

    /**
     * 存储在redis的Map，存放redis的状态标识
     */
    public static final String INIT_RUNNING_FLAG = "initialize_running_status";

    /**
     * 存储在Redis Map中的初始化的状态标识
     */
    public static final String INIT_STAUS_FLAG = "initialize_status";

    /**
     * 存储在Redis Map中的修改备份网络的状态标识
     */
    public static final String MODIFY_BACKUP_STAUS_FLAG = "modify_backup_status";

    /**
     * 存储在Redis Map中的修改归档网络的状态标识
     */
    public static final String MODIFY_ARCHIVE_STAUS_FLAG = "modify_archive_status";

    /**
     * 存储在Redis Map中的启动标准备份服务的状态标识
     */
    public static final String INIT_STANDARD_STAUS_FLAG = "init_standard_status";

    /**
     * 存储在Redis Map中的启动SFTP服务的状态标识
     */
    public static final String INIT_SFTP_STAUS_FLAG = "init_sftp_status";

    /**
     * 存储在Redis Map中的状态标识
     */
    public static final String RUNNING_STATUS_VALUE_FLAG = "running";

    /**
     * Redis key的过期时间，1分钟
     */
    public static final int INIT_STATUS_EXPIRE_TIMES = 1;

    /**
     * ----------------------------------------------------------------
     * 单个控制器 备份（BACKUP）和归档(ARCHIVE) 需要的ip个数
     */

    public static final int IP_LOGIC_COUNT = 2;

    /**
     * 单个控制器1个端口所需的VF ip个数
     */
    public static final int IP_VF_COUNT = 2;

    /**
     * 单个控制器1个端口所需的归档 VF ip个数
     */
    public static final int IP_ARCH_VF_COUNT = 1;

    /**
     * ----------------------------------------------------------------
     * 备份 逻辑端口的部分name
     */
    public static final String NFS_CIFS = "_NFS_CIFS_Port";

    /**
     * ----------------------------------------------------------------
     * 备份 逻辑端口的部分name
     */
    public static final String PROTECTENGINE_A = "protectengine-a";

    /**
     * ----------------------------------------------------------------
     * 前端卡信息 A1_P0
     */
    public static final String A1_P0 = "IOM1.P0";

    /**
     * A1_P1
     */
    public static final String A1_P1 = "IOM1.P1";

    /**
     * A1_P1
     */
    public static final String A1_P2 = "IOM1.P2";

    /**
     * A1_P1
     */
    public static final String A1_P3 = "IOM1.P3";


    /**
     * 容器的IOM0卡
     */
    public static final String IOM0 = "IOM0";

    /**
     * 容器的IOM1卡
     */
    public static final String IOM1 = "IOM1";

    /**
     * 容器的IOM3卡
     */
    public static final String IOM3 = "IOM3";

    /**
     * 第一个端口后缀
     */
    public static final String P0 = "P0";

    /**
     * 第二个端口后缀
     */
    public static final String P1 = "P1";

    /**
     * 第三个端口后缀
     */
    public static final String P2 = "P2";

    /**
     * ----------------------------------------------------------------
     * 认证 pod 长度
     */
    public static final int POD_LENGTH = 15;

    /**
     * ----------------------------------------------------------------
     * redis
     */

    /**
     * Redis锁名称
     */
    public static final String INIT_SYSTEM_LOCK_NAME = "SYSTEM_INITIALIZE_LOCK";

    /**
     * Redis锁等待时长
     */
    public static final int INIT_SYSTEM_LOCK_WAIT_TIMES = 0;

    /**
     * Redis锁释放时长，3分钟
     */
    public static final int INIT_SYSTEM_LOCK_RELEASE_TIMES = 3;

    /**
     * 检测初始化是否超时,保存在redis中，过期时间5分钟
     */
    public static final String INIT_TIMEOUT_CHECK = "INIT_TIMEOUT_CHECK";

    /**
     * 检测初始化是否超时key,保存在redis中，过期时间5分钟
     */
    public static final String INIT_TIMEOUT_CHECK_KEY = "INIT_TIMEOUT_CHECK_KEY";

    /**
     * 检测初始化是否超时value,保存在redis中，过期时间5分钟
     */
    public static final String INIT_TIMEOUT_CHECK_VALUE = "INIT_TIMEOUT_CHECK_VALUE";

    /**
     * 过期时间5分钟
     */
    public static final int INIT_TIMEOUT_PERIOD = 5;

    /**
     * Redis锁时间单位（分钟）
     */
    public static final TimeUnit INIT_SYSTEM_LOCK_TIME_UNIT = TimeUnit.MINUTES;

    /**
     * sftp service progress keys
     * 由于：标准备份和sftp不能同时进行开启操作。因此：使用相同的redis Key 作为标志位
     */
    public static final ConfigStatusKey SFTP_CONFIG_STATUS = new ConfigStatusKey(INIT_SFTP_PROGRESS_STATUS,
        INIT_SFTP_PROGRESS_CODE, INIT_SFTP_PROGRESS_DESC, INIT_SFTP_PROGRESS_RATE, INIT_SFTP_ERROR_CODE_PARAM,
        INIT_STANDARD_STAUS_FLAG);

    /**
     * stand backup service progress keys
     */
    public static final ConfigStatusKey STANDARD_CONFIG_STATUS = new ConfigStatusKey(INIT_STANDARD_PROGRESS_STATUS,
        INIT_STANDARD_PROGRESS_CODE, INIT_STANDARD_PROGRESS_DESC, INIT_STANDARD_PROGRESS_RATE,
        INIT_STANDARD_ERROR_CODE_PARAM, INIT_STANDARD_STAUS_FLAG);

    /**
     * 区分sftp或者标准备份服务进度相关key值的集合
     */
    public static final Map<ServiceType, ConfigStatusKey> SERVICE_PROGRESS_KEY = new HashMap() {
        {
            put(ServiceType.SFTP, SFTP_CONFIG_STATUS);
            put(ServiceType.STANDARD, STANDARD_CONFIG_STATUS);
        }
    };

    /**
     * 判断是否继续往后走类型
     */
    public static final Map<Integer, Boolean> RUNNING_STATUS = new HashMap<Integer, Boolean>() {
        {
            put(ERROR_CODE_RUNNING, Boolean.TRUE);
            put(ERROR_CODE_STANDARD_ROLLBACKING, Boolean.TRUE);
            put(ERROR_CODE_OK, Boolean.FALSE);
            put(ERROR_CODE_STANDARD_FAILED, Boolean.FALSE);
            put(ERROR_CODE_STANDARD_ROLLBACK_FAILED, Boolean.FALSE);
        }
    };

    /**
     * backupNetPlane
     */
    public static final String BACKUP_NET_PLANE = "backupNetPlane";

    /**
     * archiveNetPlane
     */
    public static final String ARCHIVE_NET_PLANE = "archiveNetPlane";

    /**
     * 复制平面网络
     */
    public static final String COPY_NET_PLANE = "copyNetPlane";

    /**
     * A1_P1
     */
    public static final String VLAN_NULL = "0";

    /**
     * 获取对应IP段的集合
     */
    public static final Map<String, String> IP_RANGE_NAME = new HashMap() {
        {
            put(ARCHIVE_NET_PLANE, "archive");
            put(BACKUP_NET_PLANE, "backup");
            put(COPY_NET_PLANE, "copy");
        }
    };

    /**
     * --------------------------------------------
     * 开始执行初始化
     */
    public static final String INIT_SUCCESS = "1";

    /**
     * 不能执行初始化
     */
    public static final String INIT_FAILED = "-1";

    /**
     * 已经执行初始化
     */
    public static final String INIT_READY_SUCCESS = "0";

    /**
     * ----------------------------------------
     * 默认管理数据备份策略
     */
    public static final long DEFAULT_SYSTEM_BACKUP_POLICY = 0L;

    /**
     * 容器个数
     */
    public static final String EMPTY_POD = "0";

    /**
     * 使用模式key
     */
    public static final String SERVICE_MODE = "serviceMode";

    /**
     * 租户id
     */
    public static final String VSTORE_ID = "0";

    /**
     * 默认平面网络参数
     */
    public static final String DEFAULT_NETWORK = "0";

    /**
     * 初始化鉴权时保存在数据库的设备id
     */
    public static final String STORAGE_DEVICE_ID = "storageDeviceId";

    /**
     * 私有构造函数
     */
    private InitConfigConstant() {
    }
}