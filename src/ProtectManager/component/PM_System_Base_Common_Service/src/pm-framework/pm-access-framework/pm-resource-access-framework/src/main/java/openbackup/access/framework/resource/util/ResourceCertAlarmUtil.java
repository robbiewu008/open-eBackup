/*
* This file is a part of the open-eBackup project.
* This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
* If a copy of the MPL was not distributed with this file, You can obtain one at
* http://mozilla.org/MPL/2.0/.
*
* Copyright (c) [2024] Huawei Technologies Co.,Ltd.
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*/
package openbackup.access.framework.resource.util;

import openbackup.system.base.common.constants.FaultEnum;
import openbackup.system.base.common.constants.IsmNumberConstant;
import openbackup.system.base.common.constants.LegoInternalAlarm;

/**
 * 资源证书告警类
 *
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
