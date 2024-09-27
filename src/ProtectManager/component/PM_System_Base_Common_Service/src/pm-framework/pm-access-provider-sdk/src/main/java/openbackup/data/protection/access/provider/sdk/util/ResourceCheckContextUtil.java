/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

package openbackup.data.protection.access.provider.sdk.util;

import openbackup.data.protection.access.provider.sdk.resource.ActionResult;
import openbackup.data.protection.access.provider.sdk.resource.ResourceCheckContext;
import openbackup.system.base.common.exception.LegoCheckedException;

import java.util.Collections;
import java.util.List;
import java.util.Optional;
import java.util.stream.Collectors;

/**
 * Resource Check Context Util
 *
 * @author l00650874
 * @since 2022-06-29
 */
public class ResourceCheckContextUtil {
    /**
     * UNION ERROR
     */
    public static final long UNION_ERROR = -1000000000;

    private ResourceCheckContextUtil() {
    }

    /**
     * validate resource check context
     *
     * @param context context
     * @param message message
     */
    public static void check(ResourceCheckContext context, String message) {
        List<ActionResult> results = Optional.ofNullable(context.getActionResults())
            .orElseGet(Collections::emptyList)
            .stream()
            .filter(result -> result.getCode() != ActionResult.SUCCESS_CODE)
            .collect(Collectors.toList());
        if (results.isEmpty()) {
            return;
        }
        String[] params = results.stream().map(ResourceCheckContextUtil::cast).toArray(String[]::new);
        throw new LegoCheckedException(UNION_ERROR, params, message);
    }

    /**
     * 检查结果是否成功。 有任何检查结果不为success则为失败
     *
     * @param actionResultList actionResultList
     * @return true:成功， false:失败
     */
    public static boolean isSuccess(List<ActionResult> actionResultList) {
        List<ActionResult> results = Optional.ofNullable(actionResultList)
            .orElseGet(Collections::emptyList)
            .stream()
            .filter(result -> result.getCode() != ActionResult.SUCCESS_CODE)
            .collect(Collectors.toList());
        return results.isEmpty();
    }

    private static String cast(ActionResult result) {
        String[] params = Optional.ofNullable(result.getDetailParams())
            .map(e -> e.toArray(new String[0]))
            .orElse(new String[0]);
        return new LegoCheckedException(result.getCode(), params, result.getMessage()).i18n();
    }
}
