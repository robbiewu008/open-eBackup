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
package openbackup.system.base.query;

import openbackup.system.base.common.constants.ErrorCodeConstant;
import openbackup.system.base.common.constants.TokenBo;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.JSONArray;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.common.utils.StringUtil;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.common.validator.constants.RegexpConstants;
import openbackup.system.base.sdk.copy.model.BasePage;
import openbackup.system.base.util.IdUtil;
import openbackup.system.base.util.RequestParamFilterUtil;

import com.baomidou.mybatisplus.core.conditions.query.QueryWrapper;
import com.baomidou.mybatisplus.core.metadata.IPage;
import com.baomidou.mybatisplus.extension.plugins.pagination.Page;

import lombok.extern.slf4j.Slf4j;

import org.apache.logging.log4j.util.Strings;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.context.ApplicationContext;
import org.springframework.core.annotation.AnnotatedElementUtils;
import org.springframework.stereotype.Component;

import java.lang.annotation.Annotation;
import java.lang.reflect.Field;
import java.util.AbstractMap;
import java.util.ArrayList;
import java.util.Collection;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Optional;
import java.util.Set;
import java.util.function.BiFunction;
import java.util.stream.Collectors;
import java.util.stream.Stream;

/**
 * Page Query Service
 *
 * @author l00272247
 * @since 2020-09-24
 */
@Component
@Slf4j
public class PageQueryService {
    private static final Map<String, PageQueryOperate> OPERATE_MAPPING = createOperateMapping();

    @Autowired
    private ApplicationContext applicationContext;

    @Autowired
    private SessionService sessionService;

    /**
     * split conditions
     *
     * @param conditions conditions
     * @return split result
     */
    public static List<Map.Entry<String, List<?>>> splitConditions(JSONArray conditions) {
        int ending = conditions.size();
        List<Map.Entry<String, List<?>>> list = new ArrayList<>();
        for (int index = conditions.size(); index > 0;) {
            Object item = conditions.get(--index);
            if (item instanceof List) {
                List<?> collection = (List<?>) item;
                if (collection.isEmpty()) {
                    continue;
                }
                String operator = String.valueOf(collection.get(0));
                List<?> parameters = conditions.subList(index + 1, ending);
                ending = index;
                list.add(new AbstractMap.SimpleEntry<>(operator, parameters));
            }
        }
        List<?> parameters = conditions.subList(0, ending);
        if (!parameters.isEmpty()) {
            list.add(new AbstractMap.SimpleEntry<>("in", parameters));
        }
        return list;
    }

    private static String lower(String column) {
        return "lower(" + column + ")";
    }

    private static <T> QueryWrapper<T> fullLikeOperate(QueryWrapper<T> wrapper, String column, List<?> parameters) {
        if (parameters.isEmpty()) {
            return wrapper;
        }
        String value = RequestParamFilterUtil.escape(parameters.get(0));
        return wrapper.like(value != null, lower(column), value);
    }

    private static <T> QueryWrapper<T> likeLeftOperate(QueryWrapper<T> wrapper, String column, List<?> parameters) {
        if (parameters.isEmpty()) {
            return wrapper;
        }
        String value = RequestParamFilterUtil.escape(parameters.get(0));
        return wrapper.likeLeft(value != null, lower(column), value);
    }

    private static <T> QueryWrapper<T> likeRightOperate(QueryWrapper<T> wrapper, String column, List<?> parameters) {
        if (parameters.isEmpty()) {
            return wrapper;
        }
        String value = RequestParamFilterUtil.escape(parameters.get(0));
        return wrapper.likeRight(value != null, lower(column), value);
    }

    private static <T> QueryWrapper<T> eqOperate(QueryWrapper<T> wrapper, String column, List<?> parameters) {
        Object value = parameters.stream().findFirst().orElse(null);
        if (value == null) {
            return wrapper.isNull(column);
        }
        return eq(wrapper, column, value);
    }

    private static <T> QueryWrapper<T> neOperate(QueryWrapper<T> wrapper, String column, List<?> parameters) {
        Object value = parameters.stream().findFirst().orElse(null);
        if (value == null) {
            return wrapper.isNotNull(column);
        }
        return ne(wrapper, column, value);
    }

