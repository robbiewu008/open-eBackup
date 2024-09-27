/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
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
