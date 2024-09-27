/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2020. All rights reserved.
 */

package openbackup.system.base.common.constants;

/**
 * 操作事件对应code
 *
 * @author t00482481
 * @since 2020-8-10
 */
public class CommonOperationCode {
    /**
     * 登录相关操作码
     */
    public static final String LOGIN = "0x2064032B0001";

    /**
     * 发送验证码
     */
    public static final String SEND_DYNAMIC_CODE = "0x2064032B0019";

    /**
     * 创建用户相关操作码
     */
    public static final String CREATE_USER = "0x2064032B0007";

    /**
     * 删除用户相关操作码
     */
    public static final String DELETE_USER = "0x2064032B0008";

    /**
     * 修改用户信息相关操作码
     */
    public static final String MODIFY_USER = "0x2064032B0009";

    /**
     * 修改用户找回密码邮箱操作码
     */
    public static final String MODIFY_AUTH_KEY_RETRIEVAL_EMAIL = "0x206400770002";

    /**
     * 用户手动注销相关操作码
     */
    public static final String MANUAL_LOGOUT = "0x2064032B0012";

    /**
     * 用户超时退出相关操作码
     */
    public static final String TIMEOUT_LOGOUT = "0x2063032B0001";

    /**
     * 用户被锁定导致强制下线相关操作码
     */
    public static final String LOCK_LOGOUT = "0x2064032B0015";

    /**
     * 修改用户密码相关操作码
     */
    public static final String MODIFY_AUTH_KEY = "0x2064032B000A";

    /**
     * 重置用户密码相关操作码
     */
    public static final String RESET_PWD = "0x2064032B000B";

    /**
     * 锁定用户相关操作码
     */
    public static final String LOCK_USER = "0x2064032B000C";

    /**
     * 多次修改密码锁定用户相关操作码
     */
    public static final String SYSTEM_LOCK_USER = "0x2064032B000F";

    /**
     * 解锁用户相关操作码
     */
    public static final String UNLOCK_USER = "0x2064032B000D";

    /**
     * 自动解锁用户相关操作码
     */
    public static final String AUTO_UNLOCK_USER = "0x2064032B0011";

    /**
     * 修改安全策略相关操作码
     */
    public static final String MODIFY_SEC_STRATEGY = "0x2064032B000E";

    /**
     * 安全一体机修改安全策略相关操作码
     */
    public static final String OC_MODIFY_SEC_STRATEGY = "0x2064032B0026";

    /**
     * 新增弱口令
     */
    public static final String ADD_WEAK_AUTH_KEY = "0x2064032B0010";

    /**
     * 删除弱口令
     */
    public static final String DELETE_WEAK_AUTH_KEY = "0x206400770001";

    /**
     * 发送临时验证码
     */
    public static final String SEND_VERIFY_CODE = "0x2064032B0016";

    /**
     * 校验临时验证码
     */
    public static final String CHECK_VERIFY_CODE = "0x2064032B0017";

    /**
     * 重置忘记的密码
     */
    public static final String RESET_FORGOTTEN_PASSWORD = "0x2064032B0018";

    /**
     * 添加存储用户
     */
    public static final String ADD_STORAGE_USER = "0x2064032D0001";

    /**
     * 删除存储用户
     */
    public static final String DELETE_STORAGE_USER = "0x2064032D0002";

    /**
     * 修改存储用户名称
     */
    public static final String CHANGE_STORAGE_USER_NAME = "0x2064032D0003";

    /**
     * 添加复制集群
     */
    public static final String ADD_LINK_CLUSTER = "0x2064032D0004";

    /**
     * 验证对端用户帐户密码
     */
    public static final String VERIFY_REMOTE_USER = "0x2064032D0034";

    /**
     * 修改复制集群
     */
    public static final String MODIFY_LINK_CLUSTER = "0x2064032D0005";

    /**
     * 修改集群az
     */
    public static final String MODIFY_LINK_CLUSTER_AZ = "0x2064032D0030";

    /**
     * 删除复制集群
     */
    public static final String DELETE_LINK_CLUSTER = "0x2064032D0006";

    /**
     * 修改系统备份策略
     */
    public static final String MODIFY_SYSTEM_BACKUP_POLICY = "0x2064033F0001";

    /**
     * 执行系统备份
     */
    public static final String RUN_SYSTEM_BACKUP = "0x2064033F0002";

    /**
     * 执行系统恢复
     */
    public static final String RUN_SYSTEM_RECOVERY = "0x2064033F0003";