    private static <T> QueryWrapper<T> leOperate(QueryWrapper<T> wrapper, String column, List<?> parameters) {
        if (parameters.isEmpty()) {
            return wrapper;
        }
        return wrapper.le(column, parameters.get(0));
    }

    private static <T> QueryWrapper<T> ltOperate(QueryWrapper<T> wrapper, String column, List<?> parameters) {
        if (parameters.isEmpty()) {
            return wrapper;
        }
        return wrapper.lt(column, parameters.get(0));
    }

    private static <T> QueryWrapper<T> geOperate(QueryWrapper<T> wrapper, String column, List<?> parameters) {
        if (parameters.isEmpty()) {
            return wrapper;
        }
        return wrapper.ge(column, parameters.get(0));
    }

    private static <T> QueryWrapper<T> gtOperate(QueryWrapper<T> wrapper, String column, List<?> parameters) {
        if (parameters.isEmpty()) {
            return wrapper;
        }
        return wrapper.gt(column, parameters.get(0));
    }

    private static <T> QueryWrapper<T> inOperate(QueryWrapper<T> wrapper, String column, List<?> parameters) {
        if (parameters.isEmpty()) {
            return wrapper;
        }
        return wrapper.in(column, parameters);
    }

    private static <T> QueryWrapper<T> notInOperate(QueryWrapper<T> wrapper, String column, List<?> parameters) {
        if (parameters.isEmpty()) {
            return wrapper;
        }
        return wrapper.notIn(column, parameters);
    }

    private static Map<String, PageQueryOperate> createOperateMapping() {
        Map<String, PageQueryOperate> map = new HashMap<>();
        map.put(PageQueryOperator.EQ.getValue(), PageQueryService::eqOperate);
        map.put(PageQueryOperator.NE.getValue(), PageQueryService::neOperate);
        map.put(PageQueryOperator.LIKE.getValue(), PageQueryService::fullLikeOperate);
        map.put(PageQueryOperator.LIKE_RIGHT.getValue(), PageQueryService::likeRightOperate);
        map.put(PageQueryOperator.LIKE_LEFT.getValue(), PageQueryService::likeLeftOperate);
        map.put(PageQueryOperator.IN.getValue(), PageQueryService::inOperate);
        map.put(PageQueryOperator.NOT_IN.getValue(), PageQueryService::notInOperate);
        map.put(PageQueryOperator.GT.getValue(), PageQueryService::gtOperate);
        map.put(PageQueryOperator.GE.getValue(), PageQueryService::geOperate);
        map.put(PageQueryOperator.LT.getValue(), PageQueryService::ltOperate);
        map.put(PageQueryOperator.LE.getValue(), PageQueryService::leOperate);
        return Collections.unmodifiableMap(map);
    }

    /**
     * page query
     *
     * @param type type
     * @param dao dao
     * @param pageQueryParam param
     * @param defaultOrder default order
     * @param <E> template type E
     * @param <T> template type T
     * @param <R> template type R
     * @return base page
     */
    public <E, T, R> BasePage<R> pageQuery(
        Class<T> type,
        BiFunction<IPage<T>, QueryWrapper<T>, IPage<R>> dao,
        Pagination<E> pageQueryParam,
        String defaultOrder) {
        return pageQuery(type, dao, pageQueryParam, defaultOrder, null);
    }

