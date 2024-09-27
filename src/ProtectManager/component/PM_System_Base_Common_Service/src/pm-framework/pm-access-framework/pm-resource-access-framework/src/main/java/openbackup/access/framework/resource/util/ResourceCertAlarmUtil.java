package openbackup.access.framework.resource.util;

import openbackup.system.base.common.constants.FaultEnum;
import openbackup.system.base.common.constants.IsmNumberConstant;
import openbackup.system.base.common.constants.LegoInternalAlarm;

/**
 * 资源证书告警类
 *
 * @author fwx1022842
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022/10/14
 */
public class ResourceCertAlarmUtil {
    /**
     * 获取资源证书的告警对象
     *
     * @param alarmId      告警ID
     * @param resourceName 资源名称
     * @param type 类型
     * @param resourceId 资源ID
     * @return 告警对象
     */
    public static LegoInternalAlarm genResourceCertExpiredAlarm(String alarmId, String resourceName,
        String type, String resourceId) {
        LegoInternalAlarm legoInternalAlarm = new LegoInternalAlarm();
        legoInternalAlarm.setAlarmId(alarmId);
        legoInternalAlarm.setMoName("Resource");
        legoInternalAlarm.setAlarmParam(new String[]{type, resourceName});
        legoInternalAlarm.setAlarmSequence(IsmNumberConstant.ONE);
        legoInternalAlarm.setAlarmLevel(FaultEnum.AlarmSeverity.MAJOR);
        legoInternalAlarm.setSourceType(FaultEnum.AlarmResourceType.CERTIFICATE.getValue());
        legoInternalAlarm.setResourceId(resourceId);
        return legoInternalAlarm;
    }
}