    /**
     * 上传系统备份
     */
    public static final String UPLOAD_SYSTEM_BACKUP = "0x2064033F0004";

    /**
     * 下载系统备份
     */
    public static final String DOWNLOAD_SYSTEM_BACKUP = "0x2064033F0005";

    /**
     * 删除系统备份
     */
    public static final String DELETE_SYSTEM_BACKUP = "0x2064033F0006";

    /**
     * 停止任务
     */
    public static final String JOB_CENTER_STOP_TASK = "0x2064033E0001";

    /**
     * 导出任务
     */
    public static final String JOB_CENTER_EXPORT = "0x2064033E0002";

    /**
     * 设置任务备注信息
     */
    public static final String JOB_CENTER_TAG = "0x2064033E0003";

    /**
     * 标记任务处理意见
     */
    public static final String JOB_MARK = "0x2064033E0006";

    /**
     * 任务重试
     */
    public static final String JOB_RETRY = "0x2064033E0007";

    /**
     * 操作性能开关
     */
    public static final String CLUSTER_PERFORMANCE_SWITCH = "0x2064032D0007";

    /**
     * 操作性能开关
     */
    public static final String DELETE_HISTORY_PERFORMANCE_DATA = "0x2064032D0008";

    /**
     * X系列创建初始化网络
     */
    public static final String CREATE_X_SERIES_INITCONFIG = "0x2064032E0022";

    /**
     * 分布式创建初始化网络
     */
    public static final String CREATE_E6000_INITCONFIG = "0x2064032E0023";

    /**
     * 软硬解耦创建初始化网络
     */
    public static final String CREATE_DEPENDENT_INITCONFIG = "0x2064032E0024";

    /**
     * 创建pacific初始化网络
     */
    public static final String CREATE_PACIFIC_INIT_CONFIG = "0x206403460017";

    /**
     * 修改存储认证信息
     */
    public static final String MODIFY_STORAGEAUTH = "0x2064032E0005";

    /**
     * 初始化时配置存储认证
     */
    public static final String INIT_CONFIG_STORAGE_AUTH = "0x206403460016";

    /**
     * 初始化解析lld
     */
    public static final String INITIALIZE_UPLOAD_LLD = "0x20640346001D";

    /**
     * 更新本地存储超级管理员密码
     */
    public static final String UPDATE_LOCAL_STORAGE = "0x2064032E0021";

    /**
     * 修改生产存储信息
     */
    public static final String UPDATE_PRODUCT_STORAGE = "0x2064032E0006";

    /**
     * 删除生产存储信息
     */
    public static final String DELETE_PRODUCT_STORAGE = "0x2064032E0008";

    /**
     * 接入生产存储信息
     */
    public static final String ACCESS_PRODUCT_STORAGE = "0x2064032E0007";

    /**
     * 导入License
     */
    public static final String LICENSE_IMPORT = "0x2064032F0001";

    /**
     * 导出License
     */
    public static final String LICENSE_EXPORT = "0x2064032F0002";

    /**
     * 更新告警阈值
     */
    public static final String UPDATE_THRESHOLD = "0x2064032E0009";

    /**
     * 更新密钥文件
     */
    public static final String UPDATE_KEY = "0x206403300001";

    /**
     * 修改密钥更新周期
     */
    public static final String UPDATE_KEY_LIFETIME = "0x206403300002";

    /**
     * 扩容服务操作
     */
    public static final String EXPANSION_OF_NETWORK = "0x2064032E000C";

    /**
     * 注册外部证书组件
     */
    public static final String EXTERNAL_COMPONENT_REGISTERED = "0x206403310001";

    /**
     * 导入（更新）证书组件
     */
    public static final String CERTIFICATE_IMPORTED = "0x206403310002";

    /**
     * 删除证书组件
     */
    public static final String COMPONENTS_DELETED = "0x206403310004";

    /**
     * 同步kmc
     */
    public static final String SYNC_KMC = "0x2064032D0012";

    /**
     * 回退kmc
     */
    public static final String ROLLBACK_KMC = "0x2064032D0013";

    /**
     * 同步证书
     */
    public static final String SYNC_CERT = "0x2064032D0010";

    /**
     * 回退证书
     */
    public static final String ROLLBACK_CERT = "0x2064032D0011";

    /**
     * 任务执行
     */
    public static final String EXECUTE_JOB = "0x2064033E0004";

