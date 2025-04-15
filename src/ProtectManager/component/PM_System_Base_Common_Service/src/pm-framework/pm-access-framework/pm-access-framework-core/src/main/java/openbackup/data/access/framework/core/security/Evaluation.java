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
package openbackup.data.access.framework.core.security;

import lombok.extern.slf4j.Slf4j;
import openbackup.system.base.common.utils.CollectionUtils;
import openbackup.system.base.common.utils.StringUtil;
import openbackup.system.base.security.callee.Callee;

import org.springframework.context.ApplicationContext;
import org.springframework.context.expression.MapAccessor;
import org.springframework.expression.BeanResolver;
import org.springframework.expression.Expression;
import org.springframework.expression.ExpressionParser;
import org.springframework.expression.common.CompositeStringExpression;
import org.springframework.expression.common.TemplateParserContext;
import org.springframework.expression.spel.SpelNode;
import org.springframework.expression.spel.ast.CompoundExpression;
import org.springframework.expression.spel.ast.Elvis;
import org.springframework.expression.spel.ast.FunctionReference;
import org.springframework.expression.spel.ast.MethodReference;
import org.springframework.expression.spel.ast.OpGE;
import org.springframework.expression.spel.ast.OpGT;
import org.springframework.expression.spel.ast.OpInc;
import org.springframework.expression.spel.ast.OpLE;
import org.springframework.expression.spel.ast.OpLT;
import org.springframework.expression.spel.ast.OpMinus;
import org.springframework.expression.spel.ast.OpModulus;
import org.springframework.expression.spel.ast.OpMultiply;
import org.springframework.expression.spel.ast.OpNE;
import org.springframework.expression.spel.ast.OpOr;
import org.springframework.expression.spel.ast.OpPlus;
import org.springframework.expression.spel.ast.Projection;
import org.springframework.expression.spel.ast.PropertyOrFieldReference;
import org.springframework.expression.spel.ast.Selection;
import org.springframework.expression.spel.ast.Ternary;
import org.springframework.expression.spel.standard.SpelExpression;
import org.springframework.expression.spel.standard.SpelExpressionParser;
import org.springframework.expression.spel.support.StandardEvaluationContext;

import java.lang.reflect.Method;
import java.lang.reflect.Modifier;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Objects;
import java.util.function.Consumer;
import java.util.function.Function;
import java.util.function.Supplier;
import java.util.stream.Collectors;
import java.util.stream.Stream;

/**
 * Evaluation
 *
 */
@Slf4j
public class Evaluation {
    private static final String RETURN = "$return";
    private static final List<Class<? extends SpelNode>> RECURSIVE_NODE_TYPES =
            Arrays.asList(
                    MethodReference.class,
                    FunctionReference.class,
                    Projection.class,
                    Selection.class,
                    OpGE.class,
                    OpGT.class,
                    OpInc.class,
                    OpLE.class,
                    OpLT.class,
                    OpMinus.class,
                    OpModulus.class,
                    OpMultiply.class,
                    OpNE.class,
                    OpOr.class,
                    OpPlus.class,
                    Ternary.class,
                    Elvis.class);
    private static final String PREFIX = "=";

    private final boolean isDependOnReturnValue;
    private final Function<Supplier<Map<String, Object>>, Object> function;
    private final boolean isTemplate;
    private final Expression expression;
    private final List<String> variables;

    /**
     * constructor
     *
     * @param applicationContext context
     * @param statement statement
     */
    public Evaluation(ApplicationContext applicationContext, String statement) {
        this(applicationContext, statement, false);
    }

    /**
     * constructor
     *
     * @param applicationContext context
     * @param statement statement
     * @param isTemplate template
     */
    public Evaluation(ApplicationContext applicationContext, String statement, boolean isTemplate) {
        ExpressionParser parser = new SpelExpressionParser();
        if (isTemplate) {
            this.isTemplate = true;
            expression = parser.parseExpression(PREFIX + statement, new TemplateParserContext());
        } else if (statement.trim().startsWith("=")) {
            this.isTemplate = true;
            expression = parser.parseExpression(statement.trim(), new TemplateParserContext());
        } else {
            this.isTemplate = false;
            expression = parser.parseExpression(statement);
        }
        variables = Collections.unmodifiableList(getRequiredVariables(expression));
        isDependOnReturnValue = variables.contains(RETURN);
        StandardEvaluationContext standardEvaluationContext = new StandardEvaluationContext();
        standardEvaluationContext.addPropertyAccessor(new MapAccessor());
        registerFunctions(standardEvaluationContext);
        BeanResolver beanResolver = (evaluationContext, name) -> applicationContext.getBean(name, Callee.class);
        standardEvaluationContext.setBeanResolver(beanResolver);
        this.function = args -> expression.getValue(standardEvaluationContext, args.get());
    }

    /**
     * constructor
     *
     * @param statement statement
     */
    public Evaluation(Object statement) {
        this.isDependOnReturnValue = false;
        this.function = args -> statement;
        this.isTemplate = false;
        expression = null;
        variables = Collections.emptyList();
    }

