package openbackup.system.base.config;

import lombok.RequiredArgsConstructor;
import lombok.extern.slf4j.Slf4j;

import org.springframework.jdbc.datasource.AbstractDataSource;
import org.springframework.retry.annotation.Backoff;
import org.springframework.retry.annotation.Retryable;

import java.sql.Connection;
import java.sql.SQLException;

import javax.sql.DataSource;

/**
 * 数据库获取连接重试装饰器
 *
 * @author mwx776342
 * @since 2022-05-16
 */
@Slf4j
@RequiredArgsConstructor
public class RetryableDataSource extends AbstractDataSource {
    private static final long WAIT_TIME = 3 * 1000L;

    private final DataSource dataSource;

    @Override
    @Retryable(backoff = @Backoff(delay = WAIT_TIME))
    public Connection getConnection() throws SQLException {
        return dataSource.getConnection();
    }

    @Override
    @Retryable(backoff = @Backoff(delay = WAIT_TIME))
    public Connection getConnection(String username, String password) throws SQLException {
        return dataSource.getConnection(username, password);
    }
}
