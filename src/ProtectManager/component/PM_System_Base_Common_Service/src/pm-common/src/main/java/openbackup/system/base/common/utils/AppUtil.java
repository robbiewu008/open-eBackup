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
package openbackup.system.base.common.utils;

import openbackup.system.base.common.exception.EmeiStorDefaultExceptionHandler;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.util.ArrayList;
import java.util.List;
import java.util.Locale;
import java.util.concurrent.locks.Condition;
import java.util.concurrent.locks.ReentrantLock;
import java.util.function.Consumer;

/**
 * app util
 *
 * @author l00272247
 * @since 2019-11-06
 */
public class AppUtil {
    private static final Logger LOGGER = LoggerFactory.getLogger(AppUtil.class);

    private static final ReentrantLock LOCK = new ReentrantLock();

    private static final Condition STOP = LOCK.newCondition();

    /**
     * factory
     *
     * @param <T> template type
     */
    public interface Factory<T> {
        /**
         * invoke method
         *
         * @return result
         * @throws Exception exception
         */
        T invoke() throws Exception;
    }

    /**
     * closure interface
     */
    public interface Closure {
        /**
         * execute method
         *
         * @throws Exception exception
         */
        void invoke() throws Exception;
    }

    /**
     * execute all factory
     *
     * @param factory factory
     * @param exceptionHandlers exception handlers
     * @param <Type> template type
     * @return result
     */
    public static <Type> Type execute(Factory<Type> factory, Consumer<Exception>[] exceptionHandlers) {
        try {
            return factory.invoke();
        } catch (Exception e) {
            if (exceptionHandlers == null || exceptionHandlers.length == 0) {
                LOGGER.error("execute error", ExceptionUtil.getErrorMessage(e));
            } else {
                for (Consumer<Exception> exceptionHandler : exceptionHandlers) {
                    if (exceptionHandler != null) {
                        exceptionHandler.accept(e);
                    }
                }
            }
            return null;
        }
    }

    /**
     * batch execute factory
     *
     * @param factories factories
     * @param exceptionHandlers exception handlers
     * @param <T> template type
     * @return result
     */
    public static <T> List<T> execute(Factory<T>[] factories, Consumer<Exception>[] exceptionHandlers) {
        List<T> result = new ArrayList<>();
        for (Factory<T> factory : factories) {
            result.add(execute(factory, exceptionHandlers));
        }
        return result;
    }

    /**
     * execute all closure
     *
     * @param closure closure
     * @param exceptionHandlers exception handlers
     */
    public static void execute(Closure closure, Consumer<Exception>[] exceptionHandlers) {
        execute(() -> {
            closure.invoke();
            return null;
        }, exceptionHandlers);
    }

    /**
     * execute all closures
     *
     * @param closures closures
     * @param exceptionHandlers exception handlers
     */
    public static void execute(Closure[] closures, Consumer<Exception>[] exceptionHandlers) {
        if (closures == null) {
            return;
        }
        for (Closure closure : closures) {
            if (closure != null) {
                execute(closure, exceptionHandlers);
            }
        }
    }

    /**
     * await method
     *
     * @param closures closures
     */
    public static void await(Closure... closures) {
        Runtime.getRuntime().addShutdownHook(new Thread(() -> {
            execute(closures, new Consumer[]{exception -> LOGGER.error("stop error")});
            LOGGER.info("system exit, all service stopped.");
            LOCK.lock();
            try {
                STOP.signal();
            } finally {
                LOCK.unlock();
            }
        }, "system-shutdown-hook"));
        LOCK.lock();
        try {
            STOP.await();
            LOGGER.info("service is stopped.");
        } catch (InterruptedException e) {
            LOGGER.error("service is stopped, interrupted by other thread.", ExceptionUtil.getErrorMessage(e));
        } finally {
            LOCK.unlock();
        }
    }

    /**
     * fatal method
     *
     * @param throwable throwable
     * @param code exit code
     * @param template message template
     * @param args message arguments
     * @return jvm was already shutdown, the exception will not return
     */
    public static EmeiStorDefaultExceptionHandler fatal(Throwable throwable, int code, String template, Object[] args) {
        String message = template != null ? String.format(Locale.ROOT, template, args) : "unknown error";
        if (throwable != null || code != 0) {
            LOGGER.error(message);
        } else {
            LOGGER.info(message);
        }
        System.exit(code);
        throw new EmeiStorDefaultExceptionHandler(message);
    }
}
