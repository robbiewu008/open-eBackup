/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

package openbackup.system.base.config;

import com.zaxxer.hikari.HikariDataSource;

import openbackup.system.base.config.DataSourceConfig;

import org.junit.jupiter.api.Assertions;
import org.junit.jupiter.params.ParameterizedTest;
import org.junit.jupiter.params.provider.Arguments;
import org.junit.jupiter.params.provider.MethodSource;

import java.lang.reflect.Field;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.util.stream.Stream;

/**
 * Data Source Config Test
 *
 * @author l00650874
 * @since 2022-05-05
 */
public class DataSourceConfigTest {
    /**
     * revise case data
     *
     * @return revise case data
     */
    public static Stream<Arguments> revise_cases() {
        return Stream.of(
                Arguments.of(9, 10, 20, 10),
                Arguments.of(10, 10, 20, 10),
                Arguments.of(11, 10, 20, 11),
                Arguments.of(19, 10, 20, 19),
                Arguments.of(20, 10, 20, 20),
                Arguments.of(21, 10, 20, 20));
    }

    /**
     * 用例名称：验证参数修正结果是否在允许范围以内<br/>
     * 前置条件：无<br/>
     * check点：参数修正结果是否在允许范围以内<br/>
     *
     * @param value 配置参数
     * @param min 参数最小值
     * @param max 参数最大值
     * @param expected 预期结果
     */
    @ParameterizedTest
    @MethodSource("revise_cases")
    public void test_revise(int value, int min, int max, int expected) {
        Assertions.assertEquals(expected, DataSourceConfig.revise(value, min, max));
    }

    /**
     * init pool config case data
     *
     * @return init pool config case data
     */
    public static Stream<Arguments> init_pool_config_cases() {
        return Stream.of(
                Arguments.of(0, 0, 5, 5),
                Arguments.of(5, 0, 5, 5),
                Arguments.of(6, 0, 6, 6),
                Arguments.of(50, 0, 50, 50),
                Arguments.of(51, 0, 50, 50),
                Arguments.of(51, 51, 50, 51),
                Arguments.of(51, 100, 50, 100),
                Arguments.of(51, 101, 50, 100));
    }

    /**
     * 用例名称：验证线程池参数初始化是否正确<br/>
     * 前置条件：无<br/>
     * check点：线程池参数初始化在允许范围内<br/>
     *
     * @param minimumIdleValue minimumIdleValue
     * @param maximumPoolSizeValue maximumPoolSizeValue
     * @param expectMinimumIdleValue expectMinimumIdleValue
     * @param expectMaximumPoolSizeValue expectMaximumPoolSizeValue
     * @throws NoSuchMethodException NoSuchMethodException
     * @throws InvocationTargetException InvocationTargetException
     * @throws IllegalAccessException IllegalAccessException
     * @throws NoSuchFieldException NoSuchFieldException
     */
    @ParameterizedTest
    @MethodSource("init_pool_config_cases")
    public void test_init_pool_config(
            int minimumIdleValue, int maximumPoolSizeValue, int expectMinimumIdleValue, int expectMaximumPoolSizeValue)
            throws NoSuchMethodException, InvocationTargetException, IllegalAccessException, NoSuchFieldException {
        DataSourceConfig config = createConfig(minimumIdleValue, maximumPoolSizeValue);
        Method method = DataSourceConfig.class.getDeclaredMethod("initPoolConfig", HikariDataSource.class);
        HikariDataSource dataSource = new HikariDataSource();
        method.setAccessible(true);
        method.invoke(config, dataSource);
        Assertions.assertEquals(expectMinimumIdleValue, dataSource.getMinimumIdle());
        Assertions.assertEquals(expectMaximumPoolSizeValue, dataSource.getMaximumPoolSize());
    }

    private DataSourceConfig createConfig(int minimumIdleValue, int maximumPoolSizeValue)
            throws NoSuchFieldException, IllegalAccessException {
        DataSourceConfig config = new DataSourceConfig();
        initFieldValue(config, "minimumIdleValue", minimumIdleValue);
        initFieldValue(config, "maximumPoolSizeValue", maximumPoolSizeValue);
        return config;
    }

    private void initFieldValue(DataSourceConfig config, String name, int value)
            throws NoSuchFieldException, IllegalAccessException {
        Field field = DataSourceConfig.class.getDeclaredField(name);
        field.setAccessible(true);
        field.set(config, value);
    }
}
