/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

package openbackup.openstack.protection.access.constant;

/**
 * OpenStack常量类
 *
 * @author x30038064
 * @version [OceanProtect DataBackup 1.3.0]
 * @since 2022-12-24
 */
public class OpenstackConstant {
    /**
     * 是否注册到云核Openstack
     */
    public static final String REGISTER_OPENSTACK = "true";

    /**
     * 是否注册到云核Openstack key
     */
    public static final String REGISTER_SERVICE = "register_service";

    /**
     * 生产环境的domain id key
     */
    public static final String DOMAIN_ID_KEY = "domainId";

    /**
     * domain name key
     */
    public static final String DOMAIN_NAME_KEY = "domainName";

    /**
     * 连通性检查判断是否为域的key
     */
    public static final String IS_DOMAIN = "isDomain";

    /**
     * 扫描时的资源类型key
     */
    public static final String RESOURCE_TYPE_KEY = "resourceType";

    /**
     * 扫描时的可用区域资源类型
     */
    public static final String RES_TYPE_AVAILABILITY_ZONE = "availabilityZone";

    /**
     * 扫描时的域资源类型
     */
    public static final String RES_TYPE_DOMAIN = "domain";

    /**
     * 扫描时的项目资源类型
     */
    public static final String RES_TYPE_PROJECT = "project";

    /**
     * 扫描时的云主机资源类型
     */
    public static final String RES_TYPE_SERVER = "server";

    /**
     * 扫描时的云硬盘资源类型
     */
    public static final String RES_TYPE_VOLUME = "volume";

    /**
     * 扫描时的marker标记key
     */
    public static final String SCAN_MARKER_KEY = "marker";

    /**
     * 云主机的磁盘信息映射key
     */
    public static final String VOLUME_INFO_KEY = "volInfo";

    /**
     * 环境的域名称默认值
     */
    public static final String DEFAULT_DOMAIN = "Default";

    /**
     * 备份下发的项目ID key
     */
    public static final String PROJECT_ID = "projectId";

    /**
     * 备份下发的是否全部磁盘参数key
     */
    public static final String ALL_DISK = "all_disk";

    /**
     * 保护对象中包含的磁盘id列表key
     */
    public static final String DISK_IDS = "disk_info";

    /**
     * 响应结果中的ID key
     */
    public static final String ID_KEY = "id";

    /**
     * 响应结果中的keystone service id key，对应唯一的keystone
     */
    public static final String SERVICE_ID_KEY = "service_id";

    /**
     * 项目下的云主机数量(保护时显示) key
     */
    public static final String CLOUD_SERVER_COUNT = "cloudServerCount";

    /**
     * 恢复级别 key 0:虚拟机恢复 1:卷恢复
     */
    public static final String RESTORE_LEVEL = "restoreLevel";

    /**
     * 上下电状态 key 0:下电 1:上电
     */
    public static final String POWER_STATE = "powerState";

    /**
     * 恢复的目标位置 key
     */
    public static final String RESTORE_LOCATION = "restoreLocation";

    /**
     * 恢复级别:虚拟机恢复
     */
    public static final String VM_RESTORE = "0";

    /**
     * 恢复级别:卷恢复
     */
    public static final String VOLUME_RESTORE = "1";

    /**
     * 上电状态：上电
     */
    public static final String POWER_ON = "1";

    /**
     * 上电状态：下电
     */
    public static final String POWER_OFF = "0";

    /**
     * 虚拟机状态
     */
    public static final String SERVER_STATUS = "status";

    /**
     * 虚拟机状态值，使用中
     */
    public static final String SERVER_STATUS_ACTIVE = "active";

    /**
     * 是否进行副本校验
     */
    public static final String COPY_VERIFY = "copyVerify";

    /**
     * 扩展参数
     */
    public static final String EXTEND_INFO_KEY = "extendInfo";

    /**
     * 目标卷
     */
    public static final String TARGET_VOLUME_KEY = "targetVolume";

    /**
     * uuid
     */
    public static final String UUID_KEY = "uuid";

    /**
     * 卷大小
     */
    public static final String VOLUME_SIZE_KEY = "size";

    /**
     * 是否开启证书校验
     */
    public static final String ENABLE_CERT = "enableCert";

    /**
     * 开启证书校验
     */
    public static final String CERT_ENABLE = "1";

    /**
     * 证书内容
     */
    public static final String CERTIFICATION = "certification";

    /**
     * 吊销列表内容
     */
    public static final String REVOCATION_LIST = "revocationList";

    /**
     * 证书名称
     */
    public static final String CERT_NAME = "certName";

    /**
     * 证书大小
     */
    public static final String CERT_SIZE = "certSize";

    /**
     * 吊销列表名称
     */
    public static final String CRL_NAME = "crlName";

    /**
     * 吊销列表大小
     */
    public static final String CRL_SIZE = "crlSize";

    /**
     * 空字符串
     */
    public static final String EMPTY = "";

    /**
     * 配额指定用户名
     */
    public static final String QUOTA_USER_NAME = "openstack-quota";

    /**
     * 配额指定用户ID
     */
    public static final String QUOTA_USER_ID = "quotaUserId";

    /**
     * agent接口响应成功码
     */
    public static final String SUCCESS_CODE = "0";

    /**
     * 环境依赖的主机key
     */
    public static final String AGENTS = "agents";

    /**
     * 开启备份一致性快照
     */
    public static final String OPEN_CONSISTENT_SNAPSHOTS = "open_consistent_snapshots";

    /**
     * openstack最大平台数
     */
    public static final int MAX_OPENSTACK_COUNT = 8;

    /**
     * openstack最大域名数
     */
    public static final int MAX_DOMAIN_COUNT = 256;

    /**
     * 本地存储esn
     */
    public static final String ESN = "esn";

    /**
     * domain可见性 key
     */
    public static final String VISIBLE = "visible";

    /**
     * domain项目数量 key
     */
    public static final String PROJECT_COUNT = "projectCount";

    /**
     * 虚拟机错误状态
     */
    public static final String ERROR_STATUS = "error";

    /**
     * 克隆卷类型
     */
    public static final String CLONE_VOL = "0";

    /**
     * 健康检查
     */
    public static final String HEALTH_CHECK = "healthCheck";

    /**
     * Default域默认id
     */
    public static final String DEFAULT_DOMAIN_ID = "default";

    /**
     * 环境扫描间隔 24小时
     */
    public static final Integer SCAN_INTERVAL = 24 * 3600;

    /**
     * 扫描时的OpenStack云硬盘类型资源类型
     */
    public static final String RES_TYPE_VOLUME_TYPE = "volumeType";

    /**
     * 扫描时的OpenStack的项目的flavor资源类型
     */
    public static final String PROJECT_FLAVOR = "flavor";

    /**
     * 扫描时的OpenStack的项目的网络资源类型
     */
    public static final String PROJECT_NETWORK = "network";

    /**
     * 扫描时的OpenStack的磁盘是否是新建磁盘
     */
    public static final String IS_NEW_DISK = "isNewDisk";
}
