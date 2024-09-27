/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

package openbackup.system.base.common.aspect;

import com.google.common.collect.Sets;

import org.apache.ibatis.executor.statement.StatementHandler;
import org.apache.ibatis.plugin.Interceptor;
import org.apache.ibatis.plugin.Intercepts;
import org.apache.ibatis.plugin.Invocation;
import org.apache.ibatis.plugin.Signature;
import org.postgresql.util.PSQLState;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.stereotype.Component;

import java.lang.reflect.InvocationTargetException;
import java.sql.Connection;
import java.sql.SQLException;
import java.util.Set;

/**
 * 自定义Mybatis拦截器，做数据库重试
 *
 * @author mwx776342
 * @since 2022/05/11
 */
@Component
@Intercepts({
    @Signature(type = StatementHandler.class, method = "prepare", args = {Connection.class, Integer.class})
})
public class DatabaseRetryInterceptor implements Interceptor {
    private static final Logger LOGGER = LoggerFactory.getLogger(DatabaseRetryInterceptor.class);

    private static final Set<String> needReTreyStates = Sets.newHashSet(
        PSQLState.CONNECTION_UNABLE_TO_CONNECT.getState(), PSQLState.CONNECTION_FAILURE.getState(),
        PSQLState.CONNECTION_FAILURE_DURING_TRANSACTION.getState(), PSQLState.COMMUNICATION_ERROR.getState());

    @Override
    public Object intercept(Invocation invocation) throws Throwable {
        int index = 0;
        while (index++ < 2) {
            try {
                return invocation.proceed();
            } catch (InvocationTargetException e) {
                // 判断是否是需要重试的异常
                if (!(e.getTargetException() instanceof SQLException)) {
                    throw e;
                }
                SQLException sqlException = (SQLException) e.getTargetException();
                String sqlState = sqlException.getSQLState();
                if (!needReTreyStates.contains(sqlState)) {
                    throw e;
                }
                LOGGER.error("An exception that needs to be retried was caught, times :{}.", index);
            }
        }
        return invocation.proceed();
    }
}
