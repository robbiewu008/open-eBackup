package openbackup.tidb.resources.access.restore;

import openbackup.data.access.framework.agent.DefaultProtectAgentSelector;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.tidb.resources.access.provider.TidbAgentProvider;
import openbackup.tidb.resources.access.service.TidbService;

import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mock;
import org.powermock.modules.junit4.PowerMockRunner;

/**
 * TidbTableRestoreInterceptorTest
 *
 * @author w00426202
 * @since 2023-07-27
 */
@RunWith(PowerMockRunner.class)
public class TidbTableRestoreInterceptorTest {
    @Mock
    private TidbService tidbService;

    @Mock
    private CopyRestApi copyRestApi;

    private TidbTableRestoreInterceptor tidbTableRestoreInterceptor;

    @Mock
    private ResourceService resourceService;

    @Mock
    private DefaultProtectAgentSelector defaultSelector;

    @Before
    public void setUp() {
        tidbTableRestoreInterceptor = new TidbTableRestoreInterceptor(tidbService, new TidbAgentProvider(tidbService),
            copyRestApi, resourceService, defaultSelector);
    }

    @Test
    public void applicable_test() {
        Assert.assertTrue(tidbTableRestoreInterceptor.applicable(ResourceSubTypeEnum.TIDB_TABLE.getType()));
    }
}
