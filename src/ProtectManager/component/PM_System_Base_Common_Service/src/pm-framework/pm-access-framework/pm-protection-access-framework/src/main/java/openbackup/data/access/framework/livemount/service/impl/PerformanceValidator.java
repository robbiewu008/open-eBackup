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
package openbackup.data.access.framework.livemount.service.impl;

import openbackup.data.access.framework.livemount.common.model.LiveMountObject;
import openbackup.system.base.common.constants.ErrorCodeConstant;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.sdk.livemount.model.Performance;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Component;

import java.util.Map;
import java.util.Set;
import java.util.stream.Collectors;

import javax.validation.ConstraintViolation;
import javax.validation.Validator;
import javax.validation.ValidatorFactory;

/**
 * Performance Validator
 *
 * @author l00272247
 * @since 2020-10-15
 */
@Component
public class PerformanceValidator {
    @Autowired
    private ValidatorFactory factory;

    /**
     * get valid performance
     *
     * @param liveMountObject live mount object
     * @return performance
     */
    public Performance getValidPerformance(LiveMountObject liveMountObject) {
        Map<String, Object> parameters = liveMountObject.getParameters();
        if (parameters == null) {
            return null;
        }
        Performance performance = JSONObject.cast(parameters, Performance.class);
        Set<ConstraintViolation<Performance>> violations = checkPerformanceViolations(performance);
        if (!violations.isEmpty()) {
            return null;
        }
        if (performance.validate()) {
            return performance;
        }
        return null;
    }

    /**
     * validate performance
     *
     * @param performance performance
     * @return violations
     */
    public Set<ConstraintViolation<Performance>> checkPerformanceViolations(Performance performance) {
        Validator validator = factory.getValidator();
        return validator.validate(performance);
    }

    /**
     * validate rerformance
     *
     * @param performance performance
     */
    public void validatePerformance(Performance performance) {
        Set<ConstraintViolation<Performance>> violations = checkPerformanceViolations(performance);
        if (!violations.isEmpty()) {
            String message = violations.stream().map(ConstraintViolation::getMessage).collect(Collectors.joining());
            throw new LegoCheckedException(ErrorCodeConstant.ERR_PARAM, message);
        }
        if (!performance.validate()) {
            throw new LegoCheckedException(ErrorCodeConstant.ERR_PARAM, "performance parameters is invalid");
        }
    }

    /**
     * get valid performance
     *
     * @param data data
     * @return performance
     */
    public Performance loadPerformance(Map<String, Object> data) {
        Performance performance = JSONObject.cast(data, Performance.class, true);
        if (performance == null) {
            return null;
        }
        if (JSONObject.fromObject(performance).isEmpty()) {
            return null;
        }
        return performance;
    }
}
