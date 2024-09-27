package openbackup.system.base.common.utils;

import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import java.util.Collections;
import java.util.List;
import java.util.Map;
import java.util.Objects;
import java.util.Optional;
import java.util.stream.Collectors;
import java.util.stream.IntStream;

/**
 * 集合列表工具类
 *
 * @author j00364432
 * @version [OceanStor BCManager eReplication V200R001C500, 2018-7-25]
 * @since 2019-11-01
 */
public final class CollectionUtils {
    private CollectionUtils() {}

    /**
     * 比较两个集合，返回目标集合中包含，但在源集合中没有的对象
     *
     * @param sourceList 待比较的源集合列表
     * @param targetList 目标集合列表
     * @param <T> 集合中对象的类型
     * @return 返回目标集合中包含，但在源集合中没有的对象
     */
    public static <T> Collection<T> compare(Collection<T> sourceList, Collection<T> targetList) {
        Collection<T> results = new ArrayList<>();
        for (T item : targetList) {
            if (!sourceList.contains(item)) {
                results.add(item);
            }
        }
        return results;
    }

    /**
     * check is array object
     *
     * @param value value
     * @return check result
     */
    public static boolean isArrayObject(Object value) {
        if (value == null) {
            return false;
        }
        return value instanceof Collection || value.getClass().isArray();
    }

    /**
     * cast items to list
     *
     * @param items items
     * @param <T> template type
     * @return list
     */
    public static <T> List<T> list(T[] items) {
        return Optional.ofNullable(items).map(Arrays::asList).orElse(Collections.emptyList());
    }

    /**
     * convert int array to list
     *
     * @param items int array
     * @return list
     */
    public static List<Integer> list(int[] items) {
        return Optional.ofNullable(items)
                .map(Arrays::stream)
                .orElse(IntStream.empty())
                .boxed()
                .collect(Collectors.toList());
    }

    /**
     * non null list
     *
     * @param items items
     * @param <T> template
     * @return result list
     */
    public static <T> List<T> nonNullList(T[] items) {
        return list(items).stream().filter(Objects::nonNull).collect(Collectors.toList());
    }

    /**
     * cast entry collection to map
     *
     * @param collection entry collection
     * @param <K> key template type
     * @param <V> value template type
     * @return map
     */
    public static <K, V> Map<K, V> map(Collection<Map.Entry<K, V>> collection) {
        return collection.stream().collect(Collectors.toMap(Map.Entry::getKey, Map.Entry::getValue));
    }

    /**
     * listify method
     *
     * @param object object
     * @param <T> template type T
     * @return result
     */
    public static <T> List<?> listify(T object) {
        return listify(object, 0);
    }

    /**
     * listify method
     *
     * @param object object
     * @param preemption preemption
     * @param <T> template type T
     * @return result
     */
    public static <T> List<?> listify(T object, int preemption) {
        if (object == null) {
            return preemption <= 0 ? Collections.emptyList() : Arrays.asList(new Object[preemption]);
        }
        if (object instanceof Collection) {
            List<?> list = new ArrayList<>((Collection<?>) object);
            while (list.size() < preemption) {
                list.add(null);
            }
            return list;
        }
        List<Object> list = new ArrayList<>();
        do {
            list.add(object);
        } while (list.size() < preemption);
        return list;
    }

    /**
     * get any object
     *
     * @param object object
     * @param <T> template type T
     * @return any object
     */
    public static <T> Object any(T object) {
        return listify(object).stream().findAny().orElse(null);
    }

    /**
     * get first object
     *
     * @param object object
     * @param <T> template type T
     * @return first object
     */
    public static <T> Object first(T object) {
        return listify(object).stream().findFirst().orElse(null);
    }

    /**
     * get one object
     *
     * @param object object
     * @param <T> template type T
     * @return result
     */
    public static <T> Object one(T object) {
        List<?> list = listify(object);
        if (list.isEmpty()) {
            throw new LegoCheckedException(CommonErrorCode.OBJ_NOT_EXIST);
        }
        if (list.size() > 1) {
            throw new LegoCheckedException(CommonErrorCode.ERR_PARAM);
        }
        return list.get(0);
    }

    /**
     * join with delimiter
     *
     * @param object object
     * @param delimiter delimiter
     * @param <T> template type T
     * @return result
     */
    public static <T> String join(T object, CharSequence delimiter) {
        return listify(object).stream().map(Object::toString).collect(Collectors.joining(delimiter));
    }
}
