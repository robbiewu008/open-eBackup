package openbackup.data.access.framework.livemount.common.model;

import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.system.base.common.model.livemount.LiveMountEntity;
import openbackup.system.base.sdk.copy.model.Copy;

import com.fasterxml.jackson.databind.PropertyNamingStrategies;
import com.fasterxml.jackson.databind.annotation.JsonNaming;

import lombok.Data;

/**
 * Live Mount Execute Param
 *
 * @author l00272247
 * @since 2020-09-22
 */
@Data
@JsonNaming(PropertyNamingStrategies.SnakeCaseStrategy.class)
public class LiveMountExecuteParam {
    private String requestId;

    private String jobId;

    private LiveMountEntity liveMount;

    private Copy cloneCopy;

    private Copy sourceCopy;

    private Copy mountedCopy;

    private ProtectedResource targetResource;
}
