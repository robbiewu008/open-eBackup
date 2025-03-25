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
package openbackup.system.base.common.constants;

/**
 * 错误码定义类
 *
 */
public final class CommonErrorCode {
    /**
     * ERR_CLUSTER_NUM
     */
    public static final long ERR_CLUSTER_NUM = 1677935115L;

    /**
     * ERR_STORAGE_UNIT_NUM
     */
    public static final long ERR_STORAGE_UNIT_NUM = 1677930072L;

    /**
     * ERR_PARAM
     */
    public static final long ERR_PARAM = 1677929218L;

    /**
     * OBJ_NOT_EXIST
     */
    public static final long OBJ_NOT_EXIST = 1677929217L;

    /**
     * OPERATION_FAILED
     */
    public static final long OPERATION_FAILED = 1677929219L;

    /**
     * CANNOT MODIFY DEFAULT RESOURCE SET
     */
    public static final long NOT_MODIFY_DEFAULT = 1677752066L;

    /**
     * Local storage is not initialized
     */
    public static final long LOCAL_STORAGE_IS_NOT_INITIALIZED = 1677930263L;

    /**
     * STATUS ERROR
     */
    public static final long STATUS_ERROR = 1677929222L;

    /**
     * ILLEGAL_PWD
     */
    public static final long ILLEGAL_PWD = 1677929474L;

    /**
     * 弱口令
     */
    public static final long WEAK_PASSWORD = 1677929506L;

    /**
     * ILLEGAL_TYPE
     */
    public static final long ILLEGAL_TYPE = 1677929475L;

    /**
     * ALREADY_EXIST
     */
    public static final long ALREADY_EXIST = 1677929476L;

    /**
     * 执行取消存储单元授权时，由于用户或存储单元不存在，操作失败。
     */
    public static final long USER_OR_STORAGE_UNIT_NOT_EXIST = 1677930077L;

    /**
     * 执行取消存储单元授权时，由于用户资源绑定的SLA中有策略关联到存储单元，操作失败。
     */
    public static final long USER_RESOURCE_SLA_POLICY_RELATED_TO_STORAGE = 1677930089L;

    /**
     * 执行取消存储单元授权时，由于用户SLA中有策略关联到存储单元，操作失败。
     */
    public static final long USER_SLA_POLICY_RELATED_TO_STORAGE = 1677930090L;

    /**
     * UNKNOWN_USER
     */
    public static final long UNKNOWN_USER = 1677929477L;

    /**
     * WRONG_OLD_PASSWORD
     */
    public static final long WRONG_OLD_PASSWORD = 1677929478L;

    /**
     * WRONG_PASSWORD
     */
    public static final long WRONG_PASSWORD = 1677929507L;

    /**
     * SAME_AS_OLD_PASSWORD
     */
    public static final long SAME_AS_OLD_PASSWORD = 1677929479L;

    /**
     * BEFORE_PASSWORD_SHORTEST_TIME
     */
    public static final long BEFORE_PASSWORD_SHORTEST_TIME = 1677929480L;

    /**
     * CANNOT_USE_RETAIN_PERIOD_HISTORY_PASSWORD
     */
    public static final long CANNOT_USE_RETAIN_PERIOD_HISTORY_PASSWORD = 1677929508L;

    /**
     * CANNOT_USE_IN_MAX_NUMBER_HISTORY_PASSWORD
     */
    public static final long CANNOT_USE_IN_MAX_NUMBER_HISTORY_PASSWORD = 1677929509L;

    /**
     * CANNOT_DO_FOR_DEFAULT
     */
    public static final long CANNOT_DO_FOR_DEFAULT = 1677929482L;

    /**
     * EXCEED_MAX
     */
    public static final long EXCEED_MAX = 1677929483L;

    /**
     * 执行添加设备操作时，由于当前设备数达到系统设定最大值，操作失败。
     */
    public static final long NUMBER_OF_DEVICES_EXCEEDS_THRESHOLD = 1677747205L;

    /**
     * USER_CANNOT_BE_DELETED
     */
    public static final long USER_CANNOT_BE_DELETED = 1677929485L;

    /**
     * COPY_RESTORE_STATUS_ERROR
     */
    public static final long COPY_RESTORE_STATUS_ERROR = 1677933340L;

    /**
     * COPY_LIVE_MOUNT_STATUS_ERROR
     */
    public static final long COPY_LIVE_MOUNT_STATUS_ERROR = 1677932803L;

    /**
     * SNAPSHOT_RESTORE_STATUS_ERROR
     */
    public static final long SNAPSHOT_RESTORE_STATUS_ERROR = 1677932810L;

    /**
     * SESSION_MODE_NUM
     */
    public static final long SESSION_MODE_NUM = 1677929486L;

    /**
     * MODIFY_PWD_LOCKED
     */
    public static final long MODIFY_PWD_LOCKED = 1677929487L;

    /**
     * LOGIN_CRED_WRONG
     */
    public static final long LOGIN_CRED_WRONG = 1677929488L;

    /**
     * LOGIN_USER_WILL_BE_LOCKED
     */
    public static final long LOGIN_USER_WILL_BE_LOCKED = 1677929489L;

    /**
     * LOGIN_USER_LOCKED
     */
    public static final long LOGIN_USER_LOCKED = 1677929490L;

    /**
     * LOGIN_USER_LOCKED_BY_SUPER_USER
     */
    public static final long LOGIN_USER_LOCKED_BY_SUPER_USER = 1677929491L;

    /**
     * LOGIN_USER_EXCEED_MAX_SESSION
     */
    public static final long LOGIN_USER_EXCEED_MAX_SESSION = 1677929492L;

    /**
     * USER_ALREADY_LOCKED
     * 错误场景：执行登录操作时，由于连续多次输入错误密码导致用户被锁定，操作失败。
     * 原因：连续多次输入错误密码导致用户被锁定。
     * 建议：请（{0}）秒后重试。
     */
    public static final long USER_ALREADY_LOCKED = 1677929493L;

    /**
     * LOCK_USER_YOURSELF
     */
    public static final long LOCK_USER_YOURSELF = 1677929494L;

    /**
     * UNLOCK_USER_YOURSELF
     */
    public static final long UNLOCK_USER_YOURSELF = 1677929495L;

    /**
     * DELETE_USER_YOURSELF
     */
    public static final long DELETE_USER_YOURSELF = 1677929496L;

    /**
     * DELETE_USER_WHEN_USER_HAVE_UNFINISHED_JOB
     */
    public static final long DELETE_USER_FAIL_DUR_TO_UNFINISHED_JOB = 1677752071L;

    /**
     * ACCESS_DENIED
     */
    public static final long ACCESS_DENIED = 1677929497L;

    /**
     * HCS用户登录时 因为当前用户在HCS中没有读写权限 登录被拒绝
     */
    public static final long ONLY_HAVE_READ_PERMISSION = 1677752072L;

    /**
     * SLA_ACCESS_DENIED
     */
    public static final long SLA_ACCESS_DENIED = 1677931532L;

    /**
     * RESET_PWD_YOURSELF
     */
    public static final long RESET_PWD_YOURSELF = 1677929498L;

    /**
     * 修改别人的密码
     */
    public static final long MODIFY_OTHERS_PASSWORD = 1677929499L;

    /**
     * REACH_MAX_RESET_LIMIT
     */
    public static final long REACH_MAX_RESET_LIMIT = 1677929501L;

    /**
     * 当前登录密码错误错误码
     */
    public static final long ILLEGAL_CURRENT_PWD = 1677929503L;

    /**
     * 用户第一次登录的时候密码未修改
     */
    public static final long PASSWORD_FIRST_MODIFY_NOTICE = 1677929504L;

    /**
     * 用户的密码处于初始化状态
     */
    public static final long USER_PASSWORD_IS_NOT_MODIFIED = 1677747207L;

    /**
     * 当前设备不具有勒索病毒防护能力
     */
    public static final long NO_RANSOMVIRUS_PROTECTION_CAPABILITY = 1677747206L;

    // 存储库模块相关错误码

    /**
     * 输入的告警容量阈值超过数据桶容量
     */
    public static final long ALARM_THRESHOLD_BEYOND_DATA_BUCKET_TOTAL_SIZE = 1677930261L;

    /**
     * AUTHENTICATION_LOW_LEVEL
     */
    public static final long AUTHENTICATION_LOW_LEVEL = 1677751299L;

    /**
     * 集群节点查询失败
     */
    public static final long CLUSTER_NODES_QUERY_FAILED = 1677929985L;

    /**
     * 目标集群认证失败
     */
    public static final long TARGET_CLUSTER_AUTH_FAILED = 1677929990L;

    /**
     * 目标集群不存在
     */
    public static final long TARGET_CLUSTER_NOT_EXIST = 1677929991L;

    /**
     * 目标集群添加失败
     */
    public static final long TARGET_CLUSTER_ADD_FAILED = 1677929998L;

    /**
     * 集群证书认证失败
     */
    public static final long CERTIFICATE_AUTHENTICATION_ERROR = 1677930006L;

    /**
     * 外部集群校验本地集群失败
     */
    public static final long TARGET_AUTH_LOCAL_CA_ERROR = 1677930070L;

    /**
     * snmp ip超过10条
     */
    public static final long ALARM_TRAP_IP_MAX = 1677929729L;

