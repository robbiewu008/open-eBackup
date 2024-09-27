package openbackup.system.base.sdk.schedule.model;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Data;

/**
 * Schedule Response
 *
 * @author l00272247
 * @since 2020-09-21
 */
@Data
public class ScheduleResponse {
    @JsonProperty("schedule_id")
    private String scheduleId;
}
