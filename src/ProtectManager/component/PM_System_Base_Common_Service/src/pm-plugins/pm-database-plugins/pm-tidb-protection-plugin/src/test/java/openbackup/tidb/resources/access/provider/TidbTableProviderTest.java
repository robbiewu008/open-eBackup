package openbackup.tidb.resources.access.provider;

import static org.mockito.ArgumentMatchers.anyString;
import static org.powermock.api.mockito.PowerMockito.when;

import openbackup.access.framework.resource.validator.JsonSchemaValidator;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.system.base.common.utils.json.JsonUtil;
import openbackup.tidb.resources.access.service.TidbService;

import org.apache.commons.io.FileUtils;
import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mock;
import org.powermock.modules.junit4.PowerMockRunner;

import java.io.File;
import java.io.IOException;
import java.net.URL;
import java.nio.charset.Charset;
import java.util.Objects;

/**
 * TidbTableProviderTest
 *
 * @author w00426202
 * @since 2023-07-15
 */
@RunWith(PowerMockRunner.class)
public class TidbTableProviderTest {
    @Mock
    private TidbService tidbService;

    private TidbTableProvider tidbTableProvider;

    @Mock
    private JsonSchemaValidator jsonSchemaValidator;

    @Before
    public void setUp() {
        tidbTableProvider = new TidbTableProvider(tidbService, jsonSchemaValidator);
    }

    /**
     * 用例场景：测试类型判断
     * 前置条件：
     * 检查点：返回false
     */
    @Test
    public void test_applicable() {
        ProtectedResource resource = new ProtectedResource();
        Assert.assertFalse(tidbTableProvider.applicable(resource));
    }

    @Test
    public void test_beforeCreate() throws IOException {
        ProtectedResource protectedResource = getTableReqFromJsonFile();
        when(tidbService.getResourceByCondition(anyString())).thenReturn(protectedResource);
        tidbTableProvider.beforeCreate(protectedResource);
    }

    @Test
    public void test_beforeUpdate() throws IOException {
        ProtectedResource protectedResource = getTableReqFromJsonFile();
        when(tidbService.getResourceByCondition(anyString())).thenReturn(protectedResource);
        tidbTableProvider.beforeCreate(protectedResource);
    }

    public ProtectedResource getTableReqFromJsonFile() throws IOException {
        URL resource = Thread.currentThread().getContextClassLoader().getResource("Table-req.json");
        File file = new File(Objects.requireNonNull(resource).getPath());
        String masterReqStr = FileUtils.readFileToString(file, Charset.defaultCharset());
        ProtectedResource protectedResource = JsonUtil.read(masterReqStr, ProtectedEnvironment.class);
        return protectedResource;
    }
}