    /**
     * 没有指定的服务器
     */
    public static final long ALARM_NO_SPECIFIED_SERVER = 1677929731L;

    /**
     * 数量超过限制
     */
    public static final long ALARM_MAX_NUMBER_ERROR = 1677929734L;

    /**
     * 公用的数量超过限制
     */
    public static final long NUMBER_LIMIT = 1677929225L;

    /**
     * 错误场景：执行日志导出操作，由于存在导出中的任务，操作失败。
     */
    public static final long EXIST_EXPORTING_JOB = 1677936641L;

    /**
     * 原因：存在正在运行的告警转储任务，无法修改。
     * 建议：请稍后重试。
     */
    public static final long ALARM_TASK_STATUS_ERROR = 1677929755L;

    /**
     * 原因：告警转储文件不存在。
     * 建议：无。
     */
    public static final long ALARM_FILE_NOT_EXIST = 1677929756L;

    /**
     * 原因：数据保护一体机与SMTP服务器网络连接异常。
     * 建议：请检查数据保护一体机与SMTP服务器网络连接及配置是否正常。
     */
    public static final long ALARM_SMTP_CONNECT_FAILED = 1677929757L;

    /**
     * 原因：收件人地址不存在。
     * 建议：请检查收件人邮箱是否正确。
     */
    public static final long ALARM_INVALID_EMAIL_ADDRESS = 1677929759L;

    /**
     * 原因：数据保护一体机与代理服务器网络连接异常。
     * 建议：请检查数据保护一体机与代理服务器网络连接及配置是否正常。
     */
    public static final long ALARM_CONNECT_PROXY_FAILED = 1677929760L;

    /**
     * 原因：发件人设置中的“发件人邮箱”未配置。
     * 建议：请配置后重试。
     */
    public static final long ALARM_EMPTY_SENDER_ADDRESS = 1677929761L;

    /**
     * 原因：发件人设置中的“SMTP服务器”未配置。
     * 建议：请配置后重试。
     */
    public static final long ALARM_EMPTY_EMAIL_SERVER = 1677929762L;

    /**
     * 原因：发件人设置中的“SMTP服务器认证”未配置或者SMTP服务器认信息配置不正确。
     * 建议：请检查SMTP服务器认证信息是否配置正确。
     */
    public static final long ALARM_FAILED_AUTHENTICATE = 1677929763L;

    /**
     * 原因：名称重复。<br/>
     * 建议：请使用其他名称进行重试。<br/>
     */
    public static final long DUPLICATE_NAME = 1677929223L;

    // 任务错误码

    /**
     * 任务不能被终止
     */
    public static final long TASK_CANNOT_STOP = 1677934341L;

    /**
     * 任务运行状态不支持被终止
     */
    public static final long TASK_STOP_STATUS_ERROR = 1677934339L;

    /**
     * 主机不存在
     */
    public static final long HOST_NOT_EXIST = 1677931265L;

    /**
     * 主机离线
     */
    public static final long HOST_OFFLINE = 1677931266L;

    /**
     * 集群离线
     */
    public static final long CLUSTER_OFFLINE = 1677936899L;

    // live mount 错误码

    /**
     * 更新策略重名
     */
    public static final long LIVE_MOUNT_POLICY_NAME_DUPLICATE = 1677932807L;

    /**
     * 更新策略数量超过128最大值
     */
    public static final long LIVE_MOUNT_POLICY_COUNT_OVER_LIMIT = 1677932808L;

    /**
     * 更新策略关联了Livemount
     */
    public static final long LIVE_MOUNT_POLICY_BOUND_RESOURCE = 1677932809L;

    /**
     * LIVE_MOUNT_COUNT_OVER_LIMIT
     */
    public static final long LIVE_MOUNT_COUNT_OVER_LIMIT = 1677932801L;

    /**
     * LIVE_MOUNT_COUNT_OVER_LIMIT_CYBER_ENGINE
     */
    public static final long LIVE_MOUNT_COUNT_OVER_LIMIT_CYBER_ENGINE = 1677932802L;

    /**
     * SOURCE_TARGET_ENV_INCONSISTENT
     */
    public static final long SOURCE_TARGET_ENV_INCONSISTENT = 1677932805L;

    /**
     * TARGET_ENV_HAS_SAME_NAME_RESOURCE
     */
    public static final long TARGET_ENV_HAS_SAME_NAME_RESOURCE = 1677932806L;

    /**
     * DATASTORE_INSUFFICIENT_STORAGE_SPACE
     */
    public static final long INSUFFICIENT_DATASTORE_STORAGE_SPACE = 1677932811L;

    /**
     * 系统错误
     */
    public static final long SYSTEM_ERROR = 1677929221L;

    /**
     * 用户名或密码错误
     */
    public static final long USER_OR_PASSWORD_IS_INVALID = 1677929488L;

    /**
     * 执行添加或修改复制策略操作时，由于复制集群信息认证失败，操作失败。
     */
    public static final long REPLICATION_CLUSTER_AUTH_FAILED = 1677930080L;

    /**
     * 最大用户数
     */
    public static final long MAX_USER_COUNT = 1677748482L;

    /**
     * 账号被锁定
     */
    public static final long ACCOUNT_LOCKED = 1677930242L;

    /**
     * ip被锁定
     */
    public static final long IP_LOCKED = 1677930244L;

    /**
     * 系统繁忙
     */
    public static final long SYSTEM_BUSY = 1677748481L;

    /**
     * 资源重复添加
     */
    public static final long RESOURCE_REPEAT = 1677930246L;

    /**
     * ip地址或者端口号错误
     */
    public static final long IP_PORT_ERROR = 1677931450L;

    /**
     * 超过64条存储数据
     */
    public static final long STORAGE_EXCEED_MAX_COUNT = 1677930247L;

    /**
     * 非法参数
     */
    public static final long ILLEGAL_PARAM = 1677929220L;

    /**
     * 用户创建修改请求 非法参数
     * 错误场景：执行创建或修改用户操作时，由于参数不满足要求，操作失败。
     * 参数({1})不满足要求。
     * 建议：请修改为满足要求的参数后重试。
     */
    public static final long USER_ILLEGAL_PARAM = 1677752067L;

    /**
     * 角色创建修改请求 非法参数
     * 错误场景：错误场景：执行创建或修改角色操作时，由于参数不满足要求，操作失败。
     * 参数({1})不满足要求。
     * 建议：请修改为满足要求的参数后重试。
     */
    public static final long ROLE_ILLEGAL_PARAM = 1677752068L;

    /**
     * 资源集创建修改请求 非法参数
     * 错误场景：错误场景：执行创建或修改资源集操作时，由于参数不满足要求，操作失败。
     * 参数({1})不满足要求。
     * 建议：请修改为满足要求的参数后重试。
     */
    public static final long RESOURCE_SET_ILLEGAL_PARAM = 1677752069L;

    /**
     * 修改保护操作时，指定存储单元所在设备的其他存储单元中已存在该资源副本
     * 原因：指定存储单元所在设备的其他存储单元中已存在该资源副本
     * 建议：请选择已存在该资源副本的存储单元或不存在该资源副本的其他设备的存储单元（{0}）进行保护操作
     */
    public static final long SAME_DEVICE_HAVING_SAME_RESOURCE_COPY = 1677931547L;

    /**
     * 添加本机IP错误码
     * 原因：执行添加OceanProtect存储设备时，由于选择本机IP地址进行注册，操作失败
     * 建议：重新输入IP地址后重试
     */
    public static final long ADD_DEVICE_REPEAT = 1677931416L;

    /**
     * 同一个应用的不同集群，不能选择同一个代理主机
     */
    public static final long AGENT_EXIST = 1677931380L;

    /**
     * 执行创建归档策略操作时，由于资源已关联WORM策略，操作失败。
     * 原因：资源已关联WORM策略。
     * 建议：请将已关联WORM策略的资源解除关联或选择未关联的资源。
     */
    public static final long CANNOT_PROTECT_ARCHIVE_REPLICATION_SLA = 1677931536L;

    /**
     * 设备接入版本校验失败错误码
     * 原因：设备版本不符合系统要求
     * 建议：请更新接入设备版本
     */
    public static final long VERSION_ERROR = 1677935120L;

    /**
     * 执行添加存储设备操作时，由于接入设备类型与选择设备类型不一致，操作失败。
     * 原因：接入设备类型（{0}）与选择设备类型（{1}）不一致。
     * 建议：请确保设备类型保持一致。
     */
    public static final long TYPE_DIFFERENT = 1677935111L;

    /**
     * 执行添加存储设备操作时，由于接入设备类型与选择设备类型不一致，操作失败。
     * 原因：接入设备类型与选择设备类型不一致。
     * 建议：请确保设备类型保持一致。
     */
    public static final long TYPE_DIFFERENT_WITHOUT_DETAIL = 1677935114L;

    /**
     * 免登陆跳转错误码
     */
    public static final long ACCESS_MAX = 1677935105L;

    /**
     * REPOSITORY_NAME_ALREADY_EXIST
     */
    public static final long REPOSITORY_NAME_ALREADY_EXIST = 1677930255L;

    /**
     * REPOSITORY_AK_SK_INVALID
     */
    public static final long REPOSITORY_AK_SK_INVALID = 1677930254L;

