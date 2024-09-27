package openbackup.data.access.framework.protection.common.util;

import openbackup.data.access.framework.protection.common.constants.FcConstants;
import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.enums.ClientProtocolTypeEnum;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.common.utils.VerifyUtil;

import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Objects;
import java.util.Optional;
import java.util.stream.Collectors;

/**
 * 功能描述 fc配置工具类
 *
 * @author t30028453
 * @version [OceanProtect X8000 1.3.0]
 * @since 2023-02-14
 */
public class FibreUtil {
    /**
     * 通过fc配置 获取 是否至少有一个agent的fc开关打开
     *
     * @param fcConfigMap fc配置
     * @return 是否至少有一个agent的fc开关打开
     */
    public static boolean hasOneLanFree(Map<String, String> fcConfigMap) {
        return Optional.ofNullable(fcConfigMap)
            .map(map -> !map.values()
                .stream()
                .filter(Objects::nonNull)
                .filter(isAddedFc -> isAddedFc.equals(String.valueOf(true)))
                .collect(Collectors.toList())
                .isEmpty())
            .orElse(false);
    }

    /**
     * 获取agent的id集合
     *
     * @param agents agents
     * @return agent的id集合
     */
    public static List<String> getAgentIds(List<Endpoint> agents) {
        return Optional.ofNullable(agents)
            .map(endpoints -> endpoints.stream()
                .filter(Objects::nonNull)
                .map(Endpoint::getId)
                .collect(Collectors.toList()))
            .orElse(null);
    }

    /**
     * 获取客户端协议类型
     *
     * @param fcMap fc配置
     * @return 客户端协议, 1-DATA_TURBO协议，0-IP协议
     */
    public static Integer getClientProtocol(Map<String, String> fcMap) {
        // 只要有一个agent支持，返回DATA_TURBO协议标志，否则返回IP协议标志
        return hasOneLanFree(fcMap)
            ? ClientProtocolTypeEnum.DATA_TURBO.getClientProtocolType()
            : ClientProtocolTypeEnum.IP.getClientProtocolType();
    }

    /**
     * 获取lanFree agent信息
     *
     * @param fcMap fc配置
     * @return agent信息map形式：{"fibreChannel":{"b00869a5-719f-404d-a2ce-fdb1cb3ca765":"true"}}
     */
    public static Map<String, String> getLanFreeAgents(Map<String, String> fcMap) {
        Map<String, String> param = new HashMap<>();
        Optional.ofNullable(fcMap)
            .filter(map -> !VerifyUtil.isEmpty(map))
            .ifPresent(map -> param.put(FcConstants.FIBRE_CHANNEL, JSONObject.stringify(map)));
        return param;
    }
}
