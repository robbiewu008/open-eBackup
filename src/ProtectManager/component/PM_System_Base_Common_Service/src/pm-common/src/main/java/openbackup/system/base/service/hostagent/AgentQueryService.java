package openbackup.system.base.service.hostagent;

import openbackup.system.base.common.model.PageListResponse;
import openbackup.system.base.service.hostagent.model.AgentInfo;

import java.util.List;
import java.util.Map;

/**
 * 客户端内部查询服务
 *
 * @author q00654632
 * @since 2023-10-08
 */
public interface AgentQueryService {
    /**
     * 查询客户端接口
     *
     * @param pageSize pageSize
     * @param pageNo pageNo
     * @param conditions conditions
     * @return 分页的客户端信息 PageListResponse<AgentInfo>
     */
    PageListResponse<AgentInfo> queryAgents(int pageSize, int pageNo, Map<String, Object> conditions);

    /**
     * 查询共享客户端id
     *
     * @return 共享客户端id List<AgentInfo>
     */
    List<String> querySharedAgentIds();

    /**
     * 查询指定插件类型的共享agent
     *
     * @param pluginType 插件类型
     * @return 分页的客户端信息 PageListResponse<AgentInfo>
     */
    PageListResponse<AgentInfo> querySharedAgents(String pluginType);
}
