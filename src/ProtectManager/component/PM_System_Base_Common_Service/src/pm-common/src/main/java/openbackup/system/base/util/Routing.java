/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 */

package openbackup.system.base.util;

import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.constants.IsmNumberConstant;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.ExceptionUtil;

import feign.RetryableException;
import lombok.extern.slf4j.Slf4j;

import java.net.ConnectException;
import java.net.MalformedURLException;
import java.net.NoRouteToHostException;
import java.net.SocketTimeoutException;
import java.net.URI;
import java.net.URISyntaxException;
import java.net.URL;
import java.util.AbstractMap;
import java.util.Arrays;
import java.util.List;
import java.util.Map;
import java.util.Objects;
import java.util.concurrent.CompletionService;
import java.util.concurrent.ExecutionException;
import java.util.concurrent.ExecutorCompletionService;
import java.util.concurrent.Future;
import java.util.concurrent.SynchronousQueue;
import java.util.concurrent.ThreadPoolExecutor;
import java.util.concurrent.TimeUnit;
import java.util.function.Consumer;
import java.util.function.Function;
import java.util.stream.Collectors;

import javax.net.ssl.SSLHandshakeException;

/**
 * Routing
 *
 * @author l00272247
 * @since 2020-12-18
 */
@Slf4j
public class Routing {
    private static final ThreadPoolExecutor POOL = new ThreadPoolExecutor(IsmNumberConstant.TWO,
        IsmNumberConstant.SIXTTEEN, IsmNumberConstant.SIXTY, TimeUnit.SECONDS, new SynchronousQueue<>());

    private final List<URI> targets;

    /**
     * constructor
     *
     * @param hosts hosts
     * @param port port
     */
    public Routing(List<String> hosts, int port) {
        this("https", hosts, port);
    }

    /**
     * constructor
     *
     * @param protocol protocol
     * @param hosts hosts
     * @param port port
     */
    public Routing(String protocol, List<String> hosts, int port) {
        this(hosts.stream().map(host -> buildUri(protocol, host, port)).collect(Collectors.toList()));
    }

    /**
     * constructor
     *
     * @param targets targets
     */
    public Routing(List<URI> targets) {
        if (Objects.requireNonNull(targets).isEmpty()) {
            throw new LegoCheckedException("targets can not be empty");
        }
        this.targets = targets;
    }

    /**
     * build uri
     *
     * @param protocol protocol
     * @param host host
     * @param port port
     * @return uri
     */
    private static URI buildUri(String protocol, String host, int port) {
        URI uri;
        try {
            uri = new URL(protocol, host, port, "/").toURI();
        } catch (MalformedURLException | URISyntaxException e) {
            throw new LegoCheckedException("build uri failed", e);
        }
        return uri;
    }

    /**
     * 判断异常是否属于可重试异常
     *
     * @param exception 异常
     * @return 可重试: true; 不可重试: false
     */
    protected boolean isRetryableException(Throwable exception) {
        // RetryableException有可能由于证书异常导致，首先排除
        if (ExceptionUtil.lookFor(exception, SSLHandshakeException.class) != null) {
            return false;
        }
        List<Class<? extends Throwable>> types = Arrays.asList(ConnectException.class, NoRouteToHostException.class,
            SocketTimeoutException.class, RetryableException.class);
        for (Class<? extends Throwable> type : types) {
            Throwable throwable = ExceptionUtil.lookFor(exception, type);
            if (throwable != null) {
                return true;
            }
        }
        return false;
    }

    /**
     * parallelism get
     *
     * @param function function
     * @param <T> template type
     * @return result
     */
    public <T> T get(Function<URI, T> function) {
        CompletionService<T> completion = new ExecutorCompletionService<>(POOL);
        List<Map.Entry<URI, Future<T>>> futures = targets.stream()
            .map(target -> invoke(completion, target, function))
            .collect(Collectors.toList());
        for (int index = 0, limit = futures.size(); index < limit; index++) {
            Future<T> future;
            try {
                future = completion.take();
            } catch (InterruptedException e) {
                throw new LegoCheckedException("future is interrupted.", e);
            }
            T result;
            try {
                result = future.get();
            } catch (InterruptedException | ExecutionException e) {
                URI target = getCorrespondingUriOfFuture(futures, future);
                log.error("Error Encountered on {} ", target, ExceptionUtil.getErrorMessage(e));
                continue;
            }
            cancelFuture(excludeCorrespondingUriOfFuture(futures, future));
            optimizeOrderOfTargets(futures, future);
            URI target = getCorrespondingUriOfFuture(futures, future);
            log.info("request: {} success.", target);
            return result;
        }
        throw new LegoCheckedException("No available communication object.");
    }

    private <T> void optimizeOrderOfTargets(List<Map.Entry<URI, Future<T>>> futures, Future<T> future) {
        optimizeOrderOfTargets(getCorrespondingUriOfFuture(futures, future));
    }

    private void optimizeOrderOfTargets(URI target) {
        int index = targets.indexOf(target);
        if (index > 0) {
            targets.remove(target);
            targets.add(0, target);
        }
    }

    private <T> List<Map.Entry<URI, Future<T>>> excludeCorrespondingUriOfFuture(List<Map.Entry<URI, Future<T>>> futures,
        Future<T> future) {
        return futures.stream().filter(item -> !future.equals(item.getValue())).collect(Collectors.toList());
    }

    private <T> URI getCorrespondingUriOfFuture(List<Map.Entry<URI, Future<T>>> futures, Future<T> future) {
        return futures.stream()
            .filter(item -> future.equals(item.getValue()))
            .findAny()
            .map(Map.Entry::getKey)
            .orElse(null);
    }

    private <T> AbstractMap.SimpleEntry<URI, Future<T>> invoke(CompletionService<T> completion, URI uri,
        Function<URI, T> function) {
        return new AbstractMap.SimpleEntry<>(uri, completion.submit(() -> function.apply(uri)));
    }

    private <T> void cancelFuture(List<Map.Entry<URI, Future<T>>> entries) {
        for (Map.Entry<URI, Future<T>> entry : entries) {
            try {
                Future<T> future = entry.getValue();
                if (!future.isDone()) {
                    future.cancel(true);
                }
            } catch (Throwable e) {
                log.error("cancel other future failed. uri: {}", entry.getKey(), ExceptionUtil.getErrorMessage(e));
            }
        }
    }

    /**
     * run function with any cluster
     *
     * @param function function
     * @param <T> template
     * @return result
     */
    public <T> T call(Function<URI, T> function) {
        for (URI target : targets) {
            try {
                T result = function.apply(target);
                optimizeOrderOfTargets(target);
                return result;
            } catch (RuntimeException e) {
                log.error("Fail to call remote api, uri: {}", target, ExceptionUtil.getErrorMessage(e));
                if (!isRetryableException(e)) {
                    dealException(e);
                }
            }
        }
        throw new LegoCheckedException(CommonErrorCode.NETWORK_CONNECTION_TIMEOUT, "No available communicate object.");
    }

    /**
     * deal with consumer
     *
     * @param consumer consumer
     */
    public void call(Consumer<URI> consumer) {
        call(uri -> {
            consumer.accept(uri);
            return null;
        });
    }

    /**
     * 针对不可重试异常的默认处理逻辑
     *
     * @param exception 不可重试的异常
     */
    protected void dealException(RuntimeException exception) {
        log.error("Exception is nor-retryable, finish retry.", ExceptionUtil.getErrorMessage(exception));
        throw LegoCheckedException.cast(exception, CommonErrorCode.NETWORK_CONNECTION_TIMEOUT);
    }
}
