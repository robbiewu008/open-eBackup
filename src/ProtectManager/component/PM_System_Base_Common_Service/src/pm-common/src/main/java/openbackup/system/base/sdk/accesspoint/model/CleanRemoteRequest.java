package openbackup.system.base.sdk.accesspoint.model;

import lombok.Data;

import java.util.List;

/**
 * Clean Remote Request
 *
 * @author l00272247
 * @since 2020-12-16
 */
@Data
public class CleanRemoteRequest {
    private DmeLocalDevice localDevice;

    private List<DmeRemoteDevice> remoteDevice;
}