    /**
     * REPOSITORY_PROXY_USER_PWD_INVALID
     */
    public static final long REPOSITORY_PROXY_USER_PWD_INVALID = 1677930253L;

    /**
     * REPOSITORY_NETWORK_CONNECT_FAILED
     */
    public static final long REPOSITORY_NETWORK_CONNECT_FAILED = 1677930252L;

    /**
     * REPOSITORY_ALREADY_BIND_SLA
     */
    public static final long REPOSITORY_SLA_BINDING = 1677930250L;

    /**
     * WRONG_WITH_GET_TARGET_HOST_INFORMATION
     */
    public static final long WRONG_WITH_GET_TARGET_HOST_INFORMATION = 1677931319L;

    /**
     * DATA_BUCKET_INVALID
     */
    public static final long DATA_BUCKET_INVALID = 1677930256L;

    /**
     * INDEX_BUCKET_INVALID
     */
    public static final long INDEX_BUCKET_INVALID = 1677930257L;

    /**
     * 挂载时目标主机上已经挂载同名数据库
     */
    public static final long ERR_MOUNT_SAMENAME_RESOURCE = 1677932812L;

    /**
     * 所选副本只包含部分磁盘数据，不支持挂载。请选择包含所有磁盘的副本执行操作。
     */
    public static final long NOT_ALL_DISK_COPY_CANNOT_CREATE_LIVE_MOUNT = 1677756161L;

    /**
     * 外部存储接入输入的ip或者端口错误
     */
    public static final long PRODUCT_STORAGE_IP_PORT_ERROR = 1677930259L;

    /**
     * 用户名或密码错误
     */
    public static final long USER_NAME_PASSWORD_ERROR = 1677929224L;

    /**
     * 外部存储不支持该账户restful接入
     */
    public static final long ACCOUNT_UNSUPPORT_LOGIN = 1677930260L;

    /**
     * 外部存储不支持修改为其他外部存储信息。
     */
    public static final long STORAGE_UNSUPPORT_UPDATE = 1677930262L;

    /**
     * 该IP地址与端口已存在
     */
    public static final long ALARM_TRAP_HAS_SAME_IP_AND_PORT = 1677929754L;

    /**
     * 获取数据库信息错误
     */
    public static final long WRONG_GET_DATABASE_INFORMATION = 1677932038L;

    /**
     * 选择该主机的副本，执行磁盘恢复操作时，由于指定虚拟机上没有{0}控制器，操作失败。。
     */
    public static final long DISK_RESTORE_CONTROLLER_MISMATCH = 1677933058L;

    /**
     * 执行即时恢复/迁移操作时，由于目标为独立的ESX/ESXi主机，操作失败。
     */
    public static final long TASK_TARGET_CAN_NOT_BE_ESX_OR_ESXI = 1677931290L;

    /**
     * 资源连接状态离线
     */
    public static final long RESOURCE_LINK_STATUS_OFFLINE = 1677931271L;

    /**
     * 原因：资源（{0}）的副本在存储设备（{1}）的不同存储单元上都存在。
     * 建议：资源的副本只能存在于同一设备的同一存储单元上，请删除不需要的副本。
     */
    public static final long RESOURCE_COPY_EXISTED_ON_DIFFERENT_UNIT_OF_SAME_DEVICE = 1677931784L;

    /**
     * 执行原位置恢复/即时恢复/即时挂载操作时，由于副本所属虚拟机所在的计算资源已经不可访问，操作失败。
     */
    public static final long COMPUTE_RESOURCE_INACCESSIBLE = 1677931295L;

    /**
     * vmware 文件恢复到新位置时，目标虚拟机的操作系统和副本的不一致。检查操作系统失败
     */
    public static final long FILE_RESTORE_OS_DIFF = 1677933057L;

    /**
     * 归档网络链路异常
     */
    public static final long ARCHIVE_LINK_ABNORMAL_EXCEPTION = 1677935618L;

    /**
     * 备份网络IP数量不足
     */
    public static final long BACKUP_IP_INSUFFICIENT_EXCEPTION = 1677935619L;

    /**
     * 归档网络IP数量不足
     */
    public static final long ARCHIVE_IP_INSUFFICIENT_EXCEPTION = 1677935620L;

    /**
     * 错误场景：执行系统初始化操作时，由于复制服务网段的IP数量不足，操作失败。
     * 原因：复制服务网段的IP数量不足所有节点需要复制网络的IP数量（{0}）。
     * 建议：请增加归档服务网段的IP数量。
     */
    public static final long COPY_IP_INSUFFICIENT_EXCEPTION = 1677935624L;

    /**
     * 系统正在初始化
     */
    public static final long SYSTEM_INITIALIZING_EXCEPTION = 1677935622L;

    /**
     * 网络配置正在执行中
     */
    public static final long NETYWORK_CONFIGURING_EXCEPTION = 1677935623L;

    /**
     * 原因：服务正在开启中。
     * 建议：请稍后重试。
     */
    public static final long SFTP_INSTALLING_EXCEPTION = 1677935635L;

    /**
     * 原因：控制器的网口（{0}）未连通网络。
     * 建议：请确认网口连线及网络状态正常后重试。
     * 参数列表：{0}以太网端口，例如CTE0.A.IOM1.P3, CTE0.B.IOM1.P3，多个端口以英文逗号加空格分隔
     */
    public static final long ETH_PORT_NOT_RUNNING = 1677935878L;

    /**
     * 控制器的卡槽的端口插线缺失
     */
    public static final long NETWORK_LINE_MISSING_EXCEPTION = 1677935628L;

    /**
     * 控制器的卡槽的端口插线缺失
     */
    public static final long NETWORK_CARD_MISSING_EXCEPTION = 1677935629L;

    /**
     * 原因：控制框{0}的控制器{1}没有需要的以太网卡端口逻辑类型{2}。
     * 建议：请登录设备管理界面添加该控制器的缺少的以太网卡。
     * {0}控制框名称，例如CTE0
     * {1}控制器，例如0A
     * {2}以太网卡类型
     * 0：主机端口/业务端口
     * 1：级联端口
     * 2：管理端口
     * 3：内部端口
     * 4：维护端口
     * 5：管理/业务混合口
     * 6：维护/业务混合口
     * 13：容器前端端口
     * 14：容器后端端口
     */
    public static final long CONTAINER_NO_EXIST_ETH_ERROR = 1677935638L;

    /**
     * 存储容量不足
     */
    public static final long STORAGE_POOL_CAPACITY_INSUFFICIENT_EXCEPTION = 1677935621L;

    /**
     * 系统初始化过程中，由于初始化备份网络、归档网络或存储出现异常，系统初始化失败,需恢复出厂设置
     */
    public static final long INITALIZATION_UNRECOVERABLE_EXCEPTION = 1677935617L;

    /**
     * 数据保护一体机配置的加密方式与SMTP服务器不一致。
     */
    public static final long WORRY_ENCRYPT_PROTOCOL = 1677929764L;

    /**
     * 错误场景：在执行创建报表操作时，由于报表创建超时，操作失败。
     * 原因：报表创建超时。
     * 建议：请稍后重试。
     */
    public static final long REPORT_CREATE_TIMEOUT = 1677936900L;

    /**
     * 该SMTP服务器的SSL证书不合法。
     */
    public static final long SMTP_SERVER_IDENTITY_CHECK_FAIL = 1677929765L;

    /**
     * 未导入CA证书或者导入CA证书校验失败。
     */
    public static final long SMTP_CERTIFICATE_VERIFICATION_FAILED = 1677929766L;

    /**
     * VMware备份时磁盘信息为空。
     */
    public static final long VMWARE_DISK_NOT_EXIST = 1677932035L;

    /**
     * 容器的状态异常
     */
    public static final long INSTALL_CONTAINER_STATUS_ERROR = 1677935625L;

    /**
     * 发现该路由已经存在
     */
    public static final long ROUTE_ALREADY_EXITS_ERROR = 1677935630L;

    /**
     * 归档存储下存在归档副本
     */
    public static final long STORAGE_EXIST_COPY = 1677930265L;

    /**
     * 初始化目的地址的校验错误码
     */
    public static final long NETWORK_ROUTE_TARGET_EXCEPTION = 1073743436L;

    /**
     * 虚拟机所属主机或者集群的信息发生变化
     */
    public static final long HOST_OR_CLUSTER_INFO_HAS_CHANGED = 1677933059L;

    /**
     * 备份网络与归档网路属于同一个网段
     */
    public static final long THE_SAME_SUBNETWORK_ERROR = 1677935631L;

    /**
     * 密码过期
     */
    public static final long SYSTEM_PASSWORD_EXPIRE = 1677935634L;

    /**
     * 用户未进行邮箱认证
     */
    public static final long USER_IS_NOT_AUTHENTICATED = 1677935633L;

    /**
     * 未初始化
     */
    public static final long USER_PASSWORD_IS_NOT_CHANGED = 1677935106L;

    /**
     * 服务IP与备份服务网段IP不在同一网段
     */
    public static final long THE_SAME_SERVICE_NETWORK_SEGMENT = 1677935638L;

    /**
     * 服务IP与备份服务网段内IP相同
     */
    public static final long SERVICE_SAME_NETWORK_SEGMENT = 1677935639L;