    /**
     * 任务完成
     */
    public static final String JOB_COMPLETE = "0x2064033E0005";

    /**
     * 资源加锁
     */
    public static final String LOCK_RESOURCE = "0x20640332003E";

    /**
     * 资源解锁
     */
    public static final String UNLOCK_RESOURCE = "0x20640332003F";

    /**
     * 断开链路
     */
    public static final String DISCONNECT_LINK = "0x20640344000E";

    /**
     * 备份存储单元升级为成员节点
     */
    public static final String UPGRADE_BACKUP_STORAGE_UNIT = "0x2064032D002E";

    /**
     * 更新初始化状态
     */
    public static final String UPDATE_STATUS_INFO = "0x206403460011";

    /**
     * 同步秘钥和证书
     */
    public static final String SYNC_KEY_AND_CERT = "0x2064032D0031";

    /**
     * 回退秘钥和证书
     */
    public static final String ROLLBACK_KEY_AND_CERT = "0x2064032D0032";

    /**
     * 新增逻辑端口
     */
    public static final String ADD_LOGIC_PORT = "0x206403460012";

    /**
     * 删除逻辑端口
     */
    public static final String DELETE_LOGIC_PORT = "0x206403460014";

    /**
     * 修改逻辑端口
     */
    public static final String MODIFY_LOGIC_PORT = "0x206403460015";

    /**
     * 新增绑定端口
     */
    public static final String ADD_BOUND_PORT = "0x206403460013";

    /**
     * 新增绑定端口
     */
    public static final String ADD_PORT_ROUTE = "0x20640346001B";

    /**
     * 新增绑定端口
     */
    public static final String DELETE_PORT_ROUTE = "0x20640346001C";

    /**
     * 查询HCS用户全部project信息
     */
    public static final String QUERY_PROJECT = "0x2064032D003A";

    /**
     * CREATE_RESOURCE_GROUP
     */
    public static final String CREATE_RESOURCE_GROUP = "0x206403320046";

    /**
     * UPDATE_RESOURCE_GROUP
     */
    public static final String UPDATE_RESOURCE_GROUP = "0x206403320047";

    /**
     * DELETE_RESOURCE_GROUP
     */
    public static final String DELETE_RESOURCE_GROUP = "0x206403320048";

    /**
     * CREATE_RESOURCE_GROUP_PROTECTION
     */
    public static final String CREATE_RESOURCE_GROUP_PROTECTION = "0x206403320049";

    /**
     * UPDATE_RESOURCE_GROUP_PROTECTION
     */
    public static final String UPDATE_RESOURCE_GROUP_PROTECTION = "0x20640332004A";

    /**
     * DELETE_RESOURCE_GROUP_PROTECTION
     */
    public static final String DELETE_RESOURCE_GROUP_PROTECTION = "0x20640332004B";

    /**
     * 激活保护
     */
    public static final String ACTIVATE_PROTECTION = "0x206403340004";

    /**
     * 禁用保护
     */
    public static final String DEACTIVATE_PROTECTION = "0x206403340005";

    /**
     * ADD_EXTERNAL_SYSTEM
     */
    public static final String ADD_EXTERNAL_SYSTEM = "0x206403460018";

    /**
     * DELETE_EXTERNAL_SYSTEM
     */
    public static final String DELETE_EXTERNAL_SYSTEM = "0x206403460019";

    /**
     * UPDATE_EXTERNAL_SYSTEM
     */
    public static final String UPDATE_EXTERNAL_SYSTEM = "0x20640346001A";

    /**
     * 修改pacific节点业务网络
     */
    public static final String UPDATE_NODE_BUSINESS_NETWORK = "0x2064032D0033";

    /**
     * 修改pacific业务网络
     */
    public static final String UPDATE_BUSINESS_NETWORK = "0x2064032D0039";

    /**
     * 创建资源集
     */
    public static final String CREATE_RESOURCE_SET = "0x2064032B0022";

    /**
     * 修改资源集
     */
    public static final String UPDATE_RESOURCE_SET = "0x2064032B0024";

    /**
     * 删除资源集
     */
    public static final String DELETE_RESOURCE_SET = "0x2064032B0023";

    /**
     * 创建角色
     */
    public static final String CREATE_ROLE = "0x2064032B0025";

    /**
     * 修改角色
     */
    public static final String UPDATE_ROLE = "0x2064032B0021";

    /**
     * 删除角色
     */
    public static final String DELETE_ROLE = "0x2064032B0020";
}
