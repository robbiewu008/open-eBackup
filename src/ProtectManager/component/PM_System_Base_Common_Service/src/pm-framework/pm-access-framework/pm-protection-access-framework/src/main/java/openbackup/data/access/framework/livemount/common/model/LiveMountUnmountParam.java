package openbackup.data.access.framework.livemount.common.model;

import openbackup.system.base.common.model.livemount.LiveMountEntity;
import openbackup.system.base.sdk.copy.model.Copy;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Data;

/**
 * Live Mount Destroy Param
 *
 * @author l00272247
 * @since 2020-09-22
 */
@Data
public class LiveMountUnmountParam {
    @JsonProperty("request_id")
    private String requestId;

    @JsonProperty("job_id")
    private String jobId;

    @JsonProperty("live_mount")
    private LiveMountEntity liveMount;

    @JsonProperty("mounted_copy")
    private Copy mountedCopy;

    @JsonProperty("reserve_app")
    private Boolean isReserveApp;

    @JsonProperty("reserve_copy")
    private String reserveCopy;

    @JsonProperty("force_delete")
    private String forceDelete;
}