    /**
     * 外部存储证书认证失败
     */
    public static final long EXTERNAL_STORAGE_CERT_ERROR = 1677930268L;

    /**
     * 原因：该Kerberos已被资源使用。
     * 建议：请取消关联资源所对应的Kerberos信息后重试。
     */
    public static final long KERBEROS_IS_IN_USE = 1677931397L;

    /**
     * 原因：资源绑定的SLA策略（{0}）已开启自动索引。
     * 建议：请关闭自动索引或者移除保护资源后再重试。
     */
    public static final long RESOURCE_SLA_AUTO_INDEX_ENABLED = 1677933330L;

    /**
     * 资源的端口被修改。
     */
    public static final long RESOURCE_PORT_IS_MODIFIED = 1677931430L;

    /**
     * 在执行注册数据库集群操作时，由于节点已被其他集群使用，操作失败。
     * 原因：节点已被其他集群使用。
     * 建议：请选择未被使用的集群节点。
     */
    public static final long CLUSTER_NODE_IS_REGISTERED = 1677931431L;

    /**
     * 执行修改数据库集群节点操作时，由于当前集群下已存在实例，操作失败。
     * 原因：当前集群下已存在实例
     * 建议：请删除集群下实例后重试。
     */
    public static final long DB_CLUSTER_HAS_INSTANCE = 1677931432L;

    /**
     * 执行注册数据库实例操作时，由于当前实例已注册，操作失败。
     * 原因：当前实例已注册。
     * 建议：请检查主机、业务IP、端口是否已被注册。
     */
    public static final long DB_INSTANCE_HAS_REGISTERED = 1677931433L;

    /**
     * 执行注册数据库实例操作时，由于当前实例已注册，操作失败。
     * 原因：当前实例（{0}）已注册。
     * 建议：请确保实例的主机IP、端口未被注册。
     */
    public static final long INSTANCE_REGISTED_ERROR = 1677873211L;

    /**
     * 执行注册/修改集群实例操作时，由于添加的实例未包含该集群所有实例，操作失败。
     * 原因：添加的实例未包含该集群所有实例。
     * 建议：请确保已添加所有实例。
     */
    public static final long NOT_INCLUDE_ALL_CLUSTER_INSTANCES = 1677931434L;

    /**
     * 执行注册/修改集群实例操作时，由于添加的实例不属于同一集群，操作失败。
     * 原因：添加的实例不属于同一集群。
     * 建议：请确保已添加的所有实例属于同一集群。
     */
    public static final long INSTANCES_NOT_BELONG_SAME_CLUSTER = 1677931435L;

    /**
     * 执行移除数据库操作时，由于数据库存在执行中的识别敏感数据任务或者脱敏任务，操作失败。
     * 原因：数据库存在执行中的识别敏感数据任务或者脱敏任务。
     * 建议：等待数据库的识别敏感数据任务或者脱敏任务执行结束后重试。
     */
    public static final long ANONYMIZATION_JOB_IS_RUNNING = 1677931339L;

    /**
     * 执行存储设备的扫描时，由于存储设备是Dorado6.1.3以下的设备，操作失败
     * 原因：Dorado6.1.3以下的设备，后台不支持文件系统或NasShare的扫描
     * 建议：请手动添加文件系统或Nas共享
     */
    public static final long DORADO_SCAN_ERROR = 1677943312L;

    /**
     * 原因：存在索引中的副本。
     * 建议：请等待索引完成后再重试。
     */
    public static final long RESOURCE_INDEXING = 1677933331L;

    /**
     * 网络连接超时
     */
    public static final long NETWORK_CONNECTION_TIMEOUT = 1677931275L;

    /**
     * 查询内置agent失败
     */
    public static final long INTERNAL_AGENT_NOT_EXIST = 1677935121L;

    /**
     * 资源不存在
     */
    public static final long RESOURCE_IS_NOT_EXIST = 1677935122L;

    /**
     * 证书生成组件的配置文件中不应配置限制IP和域名的subjectAltName
     */
    public static final long SUBJECTALTNAME_CONFIG_ERROR = 1677931034L;

    /**
     * 内部通信组件证书或内部数据库组件证书的subjectAltName不正确
     */
    public static final long INNER_COMM_CERT_SAN_ERROR = 1677931039L;

    /**
     * 自签发证书(服务端证书的CN信息和CA证书的CN信息相同)
     */
    public static final long SELF_SIGNED_CERT_ERROR = 1677931035L;

    /**
     * 文件名长度验证失败
     */
    public static final long FILE_NAME_LENGTH_VALIDATE_FAILED = 1677929234L;

    /**
     * 文件后缀名验证失败
     */
    public static final long FILE_NAME_SUFFIX_VALIDATE_FAILED = 1677929230L;

    /**
     * 文件大小验证失败
     */
    public static final long FILE_SIZE_VALIDATE_FAILED = 1677929231L;

    /**
     * 文件路径验证失败
     */
    public static final long FILE_PATH_VALIDATE_FAILED = 1677929232L;

    /**
     * 文件名不符合安全要求
     */
    public static final long FILE_NAME_SECURITY_VALIDATE_FAILED = 1677929233L;

    /**
     * 原因：没有可用的在线代理主机
     * 建议：1.请单击“保护 > 客户端”，查看原代理主机状态是否为离线。
     * 2.请重新选择可用的在线代理主机。
     */
    public static final long AGENT_NOT_EXIST = 1677931409L;

    /**
     * 安全一体机
     * 原因：代理主机不存在。
     * 建议：请稍后重试或联系技术支持工程师协助解决。
     */
    public static final long AGENT_NOT_EXIST_CYBER = 1677929235L;

    /**
     * 原因：网络异常或数据保护代理异常。
     * 建议：请在《事件参考》中搜索“数据保护代理异常”，根据告警处理建议进行处理。
     */
    public static final long AGENT_NETWORK_ERROR = 1677931404L;

    /**
     * 原因：备份存储集群已加入分布式NAS存储库。
     * 建议：在存储库中取消该备份集群后重试。
     */
    public static final long BACKUP_STORAGE_CLUSTER_EXISTS = 1677930018L;

    /**
     * 原因：执行添加备份存储单元时，由于该存储池在DeviceManager上不存在，操作失败。
     * 建议：请重新选择存储池。
     */
    public static final long STORAGE_POOL_IS_NOT_EXISTS_IN_DEVICE = 1677930076L;

    /**
     * 原因：该本地盘在服务器上不存在。
     * 建议：请重新选择本地盘。
     */
    public static final long BASIC_DISK_IS_NOT_EXISTS_IN_SERVER = 1677930084L;

    /**
     * 原因：该受保护环境已经注册。
     * 建议：请选择未被注册的环境。
     */
    public static final long PROTECTED_ENV_REPEATED = 1677747204L;

    /**
     * 原因：集群节点已被注册。
     * 建议：请选择未被注册的集群节点进行操作。
     */
    public static final long NODE_REPEAT = 1677931429L;

    /**
     * 原因：用户添加的集群节点与实际的集群节点不一致。
     * 建议：请确保集群节点信息一致。
     */
    public static final long CLUSTER_NODES_INCONSISTENT = 1677931427L;

    /**
     * 原因：集群节点信息错误或者网络异常。
     * 建议：1、请检查集群节点信息填写正确。
     * 2、请确保数据保护代理主机和Redis网络连接正常。
     */
    public static final long REDIS_CONNECT_FAILED = 1577209955L;

    /**
     * 原因：Redis集群节点未全部关闭。
     * 建议：请关闭Redis集群所有节点后重试。
     */
    public static final long REDIS_NODES_NOT_CLOSE = 1677933066L;

    /**
     * 原因：目标集群节点与原集群节点的槽位号不一致。
     * 建议：请确保目标集群节点与原集群节点的槽位号一致。
     */
    public static final long REDIS_SRC_DST_SLOT_DIFF = 1677933065L;

    /**
     * 原因：Redis集群节点开启了AOF持久化模式。
     * 建议：请关闭Redis集群所有节点的AOF持久化模式后重启。
     */
    public static final long REDIS_NODE_AOF_ENABLE = 1677933067L;

    /**
     * 原因：所选资源的授权信息不一致。
     * 建议：请选择相同授权信息的资源或者选择都未授权的资源。
     */
    public static final long RESOURCE_AUTHORIZE_INCONSISTENT = 1677931444L;

    /**
     * 原因：所选主机（名称:{0}，IP地址:{1}）为非受信主机。
     * 建议：请选择受信主机进行操作。
     */
    public static final long RESOURCE_NOT_TRUST = 1677931445L;

    /**
     * 原因：资源总数已达上限（{0}）。
     * 建议：请删除部分资源后重试。
     */
    public static final long RESOURCE_NUM_EXCEED_LIMIT = 1677931446L;

    /**
     * 原因：所选资源被其他资源引用。
     * 建议：请先解除资源的引用关系后重试。
     */
    public static final long RESOURCE_BE_DEPENDED_BY_OTHERS = 1677931443L;

    /**
     * 已经存在相同类型的任务处于运行中。
     */
    public static final long EXIST_SAME_TYPE_JOB_IN_RUNNING = 1677934343L;

