package openbackup.system.base.common.utils;

import openbackup.system.base.common.exception.EmeiStorDefaultExceptionHandler;
import openbackup.system.base.common.exception.LegoCheckedException;

import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import java.util.List;
import java.util.Locale;
import java.util.Map;
import java.util.concurrent.atomic.AtomicBoolean;
import java.util.concurrent.atomic.AtomicInteger;
import java.util.function.Function;
import java.util.function.Supplier;
import java.util.stream.Collectors;

/**
 * expr util
 *
 * @author l00272247
 * @since 2019-11-06
 */
public class ExprUtil {
    /**
     * constructor
     */
    protected ExprUtil() {
    }

    /**
     * expression normalize
     *
     * @param expr expression
     * @return normalize result
     */
    public static String normalize(String expr) {
        return expr.replaceAll("\\s*([\\?\\.])\\s*", "$1");
    }

    /**
     * expression evaluate. supports:<br/>
     * <li>field: a.c.b</li>
     * <li>nullable: a?.c?.b</li>
     * <li>collection: a.0.b</li>
     * <li>each: a.*.b (support array/collection/map)</li>
     * <li>size: a.size() (support array/collection/map)</li>
     * <li>stringify: a.string()</li>
     * <li>jsonify: not support</li>
     *
     * @param object object
     * @param expr   expression
     * @return result
     */
    public static Object eval(Object object, String expr) {
        return eval(object, expr, false);
    }

    /**
     * expression evaluate. supports:<br/>
     * <li>field: a.c.b</li>
     * <li>nullable: a?.c?.b</li>
     * <li>collection: a.0.b</li>
     * <li>each: a.*.b (support array/collection/map)</li>
     * <li>size: a.size() (support array/collection/map)</li>
     * <li>stringify: a.string()</li>
     * <li>jsonify: not support</li>
     *
     * @param object object
     * @param expr   expression
     * @param isStrict isStrict mode
     * @return result
     */
    public static Object eval(Object object, String expr, boolean isStrict) {
        return new Context(object, normalize(expr), isStrict).eval();
    }

    private static class Context {
        private final Object object;

        private final String expr;

        private final boolean isStrict;

        private final AtomicInteger depth;

        private final AtomicBoolean combine;

        public Context(Object object, String expr, boolean isStrict) {
            this.object = object;
            this.expr = expr;
            this.isStrict = isStrict;
            depth = new AtomicInteger();
            combine = new AtomicBoolean();
        }

        private static String getFirstField(String intent, int offset) {
            int index = intent.indexOf(".", offset);
            return index >= 0 ? intent.substring(offset, index) : intent.substring(offset);
        }

        private static Object getCollectionItemAtIndex(Collection collection, int index,
            Function<Object, Object> converter) {
            int count = 0;
            for (Object item : collection) {
                if (index == count++) {
                    return converter.apply(item);
                }
            }
            return null;
        }

        private static Object getValueFromObject(Object object, String field, Function<Object, Object> converter) {
            Object value;
            Method getter = object != null ? getFieldGetterMethod(object.getClass(), field) : null;
            if (getter == null) {
                value = null;
            } else {
                boolean isAccessible = getter.isAccessible();
                try {
                    getter.setAccessible(true);
                    value = getter.invoke(object);
                } catch (IllegalAccessException | InvocationTargetException ignore) {
                    value = null;
                } finally {
                    getter.setAccessible(isAccessible);
                }
            }
            if (value != null) {
                return convert(value, converter);
            }
            return value;
        }

        private static Method getFieldGetterMethod(Class<?> clazz, String field) {
            String suffix;
            if (field.matches("^[a-z][A-Z].*$")) {
                suffix = field;
            } else {
                suffix = field.substring(0, 1).toUpperCase(Locale.getDefault()) + field.substring(1);
            }
            Method getter = getMethod(clazz, "get" + suffix);
            if (getter != null) {
                return getter;
            }
            getter = getMethod(clazz, "is" + suffix);
            if (getter == null) {
                return null;
            }
            List<Class<?>> booleanTypes = Arrays.asList(boolean.class, Boolean.class);
            return booleanTypes.contains(getter.getReturnType()) ? getter : null;
        }

        private static Method getMethod(Class<?> clazz, String name) {
            try {
                return clazz.getMethod(name);
            } catch (NoSuchMethodException ignore) {
                return null;
            }
        }

        private static Collection<?> combine(Collection<?> result, int depth) {
            List<Object> list = new ArrayList<>();
            for (Object item : result) {
                if (depth == 0) {
                    combine(list, item);
                } else if (item instanceof Collection) {
                    list.addAll((Collection<?>) item);
                } else {
                    list.add(item);
                }
            }
            return list;
        }

