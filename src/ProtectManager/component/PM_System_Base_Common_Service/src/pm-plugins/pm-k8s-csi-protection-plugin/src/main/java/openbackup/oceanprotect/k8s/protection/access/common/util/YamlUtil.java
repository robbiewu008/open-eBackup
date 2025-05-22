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
package openbackup.oceanprotect.k8s.protection.access.common.util;

import com.alibaba.fastjson.JSONObject;

import lombok.extern.slf4j.Slf4j;
import openbackup.system.base.common.exception.DataMoverErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;

import org.yaml.snakeyaml.Yaml;
import org.yaml.snakeyaml.constructor.Constructor;
import org.yaml.snakeyaml.constructor.ConstructorException;

import java.util.Iterator;

/**
 * Yaml转换java实体类
 *
 * @author t30049904
 * @version [OceanProtect DataBack 1.5.0]
 * @since 2023/9/22
 */
@Slf4j
public class YamlUtil {
    /**
     * read yaml data as the special type data
     *
     * @param input yaml data
     * @param type type
     * @param <T> template type
     * @return result
     */
    public static <T> T read(String input, Class<T> type) {
        try {
            Yaml yaml = new Yaml(new Constructor(JSONObject.class));
            Iterable<Object> objects = yaml.loadAll(input);
            Iterator<Object> iterator = objects.iterator();
            JSONObject target = null;
            while (iterator.hasNext()) {
                Object tmp = iterator.next();
                if (tmp instanceof JSONObject) {
                    target = (JSONObject) tmp;
                    break;
                }
            }
            return (target == null ? new JSONObject() : target).toJavaObject(type);
        } catch (ConstructorException e) {
            throw new LegoCheckedException(DataMoverErrorCode.OPERATION_FAILED, "this yaml not correct");
        }
    }
}
