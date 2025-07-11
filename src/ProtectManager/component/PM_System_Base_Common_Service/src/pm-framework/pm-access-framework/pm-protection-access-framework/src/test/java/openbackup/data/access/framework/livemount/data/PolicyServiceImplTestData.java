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
package openbackup.data.access.framework.livemount.data;

import openbackup.data.access.framework.livemount.common.enums.CopyDataSelection;
import openbackup.data.access.framework.livemount.common.enums.RetentionType;
import openbackup.data.access.framework.livemount.common.enums.RetentionUnit;
import openbackup.data.access.framework.livemount.common.enums.ScheduledType;
import openbackup.data.access.framework.livemount.common.enums.ScheduledUnit;
import openbackup.data.access.framework.livemount.controller.policy.request.CreatePolicyRequest;
import openbackup.data.access.framework.livemount.controller.policy.request.UpdatePolicyRequest;
import openbackup.data.access.framework.livemount.entity.LiveMountPolicyEntity;

import java.sql.Timestamp;

/**
 * live mount policy service test
 *
 */
public class PolicyServiceImplTestData extends LiveMountCommonTestData {
    private static final String POLICY_NAME = "policy_name_1";

    private static final String POLICY_ID = "policy_id_1";

    /**
     * get live mount entity
     *
     * @return live mount entity
     */
    public static CreatePolicyRequest getCreateRequest() {
        CreatePolicyRequest createPolicyRequest = new CreatePolicyRequest();
        createPolicyRequest.setName(POLICY_NAME);
        createPolicyRequest.setCopyDataSelectionPolicy(CopyDataSelection.LATEST);
        createPolicyRequest.setRetentionPolicy(RetentionType.LATEST_ONE);
        createPolicyRequest.setSchedulePolicy(ScheduledType.AFTER_BACKUP_DONE);
        return createPolicyRequest;
    }

    /**
     * get LiveMountPolicyEntity
     *
     * @return LiveMountPolicyEntity
     */
    public static LiveMountPolicyEntity getLiveMountPolicyEntity() {
        LiveMountPolicyEntity liveMountPolicyEntity = new LiveMountPolicyEntity();
        liveMountPolicyEntity.setPolicyId(POLICY_ID);
        liveMountPolicyEntity.setName(POLICY_NAME);
        liveMountPolicyEntity.setSchedulePolicy(RetentionType.FIXED_TIME.getName());
        liveMountPolicyEntity.setRetentionPolicy(ScheduledType.PERIOD_SCHEDULE.getName());
        liveMountPolicyEntity.setScheduleStartTime(new Timestamp(111111111));
        liveMountPolicyEntity.setScheduleInterval(22);
        liveMountPolicyEntity.setScheduleIntervalUnit("d");
        return liveMountPolicyEntity;
    }

    /**
     * get updatePolicyRequest
     *
     * @return updatePolicyRequest
     */
    public static UpdatePolicyRequest getUpdatePolicyRequest() {
        UpdatePolicyRequest updatePolicyRequest = new UpdatePolicyRequest();
        updatePolicyRequest.setCopyDataSelectionPolicy(CopyDataSelection.LAST_DAY);
        updatePolicyRequest.setName(POLICY_NAME);
        updatePolicyRequest.setRetentionPolicy(RetentionType.FIXED_TIME);
        updatePolicyRequest.setRetentionUnit(RetentionUnit.DAY);
        updatePolicyRequest.setRetentionValue(22);
        updatePolicyRequest.setScheduleInterval(22);
        updatePolicyRequest.setSchedulePolicy(ScheduledType.PERIOD_SCHEDULE);
        updatePolicyRequest.setScheduleIntervalUnit(ScheduledUnit.DAY);
        updatePolicyRequest.setScheduleStartTime("2021-03-04 15:18:07");
        return updatePolicyRequest;
    }
}
