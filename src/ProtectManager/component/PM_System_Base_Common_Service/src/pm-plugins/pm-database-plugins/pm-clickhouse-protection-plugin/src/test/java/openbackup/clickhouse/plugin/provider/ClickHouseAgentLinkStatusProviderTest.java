package openbackup.clickhouse.plugin.provider;

import static org.assertj.core.api.Assertions.assertThat;

import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;

import openbackup.system.base.sdk.resource.enums.LinkStatusEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import com.google.common.collect.Lists;

import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.powermock.modules.junit4.PowerMockRunner;

import java.util.List;

/**
 * 功能描述
 *
 * @author c00826511
 * @since 2023-08-14
 */
@RunWith(PowerMockRunner.class)
public class ClickHouseAgentLinkStatusProviderTest {
    private ClickHouseAgentLinkStatusProvider clickHouseAgentLinkStatusProvider;

    @Before
    public void init() {
        clickHouseAgentLinkStatusProvider = new ClickHouseAgentLinkStatusProvider();
    }

    /**
     * 用例场景：过滤
     * 前置条件：无
     * 检查点: 识别成功
     */
    @Test
    public void applicable_success() {
        ProtectedResource nodeResource1 = new ProtectedResource();
        nodeResource1.setSubType(ResourceSubTypeEnum.CLICK_HOUSE.getType());
        boolean applicable = clickHouseAgentLinkStatusProvider.applicable(nodeResource1);
        Assert.assertTrue(applicable);
    }

    /**
     * 用例场景：过滤
     * 前置条件：无
     * 检查点: 识别成功
     */
    @Test
    public void get_link_status_order_success() {
        List<LinkStatusEnum> list = clickHouseAgentLinkStatusProvider.getLinkStatusOrderList();
        List<LinkStatusEnum> expected = Lists.newArrayList(LinkStatusEnum.ONLINE, LinkStatusEnum.PARTLY_ONLING,
            LinkStatusEnum.OFFLINE);
        assertThat(list).usingRecursiveComparison().isEqualTo(expected);
    }
}
