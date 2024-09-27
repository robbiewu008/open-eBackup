package openbackup.system.base.common.utils.asserts;

import org.apache.commons.lang3.StringUtils;
import org.springframework.lang.Nullable;
import org.springframework.util.CollectionUtils;

import java.util.Collection;
import java.util.Map;
import java.util.function.Supplier;

/**
 * Assert工具增强类，用来解决代码中通过判断抛业务异常的场景
 * <p>!!!公共代码，如需修改或增加方法，请先跟作者沟通
 *
 * <p>此工具类代码可以消除大部分代码中需要对数据判空等判断条件验证，然后抛出业务异常的场景
 * <p>使用方法参考各个函数注释中的示例
 *
 * @author: y00559272
 * @version: [OceanProtect A8000 1.1.0]
 * @since: 2021/12/20
 **/
public abstract class PowerAssert {
    /**
     * 校验参数表达式，当表达式为False时，支持抛出指定的异常
     * 示例:
     * <pre class="code">
     * PowerAssert.state(a > b, () -> new RuntimeException("a can not less than b"));
     * </pre>
     *
     * @param isNonMatch 值为布尔的表达式
     * @param exceptionSupplier 包含指定异常的函数式接口
     */
    public static void state(boolean isNonMatch, Supplier<RuntimeException> exceptionSupplier) {
        if (!isNonMatch) {
            throw nullSafeGet(exceptionSupplier);
        }
    }

    /**
     * 校验对象为null的时候，支持抛出指定错误信息的LegoCheckedException
     * 示例:
     * <pre class="code">
     * PowerAssert.notNull(value, () -> new RuntimeException("The value can not be null"));
     * </pre>
     *
     * @param object 被校验的对象
     * @param exceptionSupplier 包含指定异常的函数式接口
     */
    public static <T> void notNull(T object, Supplier<RuntimeException> exceptionSupplier) {
        if (object == null) {
            throw nullSafeGet(exceptionSupplier);
        }
    }

    /**
     * 校验集合类为null或者空的时候，支持抛出指定的异常
     * 示例:
     * <pre class="code">
     * PowerAssert.notEmpty(new ArrayList<>(), () -> new RuntimeException("The collection is empty"));
     * </pre>
     *
     * @param collection 被校验的集合对象
     * @param exceptionSupplier 包含指定异常的函数式接口
     */
    public static void notEmpty(Collection<?> collection, Supplier<RuntimeException> exceptionSupplier) {
        if (CollectionUtils.isEmpty(collection)) {
            throw nullSafeGet(exceptionSupplier);
        }
    }

    /**
     * 校验map对象为null或者空的时候，支持抛出指定的异常
     * 示例:
     * <pre class="code">
     * PowerAssert.notEmpty(new HashMap<>(), () -> new RuntimeException("The map is empty"));
     * </pre>
     *
     * @param map 被校验的map对象
     * @param exceptionSupplier 包含指定异常的函数式接口
     */
    public static void notEmpty(Map<?, ?> map, Supplier<RuntimeException> exceptionSupplier) {
        if (CollectionUtils.isEmpty(map)) {
            throw nullSafeGet(exceptionSupplier);
        }
    }

    /**
     * 校验字符串为空或者空串时，当表达式为False时，支持抛出指定的异常
     * 示例:
     * <pre class="code">
     * PowerAssert.notBlank(value, () -> new RuntimeException("Value can not be empty"));
     * </pre>
     *
     * @param value 被检验的字符串
     * @param exceptionSupplier 包含指定异常的函数式接口
     */
    public static void notBlank(String value, Supplier<RuntimeException> exceptionSupplier) {
        if (StringUtils.isBlank(value)) {
            throw nullSafeGet(exceptionSupplier);
        }
    }

    private static RuntimeException nullSafeGet(@Nullable Supplier<RuntimeException> messageSupplier) {
        return (messageSupplier != null ? messageSupplier.get() : new RuntimeException());
    }
}
