/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
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
 * @author l00272247
 * @since 2022-03-21
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