    /**
     * page query
     *
     * @param type type
     * @param dao dao
     * @param pageQueryParam param
     * @param defaultOrder default order
     * @param owner owner表的主键字段
     * @param <E> template type E
     * @param <T> template type T
     * @param <R> template type R
     * @return base page
     */
    public <E, T, R> BasePage<R> pageQuery(
        Class<T> type,
        BiFunction<IPage<T>, QueryWrapper<T>, IPage<R>> dao,
        Pagination<E> pageQueryParam,
        String defaultOrder,
        String owner) {
        JSONObject condition = pageQueryParam.conditions();
        List<String> orderList = getValidOrders(pageQueryParam, defaultOrder);
        QueryWrapper<T> wrapper = buildQueryWrapper(condition, type, pageQueryParam.getFieldNamingStrategy());
        wrapper = addOwnerFilter(wrapper, owner);
        for (String orderBy : orderList) {
            boolean isAsc = orderBy.startsWith("+");
            String field = StringUtil.mapCamelCaseToUnderscore(orderBy.substring(1));
            wrapper = wrapper.orderBy(true, isAsc, field);
        }
        int max = Math.max(pageQueryParam.getPage(), 0);
        Page<T> page = new Page<>(max + 1, pageQueryParam.getSize());
        IPage<R> pageData = dao.apply(page, wrapper);
        BasePage<R> basePage = new BasePage<>();
        basePage.setPageNo(pageQueryParam.getPage());
        basePage.setPageSize(pageData.getSize());
        basePage.setPages(pageData.getPages());
        basePage.setTotal(pageData.getTotal());
        basePage.setItems(pageData.getRecords());
        return basePage;
    }

    private <T> QueryWrapper<T> buildQueryWrapper(JSONObject condition, Class<T> type, String strategy) {
        QueryWrapper<T> wrapper = new QueryWrapper<>();
        for (Object key : condition.keySet()) {
            String field = key.toString();
            Object value = condition.get(field);
            if (VerifyUtil.isEmpty(value) || isForbidField(type, field, strategy)) {
                continue;
            }
            wrapper = addQueryCondition(wrapper, type, field, value);
        }
        return wrapper;
    }

    /**
     * add query condition
     *
     * @param wrapper wrapper
     * @param field field
     * @param value value
     * @param <E> template type E
     * @param <T> template type T
     * @return query wrapper
     */
    public <E, T> QueryWrapper<T> addQueryCondition(QueryWrapper<T> wrapper, String field, E value) {
        return addQueryCondition(wrapper, null, field, value);
    }

    private <E, T> QueryWrapper<T> addQueryCondition(QueryWrapper<T> wrapper, Class<T> type, String field, E value) {
        if (value instanceof JSONArray) {
            return addFieldConditions(wrapper, field, (JSONArray) value);
        } else if (value instanceof Collection) {
            return addFieldConditions(wrapper, field, new JSONArray((Collection<?>) value));
        } else {
            return addFieldCondition(wrapper, type, field, value);
        }
    }

    private <T> QueryWrapper<T> addFieldConditions(QueryWrapper<T> wrapper, String field, JSONArray value) {
        List<Map.Entry<String, List<?>>> conditions = splitConditions(value);
        QueryWrapper<T> queryWrapper = wrapper;
        for (Map.Entry<String, List<?>> condition : conditions) {
            queryWrapper = addFieldCondition(queryWrapper, field, condition);
        }
        return queryWrapper;
    }

    private <T> QueryWrapper<T> addFieldCondition(
        QueryWrapper<T> wrapper, String key, Map.Entry<String, List<?>> condition) {
        List<?> parameters = condition.getValue();
        String operator = condition.getKey();
        String column = StringUtil.mapCamelCaseToUnderscore(key);
        PageQueryOperate operate = OPERATE_MAPPING.get(operator);
        if (operate == null) {
            return wrapper;
        }
        return operate.apply(wrapper, column, parameters);
    }

    private <E, T> QueryWrapper<T> addFieldCondition(QueryWrapper<T> wrapper, Class<T> type, String field, E value) {
        Optional<String> config = getFuzzyMatchableFieldConfig(type, field);
        if (config.isPresent()) {
            String escapedValue = RequestParamFilterUtil.escape(value);
            String pattern = config.get().replace(field, escapedValue);
            return wrapper.like(lower(field), pattern);
        } else if (value != null) {
            return eq(wrapper, StringUtil.mapCamelCaseToUnderscore(field), value);
        } else {
            return wrapper.isNull(StringUtil.mapCamelCaseToUnderscore(field));
        }
    }

    private static <E, T> QueryWrapper<T> ne(QueryWrapper<T> wrapper, String field, E value) {
        return wrapper.and(query -> query.isNull(field).or(qw -> qw.isNotNull(field).ne(field, value)));
    }

    private static <E, T> QueryWrapper<T> eq(QueryWrapper<T> wrapper, String field, E value) {
        return wrapper.and(query -> query.isNotNull(field).eq(field, value));
    }

