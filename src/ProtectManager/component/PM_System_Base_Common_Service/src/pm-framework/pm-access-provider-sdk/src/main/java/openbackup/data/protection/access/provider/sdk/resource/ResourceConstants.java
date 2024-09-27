package openbackup.data.protection.access.provider.sdk.resource;

import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import java.util.Arrays;
import java.util.Collections;
import java.util.List;

/**
 * 资源常量
 *
 * @since 2022-05-23
 */
public class ResourceConstants {
    /**
     * 代表parent-children的key
     */
    public static final String CHILDREN = "children";

    /**
     * 被依赖的key前缀
     */
    public static final String CITATION = "$citations";

    /**
     * citations分隔符
     */
    public static final String CITATION_SEPERATOR = "_";

    /**
     * sourceType - register
     */
    public static final String SOURCE_TYPE_REGISTER = "register";

    /**
     * sourceType - auto
     */
    public static final String SOURCE_TYPE_AUTO_SCAN = "autoscan";

    /**
     * agent返回的所有ip地址key，存在于HostDTO.extendInfo
     */
    public static final String AGENT_IP_LIST = "agentIpList";

    /**
     * agent返回nginx端口的信息
     */
    public static final String AGENT_NGINX_PORT = "agentNginxPort";

    /**
     * agentfc配置关键字，资源的扩展信息字段key
     */
    public static final String IS_ADD_LAN_FREE = "isAddLanFree";

    /**
     * agentfc配置关键字，资源的扩展信息字段value
     */
    public static final String LAN_FREE_YES = "1";

    /**
     * 扩展表列名 key
     */
    public static final String KEY = "key";

    /**
     * 扩展表列名 resource_id
     */
    public static final String RESOURCE_ID = "resource_id";

    /**
     * 环境更新锁 key
     */
    public static final String UPDATE_ENV_LOCK = "/update_environment_lock";

    /**
     * 资源更新锁 key
     */
    public static final String UPDATE_RESOURCE_LOCK = "/update_resource_lock";

    /**
     * 资源更新锁等待超时时间 30秒
     */
    public static final int RESOURCE_LOCK_WAIT_TIME_OUT = 30;

    // 待实现
    /**
     * agentfc配置关键字，资源的扩展信息字段key 集群esn
     */
    public static final String CLUSTER_ESN = "clusterEsn";

    /**
     * 扩展表列名 value
     */
    public static final String VALUE = "value";

    /**
     * 项目/资源集id
     */
    public static final String PROJECT_ID = "project_id";

    /**
     * 区域id,region_id
     */
    public static final String REGION_ID = "region_id";

    /**
     * cloudHost的项目/资源集id
     */
    public static final String CLOUD_SERVER_PROJECT_ID = "cloud_server_project_id";

    /**
     * 是否是云桌面
     */
    public static final String IS_WORKSPACE = "isWorkspace";

    /**
     * 云桌面详情
     */
    public static final String WORKSPACE_INFO = "workspaceInfo";

    /**
     * hcs domainId
     */
    public static final String DOMAIN_ID = "domainId";

    /**
     * cloudHost的状态
     */
    public static final String CLOUD_SERVER_STATUS = "status";

    /**
     * cloudHost的信息
     */
    public static final String CLOUD_SERVER_HOST = "host";

    /**
     * 可用区域，标记agent所在az
     */
    public static final String AVAILABLE_ZONE = "availableZone";

    /**
     * agent上报的vpcid
     */
    public static final String VPC_ID = "vpcId";

    /**
     * agent类型的key
     */
    public static final String AGENT_TYPE_KEY = "scenario";

    /**
     * 是否允许恢复的key
     */
    public static final String IS_ALLOW_RESTORE_KEY = "isAllowRestore";

    /**
     * 允许恢复的value
     */
    public static final String ALLOW_RESTORE = "true";

    /**
     * 不允许恢复的value
     */
    public static final String NOT_ALLOW_RESTORE = "false";

    /**
     * 总是允许恢复的资源类型列表
     */
    public static final List<String> ALWAYS_ALLOW_RESTORE_RESOURCE = Collections.unmodifiableList(
        Arrays.asList(ResourceSubTypeEnum.GAUSSDB_DWS_DATABASE.getType()));
}
