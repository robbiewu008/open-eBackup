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
package openbackup.system.base.util;

import com.fasterxml.jackson.databind.JsonNode;

import lombok.AccessLevel;
import lombok.RequiredArgsConstructor;

import java.util.List;
import java.util.Optional;

/**
 * JsonNodeUtil
 *
 * @author y30044273
 * @version [OceanProtect DataBackup 1.6.0]
 * @since 2024-05-27
 */
@RequiredArgsConstructor(access = AccessLevel.PRIVATE)
public class JsonNodeUtil {
    /**
     * 根据路径找value
     *
     * @param path 路径
     * @param jsonNode jsonNode
     * @return value
     */
    public static Optional<JsonNode> findByPath(List<String> path, JsonNode jsonNode) {
        if (jsonNode == null) {
            return Optional.empty();
        }
        JsonNode root = jsonNode;
        for (String key : path) {
            JsonNode node = root.get(key);
            if (node == null) {
                return Optional.empty();
            }
            root = node;
        }
        return Optional.of(root);
    }
}
