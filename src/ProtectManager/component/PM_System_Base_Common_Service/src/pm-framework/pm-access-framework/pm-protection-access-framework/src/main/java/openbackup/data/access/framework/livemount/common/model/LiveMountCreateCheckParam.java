package openbackup.data.access.framework.livemount.common.model;

import openbackup.data.access.framework.livemount.common.enums.OperationEnums;
import openbackup.system.base.sdk.copy.model.Copy;
import openbackup.system.base.sdk.copy.model.CopyResourceSummary;
import openbackup.system.base.sdk.resource.model.ResourceEntity;

import lombok.Data;

import java.util.List;

/**
 * Live Mount Create Check Param
 *
 * @author l00272247
 * @since 2020-10-16
 */
@Data
public class LiveMountCreateCheckParam {
    private LiveMountObject liveMountObject;

    private CopyResourceSummary resource;

    private List<ResourceEntity> targetResources;

    private Copy copy;

    private OperationEnums operationEnums;
}
