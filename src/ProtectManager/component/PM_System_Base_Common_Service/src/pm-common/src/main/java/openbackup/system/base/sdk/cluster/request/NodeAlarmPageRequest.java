package openbackup.system.base.sdk.cluster.request;

import openbackup.system.base.common.enums.AlarmLang;

import lombok.Data;

/**
 * 告警查询条件
 *
 * @author w00607005
 * @since 2023-08-15
 */
@Data
public class NodeAlarmPageRequest {
    private int pageSize;

    private int pageNo;

    private AlarmLang language = AlarmLang.ZH;

    private boolean shouldAllNodes = false;
}
