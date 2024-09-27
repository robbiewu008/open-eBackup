package openbackup.data.access.framework.core.security.journal;

import openbackup.data.access.framework.core.security.Evaluation;
import openbackup.system.base.security.journal.Logging;

import org.springframework.context.ApplicationContext;
import org.springframework.stereotype.Component;

import java.util.Arrays;
import java.util.List;
import java.util.Map;
import java.util.stream.Collectors;

/**
 * Logging Evaluate Service
 *
 * @author l00272247
 * @since 2022-01-07
 */
@Component
public class LoggingEvaluateService implements EvaluateService<Logging> {
    private final ApplicationContext applicationContext;

    /**
     * constructor
     *
     * @param applicationContext application context
     */
    public LoggingEvaluateService(ApplicationContext applicationContext) {
        this.applicationContext = applicationContext;
    }

    /**
     * evaluate before execute
     *
     * @param loggingContext logging context
     * @param logging param
     */
    @Override
    public void evaluateBeforeExecute(LoggingContext loggingContext, Logging logging) {
        Map<String, Object> params = loggingContext.get(LoggingContext.PARAMS);
        List<Evaluation> evaluations =
                Arrays.stream(logging.details())
                        .map(detail -> new Evaluation(applicationContext, detail))
                        .map(evaluation -> Evaluation.evaluate(loggingContext.getArgs(), evaluation, params))
                        .collect(Collectors.toList());
        evaluations.add(
                0,
                Evaluation.evaluate(
                        loggingContext.getArgs(), new Evaluation(applicationContext, logging.target(), true), params));
        loggingContext.set(LoggingContext.EVALUATIONS, evaluations);
    }

    @Override
    public void evaluateAfterExecute(LoggingContext loggingContext) {
        List<Evaluation> evaluations = loggingContext.get(LoggingContext.EVALUATIONS);
        List<Evaluation> results =
                evaluations.stream()
                        .map(evaluation -> evaluateWithParamsInContext(loggingContext, evaluation))
                        .collect(Collectors.toList());
        loggingContext.set(LoggingContext.EVALUATIONS, results);
    }

    private Evaluation evaluateWithParamsInContext(LoggingContext loggingContext, Evaluation evaluation) {
        Map<String, Object> params = loggingContext.get(LoggingContext.PARAMS);
        Object result = evaluation.evaluate(() -> Evaluation.buildParameters(loggingContext.getArgs(), params));
        return new Evaluation(result);
    }
}
