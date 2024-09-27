/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

package openbackup.system.base.common.utils.json;

import openbackup.system.base.common.utils.JSONObject;

import com.google.common.base.CaseFormat;

/**
 * JSONObject转换工具类
 *
 * @author: y00559272
 * @version: [OceanProtect A8000 1.1.0]
 * @since: 2021/12/28
 **/
public class JSONObjectCovert {
    /**
     * 将JSONObject中的所有key从小写下划线格式转为小写驼峰
     * <p>
     * 示例代码：
     * source:
     * {
     *     "resource_name": "1111",
     *     "resource_id": "x01838432"
     * }
     * target:
     * {
     *     "resourceName": "1111",
     *     "resourceId": "x01838432"
     * }
     * </p>
     *
     * @param source 需要转换的JSONObject对象
     * @return 转换之后的JSONObject对象
     */
    public static JSONObject covertLowerUnderscoreKeyToLowerCamel(JSONObject source) {
        JSONObject target = new JSONObject();
        source.toMap(Object.class)
                .forEach(
                        (key, value) -> {
                            String lowerCamelKey = key;
                            // 如果key为下划线格式，将其转为小驼峰
                            if (key.contains("_")) {
                                lowerCamelKey = CaseFormat.LOWER_UNDERSCORE.to(CaseFormat.LOWER_CAMEL, key);
                            }
                            // 不为下划线格式无需转换
                            target.put(lowerCamelKey, value);
                        });
        return target;
    }
}
