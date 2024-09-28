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
package openbackup.system.base.util;

import openbackup.system.base.sdk.accesspoint.model.StopPlanBo;
import openbackup.system.base.sdk.job.model.JobTypeEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.util.BeanTools;

import org.junit.jupiter.api.Assertions;
import org.junit.jupiter.api.Test;

/**
 * Bean Tools Test
 *
 */
public class BeanToolsTest {
    @Test
    public void test_hold() {
        StopPlanBo stopPlanBo = new StopPlanBo();
        stopPlanBo.setSourceSubType(ResourceSubTypeEnum.AB_BACKUP_AGENT.getType());
        stopPlanBo.setType(JobTypeEnum.ARCHIVE);
        BeanTools.hold(
                () -> {
                    stopPlanBo.setType(null);
                    stopPlanBo.setSourceSubType(null);
                },
                stopPlanBo,
                new String[] {"type"});
        Assertions.assertEquals(JobTypeEnum.ARCHIVE, stopPlanBo.getType());
        Assertions.assertNull(stopPlanBo.getSourceSubType());

        try {
            BeanTools.hold(
                    () -> {
                        stopPlanBo.setType(null);
                        stopPlanBo.setSourceSubType(ResourceSubTypeEnum.AB_BACKUP_AGENT.getType());
                        throw new IllegalArgumentException();
                    },
                    stopPlanBo,
                    new String[] {"type", "sourceSubType"});
        } catch (IllegalArgumentException ignore) {
            // no-op
        }
        Assertions.assertEquals(JobTypeEnum.ARCHIVE, stopPlanBo.getType());
        Assertions.assertNull(stopPlanBo.getSourceSubType());
    }

    @Test
    public void test_copy() {
        StopPlanBo stopPlanBo = new StopPlanBo();
        stopPlanBo.setSourceSubType(ResourceSubTypeEnum.AB_BACKUP_AGENT.getType());
        stopPlanBo.setType(JobTypeEnum.ARCHIVE);
        StopPlanBo result1 = BeanTools.copy(stopPlanBo, StopPlanBo::new);
        Assertions.assertEquals(ResourceSubTypeEnum.AB_BACKUP_AGENT.getType(), result1.getSourceSubType());
        Assertions.assertEquals(JobTypeEnum.ARCHIVE, result1.getType());
        Assertions.assertNull(result1.getAssociativeId());
        Assertions.assertNull(result1.getRequestId());

        StopPlanBo result2 = BeanTools.copy(stopPlanBo, StopPlanBo::new, false, new String[] {"type"});
        Assertions.assertEquals(ResourceSubTypeEnum.AB_BACKUP_AGENT.getType(), result2.getSourceSubType());
        Assertions.assertNull(result2.getType());
        Assertions.assertNull(result2.getAssociativeId());
        Assertions.assertNull(result2.getRequestId());
    }

    @Test
    public void test_clone() {
        Assertions.assertNull(BeanTools.clone(null));
        Data object = new Data(0);
        IllegalArgumentException exception =
                Assertions.assertThrows(IllegalArgumentException.class, () -> BeanTools.clone(object));
        Assertions.assertEquals("create instance failed", exception.getMessage());
    }

    static class Data {
        private int value;

        public Data(int value) {
            this.value = value;
        }

        public int getValue() {
            return value;
        }

        public void setValue(int value) {
            this.value = value;
        }
    }
}