        private static void combine(List<Object> list, Object item) {
            Object data = unwrap(item);
            if (data instanceof Collection) {
                Collection<?> collection = (Collection<?>) data;
                for (Object element : collection) {
                    list.add(unwrap(element));
                }
            } else {
                list.add(data);
            }
        }

        private static Object unwrap(Object object) {
            return object instanceof Container ? ((Container) object).supplier.get() : object;
        }

        private static Object convert(Object object, Function<Object, Object> converter) {
            return converter != null ? converter.apply(object) : object;
        }

        /**
         * eval
         *
         * @return Object
         */
        public Object eval() {
            return eval(object, expr, 0);
        }

        private Object eval(Object object, String expr, int offset) {
            if (offset >= expr.length()) {
                return object;
            }
            int position = Math.max(offset, 0);
            String item = getFirstField(expr, position);
            boolean isNullable = item.endsWith("?");
            String prefix;
            if (isNullable || !isStrict) {
                if (object == null) {
                    return null;
                }
                prefix = isNullable ? item.substring(0, item.length() - 1) : item;
            } else if (object == null) {
                throw new EmeiStorDefaultExceptionHandler("object is null");
            } else {
                prefix = item;
            }
            String temp = prefix.replaceAll("##", "");
            boolean isStartWithPound = temp.startsWith("#");
            String infix = isStartWithPound ? temp.substring(1) : temp;
            if (isStartWithPound) {
                combine.set(true);
            }
            String field = infix.replaceAll("##", "#");
            int ending = position + item.length() + 1;
            boolean isLast = ending >= expr.length();
            return getValueByField(object, field,
                getValueConverter(expr, ending, isLast, isNullable, isStartWithPound));
        }

        private Function<Object, Object> getValueConverter(String expr, int ending, boolean isLast, boolean isNullable,
            boolean isStartWithPound) {
            Function<Object, Object> cast = result -> {
                if (result == null) {
                    if (isLast || isNullable || !isStrict) {
                        return null;
                    } else {
                        throw new LegoCheckedException("value of expr '" + expr.substring(0, ending) + "' is null");
                    }
                }
                return isLast ? result : eval(result, expr, ending);
            };
            return result -> isStartWithPound ? new Container(() -> cast.apply(result)) : cast.apply(result);
        }

        private Object getValueByField(Object object, String field, Function<Object, Object> converter) {
            Object result = object;
            if ("string()".equals(field.replaceAll("\\s+", ""))) {
                result = convert(String.valueOf(result), converter);
            } else if (result instanceof Collection) {
                result = getValueFromCollection((Collection) result, field, converter);
            } else if (result instanceof Map) {
                result = getValueFromMap((Map) result, field, converter);
            } else if (object.getClass().isArray()) {
                result = getValueFromCollection(Arrays.asList((Object[]) object), field, converter);
            } else {
                result = getValueFromObject(result, field, converter);
            }
            return result;
        }

        private Object getValueFromCollection(Collection<?> collection, String field,
            Function<Object, Object> converter) {
            if ("*".equalsIgnoreCase(field)) {
                return each(collection, converter);
            } else if ("size()".equals(field.replaceAll("\\s+", ""))) {
                return collection.size();
            } else if ("join()".equals(field.replaceAll("\\s+", ""))) {
                return collection.stream()
                    .filter(Collection.class::isInstance)
                    .map(Collection.class::cast)
                    .flatMap(Collection::stream)
                    .collect(Collectors.toList());
            } else if (field.matches("\\d+")) {
                return getCollectionItemAtIndex(collection, Integer.parseInt(field), converter);
            }
            return null;
        }

        private Object getValueFromMap(Map map, String field, Function<Object, Object> converter) {
            if ("*".equalsIgnoreCase(field)) {
                return each(map.entrySet(), converter);
            } else if ("size()".equals(field)) {
                return convert(map.size(), converter);
            } else {
                return convert(map.get(field), converter);
            }
        }

        private Object each(Collection<?> collection, Function<Object, Object> converter) {
            if (collection == null) {
                return null;
            }
            depth.incrementAndGet();
            Collection<?> result = collection.stream().map(converter).collect(Collectors.toList());
            int current = depth.decrementAndGet();
            if (!combine.get()) {
                return result;
            }
            if (current == 0) {
                combine.set(false);
            }
            return combine(result, current);
        }
    }

    private static class Container {
        private final Supplier<Object> supplier;

        public Container(Supplier<Object> supplier) {
            this.supplier = supplier;
        }
    }
}
