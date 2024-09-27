package openbackup.openstack.adapter.service;

import openbackup.openstack.adapter.dto.OpenStackQuotaDto;

import lombok.extern.slf4j.Slf4j;

import org.springframework.stereotype.Component;

import java.util.List;

/**
 * OpenStack配额适配器
 *
 * @author w00616953
 * @version [OceanProtect X8000 1.3.0]
 * @since 2023-01-28
 */
@Slf4j
@Component
public class OpenStackQuotaAdapter {
    private final OpenStackQuotaManager quotaManager;

    public OpenStackQuotaAdapter(OpenStackQuotaManager quotaManager) {
        this.quotaManager = quotaManager;
    }

    /**
     * 设置项目配额
     *
     * @param projectId OpenStack项目id
     * @param quota 配额
     */
    public void setQuota(String projectId, OpenStackQuotaDto quota) {
        quotaManager.setQuota(projectId, quota);
    }

    /**
     * 获取项目配额
     *
     * @param projectId OpenStack项目id
     * @return 项目配额
     */
    public List<OpenStackQuotaDto> getQuota(String projectId) {
        return quotaManager.getQuota(projectId);
    }
}