    /**
     * 错误场景：执行注册主机操作时，由于待注册的主机不支持指定安全加密算法套件，操作失败。
     * 原因：待注册主机不支持指定安全加密算法套件。
     * 建议：请确保待注册主机支持《ProtectAgent安装指南》中的安全加密算法套件。
     */
    public static final long NOT_SUPPORT_ENCRYPTION_ALGORITHM_SUITE = 1677873152L;

    /**
     * 错误场景：执行资源恢复操作时，由于集群处于非正常状态，操作失败。
     * 原因：集群处于非正常状态。
     * 建议：请修复集群后重试。
     */
    public static final long CLUSTER_LINK_STATUS_ERROR = 1677935361L;

    /**
     * 错误场景：执行备份代理日志导出操作时，由于正在导出代理的日志，操作失败。
     * 原因：正在导出代理的日志。
     * 建议：请等待导出完成后再重试。
     */
    public static final long ERROR_EXIST_EXPORTING_LOG = 1677936645L;

    /**
     * 原因：由于当前DevcieManager已存在需要创建的用户（dataprotect_admin），操作失败。
     * 建议：请在DevicceManager界面，“设置”->“用户与安全”->“用户与角色”->删除该用户之后重试。
     */
    public static final long USER_ALREADY_EXIST = 1677935641L;

    /**
     * 原因：DeviceManager中超级管理员用户已达系统上限。
     * 建议：请在DeviceManager界面，单击“设置 > 用户与安全 > 用户与角色”，确认系统中是否已经存在两个超级管理员用户。
     */
    public static final long SUPER_USER_UPPER_LIMIT = 1677935640L;

    /**
     * 原因：当前DevicceManager用户达到上限，操作失败。
     * 建议：请在DevicceManager界面，“设置”->“用户与安全”->“用户与角色”->删除其中一个用户之后重试。
     */
    public static final long USER_UPPER_LIMIT = 1677935632L;

    /**
     * 原因：使用的服务网段（起始IP：{0}，结束IP：{1}）不在同一网段范围内或存在无效的IP地址({3})。
     * 建议：请修改服务网段和子网掩码后重试。
     */
    public static final long RETURN_INVALID_IP = 1677935627L;

    /**
     * 原因：由于服务网段的IP（{0}）已被使用。
     * 建议：请修改为其他服务网段的IP。
     */
    public static final long NETWORK_IP_ALREADY_EXIST_ERROR = 1677935636L;

    /**
     * 原因：认证用户不是超级管理员。
     * 建议：请修改为超级管理员后重试。
     */
    public static final long IS_NOT_SUPER_ADMIN_ERROR = 1677935639L;

    /**
     * 错误场景：执行代理主机导出日志操作时，由于导出代理主机的日志大小超过上限，操作失败。
     * 原因：导出代理主机（{0}）的日志大小超过上限（{1}）。
     * 建议：请在代理主机路径（{2}）下获取日志文件。
     */
    public static final long ERROR_LOG_FILE_EXCEED_MAX_SIZE = 1677936647L;

    /**
     * 原因：资源在目标位置已存在挂载的副本。
     * 建议：请选择其他资源的副本。
     */
    public static final long DUPLICATE_MOUNT_ERROR = 1677931322L;

    /**
     * 错误场景：执行配置主机LAN-FREE操作时，由于选择的客户端WWPN个数不在1-2范围内，操作失败。
     * 原因： 选择的客户端WWPN个数不在1-2范围内。
     * 建议：请重新选择客户端WWPN。
     */
    public static final long CHOOSE_WWPN_NUM_ERROR = 1677931776L;

    /**
     * 错误场景：执行配置主机LAN-FREE操作时，由于选择的客户端WWPN个数不在范围内，操作失败。
     * 原因： 选择的客户端WWPN个数不在（{0}）-（{1}）范围内。
     * 建议：请重新选择客户端WWPN。
     */
    public static final long CHOOSE_SANCLIENT_WWPN_NUM_ERROR = 1677931346L;

    /**
     * 错误场景：执行为AIX主机配置LAN-FREE操作时，由于选择的SanClient主机个数不在范围内，操作失败。
     * 原因： 选择的SanClient主机个数不在（{0}）-（{1}）范围内。
     * 建议：请重新选择SanClient主机。
     */
    public static final long CHOOSE_SANCLIENT_NUM_ERROR = 1677931347L;

    /**
     * 错误场景：执行配置主机LAN-FREE操作时，由于选择的FC端口个数不在1-4范围内，操作失败。
     * 原因：选择的FC端口个数不在1-4范围内。
     * 建议：请重新选择FC端口。
     */
    public static final long CHOOSE_FC_PORT_NUM_ERROR = 1677931778L;

    /**
     * 错误场景：执行配置主机LAN-FREE时，由于主机不支持源端重删，操作失败。
     * 原因：主机不支持源端重删。
     * 建议：请确认主机信息。
     */
    public static final long LAN_FREE_HOST_NOT_DEDUPTION = 1677931307L;

    /**
     * 错误场景：执行配置主机LAN-FREE时，由于客户端wwpn存在重复，操作失败。
     * 原因：客户端wwpn存在重复。
     * 建议：请重新选择客户端WWPN。
     */
    public static final long LAN_FREE_WWPN_REPEAT = 1677931314L;

    /**
     * 错误场景：执行配置主机LAN-FREE时，由于客户端wwpn格式错误，操作失败。
     * 原因：WWPN是一个长度为16个字符，由字母A～F或者数字0～9或者a～f组成，非全0、非全F和非全f的16进制值数。
     * 建议：请确认WWPN格式正确。
     */
    public static final long LAN_FREE_WWPN_FORMAT_ERROR = 1677931316L;

    /**
     * 错误场景：执行配置主机LAN-FREE时，由于主机离线，操作失败。
     * 原因：主机离线。
     * 建议：请确认主机状态。
     */
    public static final long LAN_FREE_HOST_OFF_LINE = 1677931315L;

    /**
     * 错误场景：执行即时挂载时，由于挂载路径为非空目录，操作失败。
     * 原因：挂载路径为非空目录。
     * 建议：请重新选择挂载路径。
     */
    public static final long FC_MOUNT_NOT_EMPTY_DIR_ERROR = 1677931317L;

    /**
     * 原因：所选的代理主机不属于同一集群。
     * 建议：请确保所选的代理主机属于同一集群。
     */
    public static final long AGENT_NOT_BELONG_TO_SAME_CLUSTER = 1677931325L;

    /**
     * 原因：当前集群下的实例（{0}）正在执行（{1}）任务。
     * 建议：请等待实例执行完任务后重试。
     */
    public static final long CLUSTER_HAS_INSTANCE_IS_RUNNING = 1677931324L;

    /**
     * 原因：选择的代理主机（{0}）与数据节点（{1}）不匹配。
     * 建议：请选择与数据节点匹配的代理主机。
     */
    public static final long AGENT_MISMATCH_NODE = 1677931323L;

    /**
     * 原因：资源（{0}）已关联Air Gap策略。
     * 建议：请解除关联后重试。
     */
    public static final long RESOURCE_ALREADY_BIND_AIR_GAP_POLICIES = 1677931326L;

    /**
     * 原因：选择的SanClient主机（{0}）已被AIX主机关联。
     * 建议：请解除关联后重试。
     */
    public static final long SANCLIENT_RESOURCE_ALREADY_BIND_AIX = 1677931349L;

    /**
     * 原因：当前存储设备存在“排队中”或“运行中”的智能侦测任务。
     * 建议：请等待智能侦测任务执行完成，或者手动停止智能侦测任务后重试。
     */
    public static final long INTELLIGENT_DETECTION_TASK_IS_WORKING = 1677931338L;

    /**
     * 原因：上传的代理软件包版本与当前系统版本（{0}）不一致。
     * 建议：请确保上传的代理软件包版本与当前系统版本一致。
     */
    public static final long AGENT_VERSION_NOT_MATCH_PM = 1677873156L;

    /**
     * 不支持当前功能。
     */
    public static final long NOT_SUPPORT_FUNCTION = 1677929987L;

    /**
     * 原因：该类型的资源存在正在运行中的注册或修改操作。
     * 建议：请稍后重试。
     */
    public static final long SAME_RESOURCE_OPERATION_IS_RUNNING = 1677931337L;

    /**
     * 错误场景：执行恢复操作时，由于源实例和目标实例的版本不匹配，操作失败。
     * 原因：源实例和目标实例的版本不匹配。
     * 建议：请确保源实例和目标实例的版本匹配。
     */
    public static final long VERSION_NOT_MATCH_BEFORE_RESTORE = 1577210056L;

    /**
     * 错误场景：资源已经注册，不能重复注册。
     * 原因：资源已经注册，不能重复注册。
     * 建议：无。
     */
    public static final long RESOURCE_IS_REGISTERED = 1677931274L;

    /**
     * 错误场景：执行修改SLA策略操作时，由于该SLA绑定的资源类型不支持对应的备份类型，操作失败。
     * 原因：该SLA绑定的（{0}）资源类型不支持（{1}）备份类型。
     * 建议：请修改备份类型后重试。
     */
    public static final long SLA_NOT_SUPPORT_BACKUP_POLICY = 1677931542L;

