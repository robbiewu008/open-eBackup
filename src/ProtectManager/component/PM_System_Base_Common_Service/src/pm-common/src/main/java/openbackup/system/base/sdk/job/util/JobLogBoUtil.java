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
package openbackup.system.base.sdk.job.util;

import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.ExceptionUtil;
import openbackup.system.base.sdk.job.model.JobLogBo;

import java.util.Arrays;
import java.util.Collections;

/**
 * Job Log Bo Util
 *
 */
public class JobLogBoUtil {
    /**
     * init job log detail
     *
     * @param legoCheckedException lego checked exception
     * @param jobLogBo job log bo
     */
    public static void initJobLogDetail(JobLogBo jobLogBo, LegoCheckedException legoCheckedException) {
        if (legoCheckedException != null) {
            jobLogBo.setLogDetail("" + legoCheckedException.getErrorCode());
            String[] parameters = legoCheckedException.getParameters();
            if (parameters != null) {
                jobLogBo.setLogDetailParam(Arrays.asList(parameters));
            } else {
                jobLogBo.setLogDetailParam(Collections.emptyList());
            }
        }
    }

    /**
     * init job log detail
     *
     * @param throwable throwable
     * @param jobLogBo job log bo
     */
    public static void initJobLogDetail(JobLogBo jobLogBo, Throwable throwable) {
        LegoCheckedException legoCheckedException = ExceptionUtil.lookFor(throwable, LegoCheckedException.class);
        initJobLogDetail(jobLogBo, legoCheckedException);
    }
}
