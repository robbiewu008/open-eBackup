package openbackup.system.base.sdk.alarm.model;

import lombok.AllArgsConstructor;
import lombok.Data;
import lombok.NoArgsConstructor;

/**
 * 功能描述
 *
 * @author w00448845
 * @version [BCManager 8.0.0]
 * @since 2020-01-14
 */
@Data
@AllArgsConstructor
@NoArgsConstructor
public class AlarmStatisticResponse {
    private Integer warning = 0;

    private Integer minor = 0;

    private Integer major = 0;

    private Integer critical = 0;
}
