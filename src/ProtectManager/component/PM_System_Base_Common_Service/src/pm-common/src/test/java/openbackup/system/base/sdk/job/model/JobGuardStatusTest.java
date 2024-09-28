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
package openbackup.system.base.sdk.job.model;

import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.sdk.job.model.JobGuardStatus;

import com.fasterxml.jackson.core.JsonProcessingException;

import org.junit.Assert;
import org.junit.Test;

import java.util.Locale;

/**
 * Job Guard Status Test
 *
 */
public class JobGuardStatusTest {
    @Test
    public void testEnum() throws JsonProcessingException {
        String origin = "\"running\"";
        JobGuardStatus status = JSONObject.DEFAULT_OBJ_MAPPER.readValue(origin, JobGuardStatus.class);
        Assert.assertEquals(JobGuardStatus.RUNNING, status);
        String value = JSONObject.DEFAULT_OBJ_MAPPER.writeValueAsString(status);
        Assert.assertEquals(origin.toUpperCase(Locale.ENGLISH), value);
    }
}
