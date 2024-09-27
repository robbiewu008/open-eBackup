package openbackup.data.protection.access.provider.sdk.agent;

import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.enums.AgentMountTypeEnum;

import java.util.List;

/**
 * agent服务
 *
 * @author h30027154
 * @version [OceanProtect DataBackup 1.5.0]
 * @since 2023/9/15
 */
public interface CommonAgentService {
    /**
     * 填充下发agent通用信息
     *
     * @param endpoints agent信息
     */
    void supplyAgentCommonInfo(List<Endpoint> endpoints);

    /**
     * 默认的获取agent挂载类型方法
     *
     * @param jobId 任务ID
     * @return 挂载方法
     */
    AgentMountTypeEnum getJobAgentMountTypeByJob(String jobId);


    /**
     * 默认的获取agent挂载类型方法
     *
     * @param unitId 存储单元ID
     * @return 挂载方法
     */
    AgentMountTypeEnum getJobAgentMountTypeByUnitId(String unitId);
}
