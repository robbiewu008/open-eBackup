package openbackup.data.access.framework.livemount.common.model;

import openbackup.system.base.common.model.livemount.LiveMountEntity;
import openbackup.system.base.query.PageQueryConfig;

import lombok.Data;

/**
 * Live Mount Model
 *
 * @author l00272247
 * @since 2020-09-26
 */
@Data
@PageQueryConfig(conditions = {"%policy_name%", "%cluster_name%"})
public class LiveMountModel extends LiveMountEntity {
    private String policyName;

    private String clusterName;
}
