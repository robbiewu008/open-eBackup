package openbackup.system.base.security.callee;

import java.lang.reflect.InvocationTargetException;

/**
 * Evaluator
 *
 * @author l00272247
 * @since 2021-12-14
 */
public interface Callee {
    /**
     * evaluate method
     *
     * @param args 请求参数
     * @return Object 响应对象
     * @throws InvocationTargetException 反射时调用方法或者构造函数异常
     * @throws IllegalAccessException 非法访问异常
     */
    Object call(Object... args) throws InvocationTargetException, IllegalAccessException;
}
