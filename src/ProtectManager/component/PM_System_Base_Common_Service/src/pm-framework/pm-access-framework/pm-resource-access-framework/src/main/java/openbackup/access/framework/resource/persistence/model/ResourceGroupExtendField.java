package openbackup.access.framework.resource.persistence.model;

import openbackup.system.base.query.PageQueryConfig;

import lombok.Data;

/**
 * 功能描述
 *
 * @author x30058130
 * @version [OceanProtect DataBackup 1.6.0]
 * @since 2024-05-25
 */
@Data
@PageQueryConfig(conditions = {"%sla_name%", "is_sla_compliance"})
public class ResourceGroupExtendField extends ResourceGroupPo {
    /**
     * 任务所在节点名称
     */
    private String slaName;

    /**
     * 任务所在节点名称
     */
    private Boolean isSlaCompliance;
}