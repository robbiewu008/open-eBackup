package openbackup.data.access.framework.livemount.controller.policy.request;

import openbackup.data.access.framework.livemount.common.enums.CopyDataSelection;
import openbackup.data.access.framework.livemount.common.enums.RetentionType;
import openbackup.data.access.framework.livemount.common.enums.RetentionUnit;
import openbackup.data.access.framework.livemount.common.enums.ScheduledType;
import openbackup.data.access.framework.livemount.common.enums.ScheduledUnit;

import lombok.Data;

/**
 * 修改更新策略
 *
 * @author h30003246
 * @since 2020-09-17
 */
@Data
public class UpdatePolicyRequest {
    private String name;

    private CopyDataSelection copyDataSelectionPolicy;

    private RetentionType retentionPolicy;

    private Integer retentionValue;

    private RetentionUnit retentionUnit;

    private ScheduledType schedulePolicy;

    private Integer scheduleInterval;

    private ScheduledUnit scheduleIntervalUnit;

    private String scheduleStartTime;
}