    /**
     * 错误场景：执行修改SLA策略操作时，由于该SLA绑定的资源类型不支持对应的归档/复制策略，操作失败。
     * 原因：该SLA绑定的（{0}）资源类型不支持（{1}）策略。
     * 建议：请移除（{1}）策略后重试。
     */
    public static final long SLA_NOT_SUPPORT_ARCHIVING_REPLICATION_POLICY = 1677931557L;

    /**
     * 错误场景：执行注册Kubernetes集群操作时，由于配置文件有误或者集群状态异常，操作失败。
     * 原因：配置文件有误或者集群状态异常。
     * 建议：请修改配置文件或检查Kubernetes集群状态后重试。
     */
    public static final long KUBE_CONFIG_ERROR = 1577209961L;

    /**
     * 错误场景：执行注册受保护环境操作时，由于已注册的同类型受保护环境数量已达到最大规格，操作失败。
     * 原因：已注册的同类型受保护环境数量已达到最大规格（{0}）。
     * 建议：请删除不需要受保护的同类型环境后重试。
     */
    public static final long ENV_COUNT_OVER_LIMIT = 1677747202L;

    /**
     * 错误场景：执行断开复制链路操作时，由于存储设备未关联Air Gap策略，操作失败。
     * 原因：存储设备未关联Air Gap策略。
     * 建议：请将存储设备关联Air Gap策略后重试。
     */
    public static final long NO_VALID_AIR_GAP_POLICY = 1677936414L;

    /**
     * 错误场景：断开存储设备（ESN：{0}）复制链路失败。
     * 原因：存储设备连接失败。
     * 建议：请恢复存储设备连接后重试。
     */
    public static final long AIR_GAP_DEACTIVE_FAIL = 1677936413L;

    /**
     * 错误场景：执行断开复制链路操作时，安全一体机存储设备未接入，操作失败。
     * 原因：执行断开复制链路操作时，安全一体机存储设备未接入，操作失败。
     * 建议：请将存储设备接入到安全一体机中。
     */
    public static final long DEACTIVE_DEVICE_NOT_IN_CE = 1677936412L;

    /**
     * 错误场景：执行打开复制链路操作时，由于存储设备未关联Air Gap策略，操作失败。
     * 原因：存储设备未关联Air Gap策略。
     * 建议：请将存储设备关联Air Gap策略后重试。
     */
    public static final long ACTIVE_NO_VALID_AIR_GAP_POLICY = 1677936415L;

    /**
     * 错误场景：打开存储设备（ESN：{0}）复制链路失败。
     * 原因：存储设备连接失败。
     * 建议：请恢复存储设备连接后重试。
     */
    public static final long AIR_GAP_ACTIVE_FAIL = 1677936390L;

    /**
     * 错误场景：执行打开复制链路操作时，安全一体机存储设备未接入，操作失败。
     * 原因：执行打开复制链路操作时，安全一体机存储设备未接入，操作失败。
     * 建议：请将存储设备接入到安全一体机中。
     */
    public static final long ACTIVE_DEVICE_NOT_IN_CE = 1677936411L;

    /**
     * 错误场景：执行备份/恢复/副本删除/副本检验操作时，由于配置LAN-Free时AIX主机未关联sanClient主机，操作失败。
     * 原因：配置LAN-Free时AIX主机（{0}）未关联sanClient主机，操作失败。
     * 建议：请确保所有AIX主机都关联了sanClient主机后重试。
     */
    public static final long AIX_PARTIAL_ASSOCIATED_SANCLIENT_ERROR = 1677932049L;

    /**
     * 错误场景：执行备份操作时，由于配置LAN-Free时AIX主机选择的数据协议不为FC，操作失败。
     * 原因：配置LAN-Free时AIX主机（{0}）选择的数据协议不为FC。
     * 建议：请确保所有AIX主机配置LAN-Free时选择的数据协议为FC后重试。
     */
    public static final long SANCLIENT_CONFIG_FC_ERROR = 1677931353L;

    /**
     * 错误场景：执行备份/恢复/副本删除/副本检验操作时，由于配置LAN-Free时AIX主机绑定的SanClient主机全部离线，操作失败。
     * 原因：AIX主机配置LAN-Free时绑定的SanClient主机全部离线。
     * 建议：请确保所有AIX主机配置LAN-Free时绑定的SanClient主机至少有一个在线后重试。
     */
    public static final long ALL_SANCLIENT_IS_OFFLINE = 1677931354L;

    /**
     * 原因：主节点与成员节点设备型号不一致
     * 建议：请确保主节点与成员节点设备型号一致
     */
    public static final long CLUSTERS_DEPLOY_TYPE_INCONSISTENT = 1677930034L;

    /**
     * 原因：当前登录节点非主节点
     * 建议：请登录主节点执行该操作
     */
    public static final long CURRENT_CLUSTER_IS_NOT_PRIMARY = 1677930030L;

    /**
     * 原因：成员节点（{0}）不处于在线状态。
     * 建议：请确保成员节点处于在线状态。
     */
    public static final long MEMBER_CLUSTER_NOT_ONLINE = 1677931041L;

    /**
     * 原因：备节点（{0}）不处于在线状态。
     * 建议：请确保备节点处于在线状态。
     */
    public static final long STANDBY_CLUSTER_NOT_ONLINE = 1677930064L;

    /**
     * 原因：备节点处于离线状态
     * 建议：请确保备节点处于在线状态
     */
    public static final long ADD_HA_STANDBY_CLUSTER_OFFLINE = 1677930029L;

    /**
     * 原因：成员节点不存在
     * 建议：请确保已存在成员节点
     */
    public static final long MEMBER_CLUSTER_NOT_EXIST = 1677930031L;

    /**
     * 原因：成员节点处于离线状态
     * 建议：请确保成员节点处于在线状态
     */
    public static final long MEMBER_CLUSTER_OFFLINE = 1677930032L;

    /**
     * 原因：备节点已存在
     * 建议：请先移除备节点后重试
     */
    public static final long STANDBY_CLUSTER_IS_EXIST = 1677930033L;

    /**
     * 原因：备节点处于离线状态
     * 建议：请确保备节点处于在线状态或执行强制删除操作
     */
    public static final long STANDBY_CLUSTER_OFFLINE = 1677930049L;

    /**
     * 原因：HA未添加
     * 建议：请先添加HA
     */
    public static final long HA_DOES_NOT_EXIST = 1677930054L;

    /**
     * 原因：用户为人机帐号
     * 建议：无
     */
    public static final long UPDATE_LOCAL_STORAGE_PASSWORD_FAIL = 1677930294L;

    /**
     * 原因：备份成员节点存在关联的Air Gap策略
     * 建议：请移除关联的Air Gap策略后重试
     */
    public static final long CLUSTER_HAS_AIRGAP = 1677930057L;

    /**
     * 原因：备份成员节点存在正在运行的任务
     * 建议：请等待任务完成后重试
     */
    public static final long CLUSTER_HAS_RUNNING_TASK = 1677930058L;

    /**
     * 原因：备份成员节点存在正在运行的任务
     * 建议：请先删除主机LAN-Free配置后重试
     */
    public static final long CLUSTER_HAS_LAN_FREE = 1677930061L;

    /**
     * 原因：未设置邮件告警级别
     * 建议：请先设置邮件告警级别后重试
     */
    public static final long NO_EMAIL_ALARM_SETTINGS = 1677929236L;

    /**
     * 原因：DeviceManager未开启NFSv4.1服务。
     * 建议：请登录Devicemanager，选择“设置 > 文件服务 > NFS服务”，启用NFSv4.1服务。
     */
    public static final long NFS_V41_SERVICE_NOT_OPEN = 1677931379L;

    /**
     * 原因：DeviceManager未开启NFSv4.1服务。
     * 建议：请登录Devicemanager，选择“设置 > 共享设置 > NFS服务”，启用NFSv4.1服务。
     */
    public static final long PACIFIC_NFS_V41_SERVICE_NOT_OPEN = 1677931482L;

    /**
     * 原因：开启并行数据存储且在线备份存储单元数量低于系统下限（{0}）。
     * 建议：请确保备份存储单元均为在线状态。
     */
    public static final long BACKUP_CLUSTER_NOT_ALL_ONLINE = 1677945344L;

    /**
     * 原因：外部集群用户原密码未修改。
     * 建议：请在外部集群修改原密码。
     */
    public static final long EXTERNAL_CLUSTER_PASSWORD_FIRST_MODIFY_NOTICE = 1677930013L;

    /**
     * 原因：复制集群用户密码过期。
     * 建议：请登录复制目标集群，单击“系统 > 安全 > 用户和角色”，修改远端设备管理员密码后，
     * 在本系统单击“系统 > 基础设施 > 集群管理 > 复制集群”修改对应的复制目标集群的密码后重试。
     */
    public static final long REPLICATION_CLUSTER_PASSWORD_EXPIRE = 1677749506L;

    /**
     * 原因：生成副本时主机的LAN-Free配置与当前配置不一致。
     * 建议：请检查该副本对应主机的LAN-Free配置。
     */
    public static final long INCONSISTENT_COPY_TYPE = 1677931453L;

    /**
     * 原因：SAN Client主机({0})的SAN Client IQN未配置。
     * 建议：请配置SAN Client主机的SAN Client IQN。
     */
    public static final long SAN_CLIENT_NOT_IQN = 1677873244L;

