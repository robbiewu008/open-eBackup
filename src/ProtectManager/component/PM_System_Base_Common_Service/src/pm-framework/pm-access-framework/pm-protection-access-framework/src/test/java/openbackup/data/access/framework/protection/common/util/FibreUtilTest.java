package openbackup.data.access.framework.protection.common.util;

import openbackup.data.access.framework.protection.common.util.FibreUtil;
import openbackup.data.protection.access.provider.sdk.base.Endpoint;

import org.junit.Assert;
import org.junit.Test;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

/**
 * 功能描述 fc配置工具类 测试类
 *
 * @author t30028453
 * @version [OceanProtect X8000 1.3.0]
 * @since 2023-02-24
 */
public class FibreUtilTest {
    /**
     * 用例场景：获取agent的id集合
     * 前置条件：无
     * 检查点：成功获取agent的id集合
     */
    @Test
    public void should_get_agent_id() {
        List<Endpoint> agents = new ArrayList<>();
        for (int i = 0; i < 3; i++) {
            Endpoint endpoint = new Endpoint();
            endpoint.setId("1");
            agents.add(endpoint);
        }
        List<String> ids = FibreUtil.getAgentIds(agents);
        for (String id : ids ) {
            Assert.assertTrue(id.equals("1"));
        }
    }

    /**
     * 用例场景：检查通过fc配置 获取 是否至少有一个agent的fc开关打开
     * 前置条件：fcConfigMap mock成功
     * 检查点：fcConfigMap中存在“1”的value时，hasOneLanFree方法返回true，否则返回false
     */
    @Test
    public void should_get_has_one_lanfree() {
        Map<String, String> fcConfigMap = getFcConfigMap();
        Assert.assertTrue(FibreUtil.hasOneLanFree(fcConfigMap));
        fcConfigMap.remove("123");
        Assert.assertFalse(FibreUtil.hasOneLanFree(fcConfigMap));
        fcConfigMap.remove("23");
        Assert.assertFalse(FibreUtil.hasOneLanFree(fcConfigMap));
        Assert.assertFalse(FibreUtil.hasOneLanFree(null));
    }

    private Map<String, String> getFcConfigMap() {
        Map<String, String> fcConfigMap = new HashMap<>();
        fcConfigMap.put("123", "true");
        fcConfigMap.put("23", "false");
        return fcConfigMap;
    }
}