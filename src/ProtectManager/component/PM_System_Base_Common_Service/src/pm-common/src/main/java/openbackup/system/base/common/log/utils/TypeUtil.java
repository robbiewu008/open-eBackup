package openbackup.system.base.common.log.utils;

import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;

import org.apache.commons.collections.CollectionUtils;
import org.springframework.core.annotation.AnnotatedElementUtils;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.ResponseBody;

import java.lang.reflect.GenericArrayType;
import java.lang.reflect.Method;
import java.lang.reflect.ParameterizedType;
import java.lang.reflect.Type;
import java.util.Arrays;
import java.util.Collection;
import java.util.Collections;
import java.util.Optional;

/**
 * type util
 *
 * @author l00272247
 * @since 2019-11-07
 */
public class TypeUtil {
    /**
     * constructor
     */
    protected TypeUtil() {
    }

    /**
     * check the object is array or collection
     *
     * @param object object
     * @return result
     */
    public static boolean isArrayObject(Object object) {
        if (object == null) {
            return false;
        }
        return object.getClass().isArray() || object instanceof Collection;
    }

    /**
     * check the base type is assignable from subtype
     *
     * @param baseType base type
     * @param subType  subtype
     * @return result
     */
    public static boolean isAssignableFrom(Type baseType, Type subType) {
        if (baseType == null || subType == null) {
            return false;
        }
        final Optional<Boolean> sameType = getBoolean(baseType, subType);
        if (sameType.isPresent()) {
            return sameType.get();
        }
        final Optional<Boolean> sameArrayType = getArrayType(baseType, subType);
        if (sameArrayType.isPresent()) {
            return sameArrayType.get();
        }
        if (baseType instanceof ParameterizedType) {
            ParameterizedType baseParameterizedType = (ParameterizedType) baseType;
            Class<?> baseRawType = (Class<?>) baseParameterizedType.getRawType();
            if (subType instanceof ParameterizedType) {
                ParameterizedType subParameterizedType = (ParameterizedType) subType;
                Class<?> subRawType = (Class<?>) subParameterizedType.getRawType();
                if (!baseRawType.isAssignableFrom(subRawType)) {
                    return false;
                }
                Type[] baseActualTypeArguments = baseParameterizedType.getActualTypeArguments();
                Type[] subActualTypeArguments = subParameterizedType.getActualTypeArguments();
                int length = baseActualTypeArguments.length;
                if (length != subActualTypeArguments.length) {
                    return false;
                }
                return !isAssign(baseActualTypeArguments, subActualTypeArguments, length);
            }
            return false;
        }
        return false;
    }

    private static Optional<Boolean> getArrayType(Type baseType, Type subType) {
        if (baseType instanceof GenericArrayType) {
            if (subType instanceof GenericArrayType) {
                return Optional.of(isAssignableFrom(((GenericArrayType) baseType).getGenericComponentType(),
                    ((GenericArrayType) subType).getGenericComponentType()));
            }
            return Optional.of(false);
        }
        return Optional.empty();
    }

    private static Optional<Boolean> getBoolean(Type baseType, Type subType) {
        if (baseType instanceof Class) {
            Class<?> type = (Class) baseType;
            if (Object.class.equals(type)) {
                return Optional.of(true);
            }
            if (subType instanceof Class) {
                return Optional.of(type.isAssignableFrom((Class) subType));
            }
            return Optional.of(false);
        }
        return Optional.empty();
    }

    private static boolean isAssign(Type[] baseActualTypeArguments, Type[] subActualTypeArguments, int length) {
        for (int index = 0; index < length; index++) {
            if (!isAssignableFrom(baseActualTypeArguments[index], subActualTypeArguments[index])) {
                return true;
            }
        }
        return false;
    }

    /**
     * check the object is array or collection
     *
     * @param type type
     * @return result
     */
    public static boolean isArrayRelatedType(Type type) {
        if (type == null) {
            return false;
        }
        if (type instanceof Class) {
            return ((Class) type).isArray();
        }
        if (type instanceof GenericArrayType) {
            return true;
        }
        if (!(type instanceof ParameterizedType)) {
            return false;
        }
        ParameterizedType parameterizedType = (ParameterizedType) type;
        Type ownerType = parameterizedType.getOwnerType();
        return isAssignableFrom(Collection.class, ownerType);
    }

    /**
     * get array element type
     *
     * @param type should be array or collection type
     * @return element type
     */
    public static Type getArrayElementType(Type type) {
        if (type instanceof GenericArrayType) {
            return ((GenericArrayType) type).getGenericComponentType();
        }
        if (!(type instanceof ParameterizedType)) {
            throw new LegoCheckedException(CommonErrorCode.ILLEGAL_PARAM,
                "Type is not instance of ParameterizedType");
        }
        ParameterizedType parameterizedType = (ParameterizedType) type;
        if (!isAssignableFrom(Collection.class, parameterizedType.getOwnerType())) {
            throw new LegoCheckedException("not collection type: " + type);
        }
        return parameterizedType.getActualTypeArguments()[0];
    }

    /**
     * cast object as collection
     *
     * @param data data
     * @return collection
     */
    public static Collection<?> castObjectAsCollection(Object data) {
        if (data == null) {
            return CollectionUtils.EMPTY_COLLECTION;
        }
        if (data instanceof Collection) {
            return (Collection) data;
        }
        if (data.getClass().isArray()) {
            return Arrays.asList((Object[]) data);
        }
        return Collections.singleton(data);
    }

    /**
     * check if the method is rest operation method
     *
     * @param method method
     * @return result
     */
    public static boolean isRestOperationMethod(Method method) {
        Class owner = method.getDeclaringClass();
        if (!AnnotatedElementUtils.hasAnnotation(method, RequestMapping.class)) {
            return false;
        }
        if (!AnnotatedElementUtils.hasAnnotation(owner, RequestMapping.class)) {
            return false;
        }
        if (AnnotatedElementUtils.hasAnnotation(owner, ResponseBody.class)) {
            return true;
        }
        return AnnotatedElementUtils.hasAnnotation(method, ResponseBody.class);
    }
}
