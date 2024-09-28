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
package openbackup.data.access.framework.core.security.journal;

import openbackup.data.access.framework.core.security.Evaluation;
import openbackup.system.base.security.context.Context;

import org.springframework.context.ApplicationContext;
import org.springframework.stereotype.Component;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.stream.Collectors;

/**
 * Context Evaluate Service
 *
 */
@Component
public class ContextEvaluateService implements EvaluateService<List<Context>> {
    private final ApplicationContext applicationContext;

    /**
     * constructor
     *
     * @param applicationContext context
     */
    public ContextEvaluateService(ApplicationContext applicationContext) {
        this.applicationContext = applicationContext;
    }

    /**
     * evaluate before execute
     *
     * @param loggingContext logging context
     * @param contexts contexts
     */
    @Override
    public void evaluateBeforeExecute(LoggingContext loggingContext, List<Context> contexts) {
        List<?> args = loggingContext.getArgs();
        List<ContextDataRegistration> registrations = new ArrayList<>();
        Map<String, Object> params = new HashMap<>();
        List<ContextDataRegistration> contextDataRegistrations =
                contexts.stream()
                        .map(context -> new ContextDataRegistration(applicationContext, context, registrations))
                        .sorted()
                        .map(contextDataRegistration -> contextDataRegistration.evaluate(args, params))
                        .collect(Collectors.toList());
        loggingContext
                .set(LoggingContext.CONTEXT_REGISTRATIONS, contextDataRegistrations)
                .set(LoggingContext.PARAMS, params);
    }

    /**
     * evaluate after execute
     *
     * @param loggingContext logging context
     */
    @Override
    public void evaluateAfterExecute(LoggingContext loggingContext) {
        Map<String, Object> params = loggingContext.get(LoggingContext.PARAMS);
        List<ContextDataRegistration> registrations = loggingContext.get(LoggingContext.CONTEXT_REGISTRATIONS);
        registrations.stream()
                .filter(
                        contextDataRegistration ->
                                contextDataRegistration.getEvaluation().isDependOnReturnValue())
                .forEach(
                        contextDataRegistration -> {
                            Evaluation evaluation = contextDataRegistration.getEvaluation();
                            Object value =
                                    evaluation.evaluate(
                                            () -> Evaluation.buildParameters(loggingContext.getArgs(), params));
                            params.put(contextDataRegistration.getName(), value);
                        });
    }
}
