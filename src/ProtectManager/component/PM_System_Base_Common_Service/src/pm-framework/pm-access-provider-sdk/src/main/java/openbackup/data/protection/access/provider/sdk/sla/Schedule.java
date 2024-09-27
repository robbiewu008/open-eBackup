package openbackup.data.protection.access.provider.sdk.sla;

import com.fasterxml.jackson.annotation.JsonFormat;
import com.fasterxml.jackson.databind.PropertyNamingStrategy;
import com.fasterxml.jackson.databind.annotation.JsonNaming;

import lombok.Data;

import java.util.Date;

/**
 * Schedule实体类
 *
 * @author y00559272
 * @version [OceanProtect A8000 1.1.0]
 * @since 2021-11-19
 */
@Data
@JsonNaming(PropertyNamingStrategy.SnakeCaseStrategy.class)
public class Schedule {
    /**
     * trigger type
     */
    private Integer trigger;

    /**
     * trigger interval
     */
    private Integer interval;

    /**
     * trigger interval unit
     */
    private String intervalUnit;

    /**
     * trigger start time
     */
    @JsonFormat(pattern = "yyyy-MM-dd'T'HH:mm:ss")
    private Date startTime;

    /**
     * time window start
     */
    private String windowStart;

    /**
     * time window end
     */
    private String windowEnd;
}
