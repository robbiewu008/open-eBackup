package openbackup.system.base.kafka.retry;

import org.springframework.classify.BinaryExceptionClassifier;
import org.springframework.retry.RetryContext;
import org.springframework.retry.RetryPolicy;
import org.springframework.retry.context.RetryContextSupport;
import org.springframework.retry.policy.SimpleRetryPolicy;
import org.springframework.util.ClassUtils;

import java.util.Collections;
import java.util.Map;

/**
 * 自定义异常重试策略
 *
 * @author y00559272
 * @since 2021-04-10
 */
public class ExceptionRetryPolicy implements RetryPolicy {
    /**
     * The default limit to the number of attempts for a new policy.
     */
    public static final int DEFAULT_MAX_ATTEMPTS = 3;

    private volatile int maxAttempts;

    private final BinaryExceptionClassifier retryableClassifier;

    /**
     * Create a {@link SimpleRetryPolicy} with the default number of retry
     * attempts, retrying all exceptions.
     */
    public ExceptionRetryPolicy() {
        this(
                DEFAULT_MAX_ATTEMPTS,
                Collections.singletonMap(Exception.class, true));
    }

    /**
     * Create a {@link SimpleRetryPolicy} with the specified number of retry
     * attempts, retrying all exceptions.
     *
     * @param maxAttempts max attempt times
     */
    public ExceptionRetryPolicy(int maxAttempts) {
        this(maxAttempts, Collections.singletonMap(Exception.class, true));
    }

    /**
     * Create a {@link SimpleRetryPolicy} with the specified number of retry
     * attempts.
     *
     * @param maxAttempts the maximum number of attempts
     * @param retryableExceptions the map of exceptions that are retryable
     */
    public ExceptionRetryPolicy(int maxAttempts, Map<Class<? extends Throwable>, Boolean> retryableExceptions) {
        this(maxAttempts, retryableExceptions, false);
    }

    /**
     * Create a {@link SimpleRetryPolicy} with the specified number of retry
     * attempts. If traverseCauses is true, the exception causes will be traversed until
     * a match is found.
     *
     * @param maxAttempts the maximum number of attempts
     * @param retryableExceptions the map of exceptions that are retryable based on the
     * map value (true/false).
     * @param isTraverseCauses is this clause traversable
     */
    public ExceptionRetryPolicy(
            int maxAttempts, Map<Class<? extends Throwable>, Boolean> retryableExceptions, boolean isTraverseCauses) {
        this(maxAttempts, retryableExceptions, isTraverseCauses, false);
    }

    /**
     * Create a {@link SimpleRetryPolicy} with the specified number of retry
     * attempts. If traverseCauses is true, the exception causes will be traversed until
     * a match is found. The default value indicates whether to retry or not for exceptions
     * (or super classes) are not found in the map.
     *
     * @param maxAttempts the maximum number of attempts
     * @param retryableExceptions the map of exceptions that are retryable based on the
     * map value (true/false).
     * @param isTraverseCauses is this clause traversable
     * @param hasDefaultValue the default action.
     */
    public ExceptionRetryPolicy(
            int maxAttempts,
            Map<Class<? extends Throwable>, Boolean> retryableExceptions,
            boolean isTraverseCauses,
            boolean hasDefaultValue) {
        super();
        this.maxAttempts = maxAttempts;
        this.retryableClassifier = new BinaryExceptionClassifier(retryableExceptions, hasDefaultValue);
        this.retryableClassifier.setTraverseCauses(isTraverseCauses);
    }

    /**
     * Set the number of attempts before retries are exhausted. Includes the initial
     * attempt before the retries begin so, generally, will be {@code >= 1}. For example
     * setting this property to 3 means 3 attempts total (initial + 2 retries).
     *
     * @param maxAttempts the maximum number of attempts including the initial attempt.
     */
    public void setMaxAttempts(int maxAttempts) {
        this.maxAttempts = maxAttempts;
    }

    /**
     * The maximum number of attempts before failure.
     *
     * @return the maximum number of attempts
     */
    public int getMaxAttempts() {
        return this.maxAttempts;
    }

    /**
     * Test for retryable operation based on the status.
     *
     * @param context RetryContext
     * @see RetryPolicy#canRetry(RetryContext)
     *
     * @return true if the last exception was retryable and the number of
     * attempts so far is less than the limit.
     */
    @Override
    public boolean canRetry(RetryContext context) {
        Throwable throwable = context.getLastThrowable();
        return (throwable == null || throwable.getCause() == null || retryForException(throwable.getCause()))
                && context.getRetryCount() < maxAttempts;
    }

    /**
     * close retry
     *
     * @param status close status
     * @see RetryPolicy#close(RetryContext)
     */
    @Override
    public void close(RetryContext status) {}

    /**
     * Update the status with another attempted retry and the latest exception.
     *
     * @param context  retry context
     * @param throwable exception
     * @see RetryPolicy#registerThrowable(RetryContext, Throwable)
     */
    @Override
    public void registerThrowable(RetryContext context, Throwable throwable) {
        if (context instanceof SimpleRetryContext) {
            SimpleRetryContext simpleContext = ((SimpleRetryContext) context);
            simpleContext.registerThrowable(throwable);
        }
    }

    /**
     * Get a status object that can be used to track the current operation
     * according to this policy. Has to be aware of the latest exception and the
     * number of attempts.
     *
     * @param parent parent retryContext
     * @see RetryPolicy#open(RetryContext)
     * @return RetryContext
     */
    @Override
    public RetryContext open(RetryContext parent) {
        return new SimpleRetryContext(parent);
    }

    private static class SimpleRetryContext extends RetryContextSupport {
        public SimpleRetryContext(RetryContext parent) {
            super(parent);
        }
    }

    /**
     * Delegates to an exception classifier.
     *
     * @param ex exception
     * @return true if this exception or its ancestors have been registered as
     * retryable.
     */
    private boolean retryForException(Throwable ex) {
        return retryableClassifier.classify(ex);
    }

    @Override
    public String toString() {
        return ClassUtils.getShortName(getClass()) + "[maxAttempts=" + maxAttempts + "]";
    }
}