    /**
     * 原因：资源扫描任务未完成。
     * 建议：请等待资源扫描任务完成后重试
     */
    public static final long SCAN_JOB_NOT_COMPLETE = 1677931417L;

    /**
     * 原因：存储资源（{0}）无法连通。
     * 建议：1.请检查用户名、密码或管理IP地址是否正确。
     * 2.请检查导入的证书是否有效。
     * 3.请检查代理主机与存储资源网络的连通性。
     */
    public static final long STORAGE_CONNECTION_CHECK_FAILED = 1677931439L;

    /**
     * 原因：添加的存储资源数量已达到系统上限（{0}）。
     * 建议：请减少存储资源数量后重试。
     */
    public static final long STORAGE_OVER_LIMIT = 1677931438L;

    /**
     * 原因：存在正在运行的任务。
     * 建议：手动停止任务，或等待任务运行结束。
     */
    public static final long HAVE_RUNNING_JOB = 1677935674L;

    /**
     * 原因：资源（{0}）已关联实时侦测策略。
     * 建议：请解除关联后重试。
     */
    public static final long RESOURCE_ALREADY_BIND_IO_DETECTION_POLICY = 1677931423L;

    /**
     * 原因：当前存储池与存储设备关联的存储单元已存在。
     * 建议：重新使用其他存储池和存储设备创建存储单元。
     */
    public static final long STORAGE_UNIT_EXISTED = 1677747968L;

    /**
     * 原因：备份存储单元名称重复。<br/>
     * 建议：请使用其他名称进行重试。<br/>
     */
    public static final long STORAGE_UNIT_NAME_EXISTED = 1677935117L;

    /**
     * 原因：即时恢复任务未完成。<br/>
     * 建议：请等待即时恢复任务完成后重试。<br/>
     */
    public static final long EXIST_INSTANCE_RESTORE_TASK = 1677934346L;

    /**
     * 原因：用户不是系统管理员。<br/>
     * 建议：请使用系统管理员用户进行操作。<br/>
     */
    public static final long NOT_SYSADMIN_ROLE = 1594050304L;

    /**
     * 原因：存储单元（组）已关联SLA策略。<br/>
     * 建议：请解除SLA策略关联关系后重试。<br/>
     */
    public static final long STORAGE_SLA_BINDING = 1677747971L;

    /**
     * 原因：用户添加的Exchange节点类型与实际的节点类型不一致。
     * 建议：请确保Exchange节点类型一致。
     */
    public static final long CLUSTER_NODES_TYPE_INCONSISTENT = 1677931460L;

    /**
     * 原因：输入的Exchange用户无管理角色组成员权限。
     * 建议：请使用具有权限的管理角色组成员再次尝试。
     */
    public static final long EXCHANGE_USER_NO_ACCESS = 1677931461L;

    /**
     * 原因：输入的用户名或者密码错误。
     * 建议：请输入正确的用户名和密码。
     */
    public static final String USERNAME_OR_PASSWORD_WRONG = "1077987870";

    /**
     * 原因：用户帐号已锁定。
     * 建议：登录失败的次数已超过限制，请##00秒后重试。
     */
    public static final String ACCOUNT_LOCKED_ERROR = "1077987871";

    /**
     *
     * 原因：用户名已存在。
     * 建议：请重新输入。
     */
    public static final String ADD_USERNAME_REPEATED = "1077949059";

    /**
     * 原因：用户添加的可用性组主机数量与实际的可用性组主机数量不一致。
     * 建议：请确保可用性组主机数量一致。
     */
    public static final long EXCHANGE_DAG_HOSTS_INCONSISTENT = 1677931462L;

    /**
     * 底座的错误码
     * 原因：指定的IP地址已经存在。
     * 建议：请重新输入IP地址。
     */
    public static final long IP_ADDRESS_REPEATED = 1073743379L;

    /**
     * 原因：域名或用户名输入错误。
     * 建议：请按照域名\用户名或用户名@域名格式输入正确的用户名，例如：DAG\Administrator或者Administrator@dag.com。
     */
    public static final long EXCHANGE_DOMAIN_USERNAME_INVALID = 1677931458L;

    /**
     * 原因：所选的代理主机不属于同一可用性组。
     * 建议：请确保所选的代理主机属于同一可用性组。
     */
    public static final long AGENT_NOT_BELONG_TO_SAME_DAG = 1677931466L;

    /**
     * 错误场景：执行LLD_Design配置文件参数解析操作时，由于参数不满足要求，操作失败。
     * 原因：{0}参数({1})不满足要求。
     * 建议：请修改为满足要求的参数后重试。
     */
    public static final long LLD_FILE_PARSED_ERROR = 1677935676L;

    /**
     * 错误场景：执行创建绑定类型业务端口操作时，由于选择的以太网端口不满足要求，操作失败。
     * 原因：选择的以太网端口{0}{1}{2}。
     * 建议：1、请选择未创建VLAN的以太网端口后重试。
     * 2、请选择未创建绑定端口的以太网端口后重试。
     * 3、请选择未在自定义漂移组中的以太网端口后重试。
     * 4、请选择未创建逻辑端口的以太网端口后重试。
     */
    public static final long CREATE_BOND_FAIL = 1677929239L;

    /**
     * 错误场景：创建绑定类型业务端口时，由于选择的以太网端口未在同一个接口模块上，操作失败。
     * 原因：选择的以太网端口未在同一个接口模块上。
     * 建议：请选择同一个接口模块的以太网端口后重试。
     */
    public static final long ETH_PORT_NOT_ON_SAME_CARD_CREATE_BOND_FAIL = 1677929238L;

    /**
     * 错误场景：执行VMware RDM盘备份操作时，由于未查询到RDM盘所在的存储设备信息，操作失败。
     * 原因：未查询到RDM盘所在的存储设备信息。
     * 建议：请检查RDM和ESXI的映射是否建立或者检查虚拟机RDM盘是否存在wwn信息后重试。
     */
    public static final long NO_RDM_STORAGE_FOUND = 1677931467L;

    /**
     * 错误场景：执行Vmware备份操作时，无法获取RDM盘的wwn信息，操作失败。
     * 原因：无法获取RDM盘的wwn信息。
     * 建议：请检查虚拟机RDM盘是否存在wwn信息后重试。
     */
    public static final long CAN_NOT_GET_RDM_WWN = 1677929229L;

    /**
     * 错误场景：执行配置业务端口操作时，由于端口配置IP的网段与端口中现有IP的网段不相同，操作失败。
     * 原因：由于端口所配置IP的网段（{0}）与端口中现有IP的网段（{1}）不相同。
     * 建议：请修改IP信息后重试。
     */
    public static final long IP_NETWORK_SEGMENT_LIMIT = 1677935129L;

    /**
     * 错误场景：执行关联或解除关联文件系统的文件扩展名过滤规则、禁用或开启文件拦截功能操作时，由于文件系统所在设备离线，操作失败。
     * 原因：文件系统所在设备离线。
     * 建议：请调整设备状态为在线后重试。
     */
    public static final long ERROR_DEVICE_OFFLINE = 1593990418L;

    /**
     * 错误场景：执行配置业务端口操作时，由于端口配置数量超出限制，操作失败。
     * 原因：（{0}）端口配置数量超出限制（{1}）。
     * 建议：请调整业务端口数量后重试。
     */
    public static final long SERVICE_PORT_NUM_EXCEED_LIMIT = 1677935128L;

    /**
     * 错误场景：执行LLD配置文件上传操作时，由于LLD配置文件格式不满足要求，操作失败。
     * 原因：LLD配置文件格式不满足要求。
     * 建议：请上传正确的LLD配置文件后重试。
     */
    public static final long LLD_FILE_VALIDATE_FAILED = 1677935677L;

    /**
     * 错误场景：执行修改或删除用户时，由于不允许修改或删除系统内置用户，操作失败。
     * 原因：不允许修改或删除系统内置用户。
     * 建议：无。
     */
    public static final long DELETE_DEFAULT_ROLE_FAILED = 1677929522L;

    /**
     * 错误场景：执行删除角色操作时，由于该角色关联了用户，操作失败。
     * 原因：该角色关联了用户。
     * 建议：无。
     */
    public static final long ROLE_ASSOCIATED_USERS_DELETE_FAILED = 1677930081L;

    /**
     *
     * 错误场景：执行细粒度恢复操作时， 由于选择的细粒度恢复对象数量超过限制，操作失败。
     * 原因：选择的细粒度恢复对象数量超过限制（{}）。
     * 建议：请调整选择的细粒度恢复对象数量。
     */
    public static final long RESTORE_SUB_OBJECT_NUM_MAX_LIMIT = 1677933076L;

    /**
     *
     * 错误场景：执行细粒度恢复操作时， 由于选择的细粒度恢复对象路径过长和数量过多，操作失败。
     * 原因：选择的细粒度恢复对象路径过长和数量过多。
     * 建议：细粒度恢复对象路径过长时不要选择过多的资源数量。
     */
    public static final long RESTORE_SUB_OBJECT_MAX_BYTES = 1677933077L;

    /**
     * 错误场景：执行删除副本操作，由于该副本正在进行防勒索检测，操作失败。
     * 原因：该副本正在进行防勒索检测。
     * 建议：请等待防勒索检测完成后重试。
     * 适用于HyperDetect X8000
     */
    public static final long COPY_IS_DETECTING = 1677933328L;

