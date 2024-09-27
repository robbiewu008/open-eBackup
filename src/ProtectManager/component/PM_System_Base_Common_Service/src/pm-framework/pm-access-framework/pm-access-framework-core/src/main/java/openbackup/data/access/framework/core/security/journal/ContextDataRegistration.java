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
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.security.context.Context;

import org.springframework.context.ApplicationContext;

import java.util.Arrays;
import java.util.Collection;
import java.util.HashSet;
import java.util.List;
import java.util.Locale;
import java.util.Map;
import java.util.Objects;
import java.util.Set;
import java.util.stream.Collectors;
import java.util.stream.Stream;

/**
 * Definition
 *
 * @author l00272247
 * @since 2021-12-14
 */
public class ContextDataRegistration implements Comparable<ContextDataRegistration> {
    private final ApplicationContext applicationContext;
    private final Context context;
    private final List<ContextDataRegistration> registrations;
    private final String name;
    private final Evaluation evaluation;
    private final List<String> variables;

    /**
     * constructor
     *
     * @param applicationContext context
     * @param context annotation
     * @param registrations registrations
     */
    public ContextDataRegistration(
            ApplicationContext applicationContext, Context context, List<ContextDataRegistration> registrations) {
        String text = context.name();
        if (text.isEmpty()) {
            throw new LegoCheckedException("name can not be empty");
        }
        if (Arrays.asList("return", "request", "session").contains(text)) {
            throw new LegoCheckedException(String.format(Locale.ENGLISH, "name can not be '%s'", text));
        }
        this.applicationContext = applicationContext;
        this.context = context;
        this.registrations = Objects.requireNonNull(registrations);
        this.name = "$" + text;
        evaluation = new Evaluation(applicationContext, context.statement());
        variables = Evaluation.getRequiredVariables(evaluation.getExpression());
        if (variables.contains(name)) {
            throw new LegoCheckedException("self-reference is not allowed");
        }
        checkCircleReference(registrations, this);
        registrations.add(this);
    }

    private ContextDataRegistration(
            ApplicationContext applicationContext,
            Context context,
            List<ContextDataRegistration> registrations,
            Evaluation evaluation,
            List<String> variables) {
        this.applicationContext = applicationContext;
        this.context = context;
        this.registrations = registrations;
        this.name = "$" + context.name();
        this.evaluation = evaluation;
        this.variables = variables;
    }

    private static void checkCircleReference(
            List<ContextDataRegistration> registrations, ContextDataRegistration contextDataRegistration) {
        List<String> variableList = contextDataRegistration.variables;
        String contextDataRegistrationName = contextDataRegistration.getName();
        List<ContextDataRegistration> contextDataRegistrations =
                Stream.concat(registrations.stream(), Stream.of(contextDataRegistration)).collect(Collectors.toList());
        Set<String> requiredVariables = new HashSet<>(variableList);
        Set<String> checkingVariables = new HashSet<>(variableList);
        while (true) {
            List<ContextDataRegistration> items = getDefinitions(contextDataRegistrations, checkingVariables);
            Set<String> indirections =
                    items.stream().flatMap(item -> item.variables.stream()).collect(Collectors.toSet());
            indirections.removeAll(requiredVariables);
            if (indirections.contains(contextDataRegistrationName)) {
                throw new LegoCheckedException("circle-reference is not allowed");
            }
            if (indirections.isEmpty()) {
                break;
            } else {
                requiredVariables.addAll(indirections);
                checkingVariables = indirections;
            }
        }
    }

    private static List<ContextDataRegistration> getDefinitions(
            List<ContextDataRegistration> contextDataRegistrations, Collection<String> names) {
        return contextDataRegistrations.stream()
                .filter(item -> names.contains(item.getName()))
                .collect(Collectors.toList());
    }

    public String getName() {
        return name;
    }

    public ApplicationContext getApplicationContext() {
        return applicationContext;
    }

    public Context getContext() {
        return context;
    }

    public Evaluation getEvaluation() {
        return evaluation;
    }

    /**
     * evaluate with join point
     *
     * @param args args
     * @param params params
     * @param <T> template type T
     * @return result definition
     */
    public final <T> ContextDataRegistration evaluate(List<T> args, Map<String, Object> params) {
        return new ContextDataRegistration(
                applicationContext,
                context,
                registrations,
                Evaluation.evaluate(args, evaluation, params, value -> params.put(name, value)),
                variables);
    }

    /**
     * require detect
     *
     * @param that that
     * @return detect result
     */
    public boolean require(ContextDataRegistration that) {
        Set<String> whole = new HashSet<>(variables);
        Set<String> items = new HashSet<>(variables);
        while (true) {
            if (items.contains(that.getName())) {
                return true;
            }
            List<ContextDataRegistration> contextDataRegistrations = getDefinitions(registrations, items);
            Set<String> requires =
                    contextDataRegistrations.stream()
                            .flatMap(contextDataRegistration -> contextDataRegistration.variables.stream())
                            .collect(Collectors.toSet());
            requires.removeAll(whole);
            if (requires.isEmpty()) {
                return false;
            }
            whole.addAll(requires);
            items = requires;
        }
    }

    @Override
    public int compareTo(ContextDataRegistration that) {
        if (this.require(that)) {
            return 1;
        } else if (that.require(this)) {
            return -1;
        } else {
            return 0;
        }
    }
}
