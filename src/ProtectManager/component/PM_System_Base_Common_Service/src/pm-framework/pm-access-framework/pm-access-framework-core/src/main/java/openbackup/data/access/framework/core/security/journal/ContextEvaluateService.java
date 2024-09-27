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
 * @author l00272247
 * @since 2022-01-07
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