    /**
     * get required variables
     *
     * @param expression expression
     * @return required variables
     */
    public static List<String> getRequiredVariables(Expression expression) {
        if (expression instanceof SpelExpression) {
            SpelNode node = ((SpelExpression) expression).getAST();
            return getRequiredVariables(node);
        } else if (expression instanceof CompositeStringExpression) {
            return Stream.of(((CompositeStringExpression) expression).getExpressions())
                    .filter(SpelExpression.class::isInstance)
                    .map(SpelExpression.class::cast)
                    .map(Evaluation::getRequiredVariables)
                    .flatMap(List::stream)
                    .distinct()
                    .collect(Collectors.toList());
        } else {
            return Collections.emptyList();
        }
    }

    /**
     * get required variables
     *
     * @param node node
     * @return required variables
     */
    private static List<String> getRequiredVariables(SpelNode node) {
        List<String> requires = new ArrayList<>();
        if (node instanceof CompoundExpression) {
            requires.addAll(getRequiredVariables(node.getChild(0)));
            for (int index = 1, limit = node.getChildCount(); index < limit; index++) {
                SpelNode child = node.getChild(index);
                if (isRecursiveNode(child)) {
                    requires.addAll(getRequiredVariables(child));
                }
            }
        } else if (isRecursiveNode(node)) {
            for (int index = 0, limit = node.getChildCount(); index < limit; index++) {
                SpelNode child = node.getChild(index);
                requires.addAll(getRequiredVariables(child));
            }
        } else if (node instanceof PropertyOrFieldReference) {
            String ast = node.toStringAST();
            boolean isRequired = ast.startsWith("$");
            if (isRequired) {
                requires.add(ast.split("\\.")[0]);
            }
        } else {
            requires.addAll(Collections.emptyList());
        }
        return requires;
    }

    private static boolean isRecursiveNode(SpelNode node) {
        return RECURSIVE_NODE_TYPES.stream().anyMatch(type -> type.isInstance(node));
    }

    /**
     * cast array as map
     *
     * @param array array
     * @param <T> template type T
     * @return map
     */
    public static <T> Map<String, Object> castArrayToMap(List<T> array) {
        Map<String, Object> parameters = new HashMap<>();
        parameters.put("$0", array);
        for (int index = 0; index < array.size(); index++) {
            parameters.put("$" + (index + 1), array.get(index));
        }
        return parameters;
    }

    /**
     * build parameters
     *
     * @param args args
     * @param params params
     * @param <T> template type T
     * @return result
     */
    public static <T> Map<String, Object> buildParameters(List<T> args, Map<String, Object> params) {
        Map<String, Object> map = castArrayToMap(args);
        if (params != null) {
            map.putAll(params);
        }
        return map;
    }

    /**
     * evaluate
     *
     * @param args args
     * @param evaluation evaluation
     * @param params params
     * @param consumers consumers
     * @param <T> template type T
     * @return result evaluation
     */
    @SafeVarargs
    public static <T> Evaluation evaluate(
            List<T> args, Evaluation evaluation, Map<String, Object> params, Consumer<Object>... consumers) {
        if (evaluation.isDependOnReturnValue()) {
            return evaluation;
        }
        Object result = evaluation.evaluate(() -> buildParameters(args, params));
        for (Consumer<Object> consumer : consumers) {
            consumer.accept(result);
        }
        return new Evaluation(result);
    }

    private void registerFunctions(StandardEvaluationContext context) {
        registerFunction(context, "list", CollectionUtils.class, "listify", new Class[] {Object.class});
        registerFunction(context, "any", CollectionUtils.class, "any", new Class[] {Object.class});
        registerFunction(context, "one", CollectionUtils.class, "one", new Class[] {Object.class});
        registerFunction(
                context, "join", CollectionUtils.class, "join", new Class[] {Object.class, CharSequence.class});
        registerFunction(context, "snake", StringUtil.class, "snake", new Class[] {String.class});
        registerFunction(
                context,
                "replace",
                StringUtil.class,
                "replace",
                new Class[] {String.class, String.class, String[].class});
        registerFunction(context, "lower", StringUtil.class, "lower", new Class[] {String.class});
        registerFunction(context, "format", StringUtil.class, "format", new Class[] {String.class, Object[].class});
        registerFunction(context, "first", CollectionUtils.class, "first", new Class[] {Object.class});
        registerFunction(context, "char", StringUtil.class, "toChar", new Class[] {int.class});
    }

    private void registerFunction(
            StandardEvaluationContext standardEvaluationContext,
            String functionName,
            Class<?> clazz,
            String methodName,
            Class<?>[] types) {
        Method method;
        try {
            method = clazz.getMethod(methodName, types);
        } catch (NoSuchMethodException e) {
            log.error("not found method: {}", methodName, e);
            return;
        }
        int modifiers = method.getModifiers();
        if (Modifier.isStatic(modifiers) && Modifier.isPublic(modifiers)) {
            standardEvaluationContext.registerFunction(functionName, method);
        } else {
            log.error("method: {} should be a static public method", methodName);
        }
    }

    public List<String> getVariables() {
        return variables;
    }

    /**
     * evaluate method
     *
     * @param args args
     * @return result
     */
    public Object evaluate(Supplier<Map<String, Object>> args) {
        Object result = function.apply(Objects.requireNonNull(args));
        if (isTemplate) {
            return result != null ? result.toString().substring(PREFIX.length()) : null;
        } else {
            return result;
        }
    }

    public boolean isTemplate() {
        return isTemplate;
    }

    public Expression getExpression() {
        return expression;
    }

    public boolean isDependOnReturnValue() {
        return isDependOnReturnValue;
    }
}
