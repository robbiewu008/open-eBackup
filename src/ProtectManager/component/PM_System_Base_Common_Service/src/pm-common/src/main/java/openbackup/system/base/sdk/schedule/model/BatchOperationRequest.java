package openbackup.system.base.sdk.schedule.model;

import lombok.Data;

import java.util.List;

/**
 * 功能描述 BatchOperationRequest
 *
 * @author m00576658
 * @since 2021-07-23
 */
@Data
public class BatchOperationRequest {
    private List<String> scheduleIds;
}
