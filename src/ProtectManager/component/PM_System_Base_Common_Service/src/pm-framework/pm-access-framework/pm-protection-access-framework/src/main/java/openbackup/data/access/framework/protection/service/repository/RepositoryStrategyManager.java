package openbackup.data.access.framework.protection.service.repository;

import openbackup.data.access.framework.protection.service.repository.strategies.RepositoryStrategy;
import openbackup.data.protection.access.provider.sdk.enums.RepositoryProtocolEnum;

import com.google.common.base.CaseFormat;

import org.springframework.stereotype.Component;
import org.springframework.util.Assert;

import java.util.Locale;
import java.util.Map;

/**
 * 认证信息的策略管理器，后续不同协议
 *
 * @author: y00559272
 * @version: [OceanProtect A8000 1.1.0]
 * @since: 2021/12/8
 **/
@Component
public class RepositoryStrategyManager {
    /**
     * 认证策略后缀
     */
    private static final String STRATEGY_SUFFIX = "RepositoryStrategy";

    private final Map<String, RepositoryStrategy> strategyMap;

    public RepositoryStrategyManager(Map<String, RepositoryStrategy> strategyMap) {
        this.strategyMap = strategyMap;
    }

    /**
     * 根据协议枚举获取对应的认证策略类
     *
     * @param protocol 协议枚举对象
     * @return 认证策略类 {@code AuthStrategy}
     */
    public RepositoryStrategy getStrategy(RepositoryProtocolEnum protocol) {
        // 协议枚举名称大写下划线转小写驼峰风格
        String key = CaseFormat.UPPER_UNDERSCORE.to(CaseFormat.LOWER_CAMEL, protocol.name()) + STRATEGY_SUFFIX;
        Assert.isTrue(
                strategyMap.containsKey(key),
                String.format(Locale.ENGLISH, "protocol[%s] is not supported", protocol.name()));
        return strategyMap.get(key);
    }
}