    /**
     * 错误场景：执行删除快照操作，由于该快照正在进行防勒索检测，操作失败。
     * 原因：该快照正在进行防勒索检测。
     * 建议：请等待防勒索检测完成后重试。
     * 适用于OceanCyber
     */
    public static final long SNAPSHOT_IS_DETECTING = 1677933328L;

    /**
     *
     * 错误场景：执行任务重试操作时，由于任务的处理状态不是已处理，操作失败。
     * 原因：任务（{0}）的处理状态不是已处理。
     * 建议：请先标记任务的处理意见，等待任务的处理状态更新为已处理后重试。
     */
    public static final long JOB_MARK_STATUS_IS_NOT_MARKED = 1677934347L;

    /**
     *
     * 错误场景：执行任务重试操作时，由于创建重试任务失败，操作失败。
     * 原因：创建重试任务（{0}）失败。
     * 建议：请稍后进行重试，若重试任务依然失败，请收集日志并联系技术支持工程师协助解决。
     */
    public static final long JOB_RETRY_FAILED = 1677934348L;

    /**
     *
     * 错误场景：查询副本防勒索检测状态，副本数据不存在
     * 原因：副本不存在检测任务。
     * 建议：创建副本防勒索任务后查询。
     */
    public static final long COPY_NOT_EXIST_ERROR = 1677933323L;

    /**
     * 用户状态异常
     */
    public static final long USER_STATUS_ABNORMAL = 1677929505L;

    /**
     *
     * 错误场景：本地盘不支持即时挂载，操作失败。
     * 原因：本地盘不支持即时挂载，操作失败
     */
    public static final long LIVE_MOUNT_NOT_SUPPORT_BASIC_DISK = 1677932814L;

    /**
     * 原因：未指定证书。
     * 建议：请指定证书后重试，或者使用http协议。
     */
    public static final long NO_CERTIFICATE_PROVIDED = 1677930293L;

    /**
     * 错误场景：执行添加备份存储设备时，由于仅支持OceanProtect X3000/X6000/X8000 1.5.0及以上版本的存储设备，所以操作失败。
     * 原因：仅支持OceanProtect X3000/X6000/X8000 1.5.0及以上版本的存储设备。
     * 建议：请更换OceanProtect X3000/X6000/X8000 1.5.0及以上版本的存储设备。
     */
    public static final long TARGET_CLUSTER_NOT_SUPPORT = 1677930086L;

    /**
     * 连接LDAP失败错误
     */
    public static final long CONNECT_LDAP_SERVER_FAILED = 1677929481L;

    /**
     * 错误场景：执行测试连接LDAP服务操作时，由于域名解析失败，操作失败。
     * 原因：域名（{0}）解析失败。
     * 建议：请前往DeviceManager设置DNS域名服务器，并确保DNS服务器上已配置域名（{0}）。
     */
    public static final long DOMAIN_NAME_RESOLVED_FAILED = 1677935647L;

    /**
     * ADFS链接网络超时
     */
    public static final long CONNECT_ADFS_SERVER_TIMEOUT = 1677935652L;

    /**
     * SLA POLICY不存在
     */
    public static final long SLA_POLICY_NOT_EXIST = 1677931554L;

    /**
     * 错误场景：执行SLA操作时，由于WORM保留时间大于副本保留时间，操作失败。
     * 原因：WORM保留时间大于副本保留时间。
     * 建议：请检查WORM保留时间是否小于等于副本保留时间。
     */
    public static final long WORM_RETENTION_TIME_ERROR = 1677929523L;

    /**
     * 错误场景：执行SLA操作时，由于WORM保留时间大于20年或7300天，操作失败。
     * 原因：WORM保留时间大于20年或7300天。
     * 建议：请检查WORM保留时间是否小于等于20年或7300天。
     */
    public static final long WORM_TIME_EXCEEDS_MAX_TIME_ERROR = 1677929524L;

    /**
     * 错误场景：执行WORM设置操作时，由于当前WORM有效时间小于以前WORM设置时间，操作失败。
     * 原因：WORM过期时间小于以前WORM设置时间。
     * 建议：请检查WORM有效时间是否大于等于以前WORM设置时间。
     */
    public static final long MODIFY_WORM_VALIDITY_TIME_LESS_THAN_EXIST_TIME_ERROR = 1677932055L;

    /**
     * 错误场景：执行修改WORM设置操作时，由于当前设置WORM过期时间大于之前副本过期时间，操作失败。
     * 原因：当前WORM过期时间大于之前副本过期时间。
     * 建议：请检查当前WORM设置时间是否小于等于副本过期时间。
     */
    public static final long MODIFY_WORM_VALIDITY_TIME_EXCEEDS_COPY_RETENTION_TIME_ERROR = 1677932056L;

    /**
     * 错误场景：执行修改副本过期时间操作时，由于副本过期时间小于WORM过期时间，操作失败。
     * 原因：副本过期时间小于WORM过期时间。
     * 建议：请检查副本过期时间是否大于等于WORM过期时间。
     */
    public static final long COPY_RETENTION_TIME_ERROR = 1677933348L;

    /**
     * 错误场景：执行保护操作时，由于该资源对应WORM策略已开启且选中SLA的WORM设置也开启，操作失败。
     * 原因：该资源的WORM策略和SLA中WORM设置不能同时开启。
     * 建议：请取消防勒索中的WORM设置开关或取消选择SLA中的WORM设置开关。
     */
    public static final long BOTH_SLA_WORM_AND_ANTI_RANSOMWARE_WORM_TURN_ON = 1677932057L;

    /**
     * 错误场景：执行SLA操作时，由于复制策略中副本保留时间小于WORM的保留时间，操作失败。
     * 原因：复制策略中副本保留时间小于WORM的保留时间。
     * 建议：请检查复制策略中副本保留时间是否大于等于WORM的保留时间。
     */
    public static final long COPY_RETENTION_TIME_LESS_THAN_WORM_RETENTION_TIME_ERROR = 1677931553L;

    /**
     * 错误场景：执行SLA操作时，由于复制策略中副本保留时间小于WORM的保留时间，操作失败。
     * 原因：复制策略中副本保留时间小于WORM的保留时间。
     * 建议：请检查复制策略中副本保留时间是否大于等于WORM的保留时间。
     */
    public static final long BASIC_DISK_NOT_SUPPORT_WORM_AND_ANTI = 1677931555L;

    /**
     * 错误场景：在代理主机上创建安装目录时，由于用户权限不足、磁盘空间不足或者文件系统只读等原因，操作失败。
     * 原因：用户权限不足、磁盘空间不足或者文件系统只读等。
     * 建议：请在“ProtectAgent安装指南”中搜索“安装目录”进行处理。
     */
    public static final long CLIENT_REGISTER_FAILED = 1677873409L;

    /**
     * 错误场景：执行更换资源所属用户时，由于所修改用户配额不足，操作失败。
     * 原因：当前用户的用户类型为（{0}），该用户的（{1}）配额不足导致操作失败。
     * 建议：具体操作请参见“管理员指南”中的“设置配额”章节。
     */
    public static final long USER_QUOTA_NOT_ENOUGH = 1677931479L;

    /**
     * 错误场景：执行更换资源所属用户时，由于待更换的用户不是数据保护管理员，操作失败。
     * 原因：待更换的用户不是数据保护管理员。
     * 建议：选择其他用户后重试。
     */
    public static final long USER_NOT_DP_ADMIN = 1677931480L;

    /**
     * 请求超时错误码
     */
    public static final long REQUEST_TIMEOUT = 1677929226L;

    /**
     * 错误场景：执行创建端口时，选择端口类型是绑定端口，由于填写绑定端口名称在底座已存在，创建失败。
     * 原因：创建的绑定端口名称在底座已存在，创建失败。
     * 建议：请重新输入绑定端口名称后重试。
     */
    public static final long ERROR_BOND_PORT_NAME = 1677929525L;

    /**
     * 动态口令邮件发送失败
     */
    public static final long SEND_EMAIL_DYNAMIC_PWD_FAILED = 1677936128L;

    /**
     * 错误场景：执行注册受保护PDB集操作时，由于该受保护PDB集内存在已经注册的PDB，操作失败。
     * 原因：存在已经被注册的PDB:（{0}）。
     * 建议：请选择未被注册的PDB。
     */
    public static final long PROTECTED_PDB_REPEATED = 1677747713L;

    /**
     * 错误场景：执行创建端口操作时，选择端口类型是绑定端口，由于选择的以太网口已被其它绑定端口占用，创建失败。
     * 原因：绑定端口({0})选择的以太网端口已被其它绑定端口占用。
     * 建议：请选择其它未被占用的以太网端口后重试。
     */
    public static final long ERROR_DIFFERENT_BOND_PORT_NAME = 1677929526L;

    /**
     * 错误场景：创建/修改SLA时，域内复制的存储id与备份策略的存储id相同，报错。
     * 原因：域内复制的存储id与备份策略的存储id不能相同。
     * 建议：请修改域内复制策略或备份策略的存储单元。
     */
    public static final long ERROR_BACKUP_REP_SAME_STORAGEID = 1677749250L;

    private CommonErrorCode() {
    }
}