    private <T> QueryWrapper<T> addOwnerFilter(QueryWrapper<T> wrapper, String owner) {
        if (owner == null) {
            return wrapper;
        }
        TokenBo.UserBo user = sessionService.getCurrentUser();
        if (user == null || Strings.isEmpty(user.getDomainId())) {
            return wrapper;
        }
        if (!IdUtil.isUUID(user.getDomainId())) {
            throw new LegoCheckedException(ErrorCodeConstant.ERR_PARAM, "user domain id incorrect");
        }
        return wrapper.inSql(owner,
            "select resource_object_id from t_domain_r_resource_object where domain_id = '" + user.getDomainId()
                + "'");
    }

    private <E> List<String> getValidOrders(Pagination<E> pageQueryParam, String defaultOrder) {
        List<String> orders =
            Optional.ofNullable(pageQueryParam.getOrders()).orElse(Collections.emptyList()).stream()
                .filter(order -> !VerifyUtil.isEmpty(order))
                .filter(this::isValidOrderConfig)
                .collect(Collectors.toList());
        if (defaultOrder == null) {
            return orders;
        }
        for (String order : defaultOrder.split(",")) {
            order = order.trim();
            if (isValidOrderConfig(order)) {
                String defaultOrderField = order.substring(1);
                boolean isMissing = orders.stream().map(item -> item.substring(1)).noneMatch(defaultOrderField::equals);
                if (isMissing) {
                    orders.add(order);
                }
            }
        }
        return orders;
    }

    private boolean isValidOrderConfig(String config) {
        return config != null
            && config.matches(RegexpConstants.REGEXP_PAGE_QUERY_ORDER_BY)
            && !config.contains(";");
    }

    private <A extends Annotation> List<A> getMergedRepeatableAnnotations(Class<?> type, Class<A> annotationType) {
        if (type == null) {
            return Collections.emptyList();
        }
        return Stream.concat(
            AnnotatedElementUtils.getMergedRepeatableAnnotations(type, annotationType).stream(),
            getMergedRepeatableAnnotations(type.getSuperclass(), annotationType).stream())
            .collect(Collectors.toList());
    }

    private boolean isForbidField(Class<?> type, String name, String strategy) {
        Set<String> specials = getSpecialFieldsConfig(type);
        boolean isExcludeField = specials.contains("!" + name);
        boolean isForbidField = !specials.contains(name) && !hasField(type, name, strategy);
        return isExcludeField || isForbidField;
    }

    private Optional<String> getFuzzyMatchableFieldConfig(Class<?> type, String name) {
        Set<String> specials = getSpecialFieldsConfig(type);
        return specials.stream()
            .filter(special -> !special.startsWith("!") && special.contains("%"))
            .filter(special -> special.replaceAll("^%|%$", "").equals(name))
            .findFirst();
    }

    private Set<String> getSpecialFieldsConfig(Class<?> type) {
        if (type == null) {
            return Collections.emptySet();
        }
        List<PageQueryConfig> configs = getMergedRepeatableAnnotations(type, PageQueryConfig.class);
        return configs.stream().flatMap(config -> Stream.of(config.conditions())).collect(Collectors.toSet());
    }

    private boolean hasField(Class<?> type, String name, String strategy) {
        if (type == null) {
            return false;
        }
        Field[] fields = type.getDeclaredFields();
        FieldNamingStrategy fieldNamingStrategy = getStrategyByName(strategy);
        for (Field field : fields) {
            String fieldName;
            if (fieldNamingStrategy != null) {
                fieldName = fieldNamingStrategy.translate(type, field);
            } else {
                fieldName = field.getName();
            }
            if (fieldName.equals(name)) {
                return true;
            }
        }
        return hasField(type.getSuperclass(), name, strategy);
    }

    private FieldNamingStrategy getStrategyByName(String name) {
        return Optional.ofNullable(name)
            .map(component -> applicationContext.getBean(component))
            .filter(strategyObject -> strategyObject instanceof FieldNamingStrategy)
            .map(strategyObject -> (FieldNamingStrategy) strategyObject)
            .orElse(null);
    }
}
