package openbackup.openstack.protection.access.common;

import openbackup.data.protection.access.provider.sdk.base.PageListResponse;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceConstants;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.openstack.protection.access.common.OpenstackCommonService;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import org.junit.Assert;
import org.junit.BeforeClass;
import org.junit.Test;
import org.mockito.Mockito;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

/**
 * @author t30049904
 * @version [OceanProtect DataBack 1.5.0]
 * @since 2023/9/19
 */
public class OpenstackCommonServiceTest {
    private static OpenstackCommonService openstackCommonService;
    private static final ResourceService resourceService = Mockito.mock(ResourceService .class);
    @BeforeClass
    public static void init(){
        openstackCommonService = new OpenstackCommonService(resourceService);
    }
    @Test
    public void testCheckMaxNum() {
        ProtectedResource protectedResource = new ProtectedResource();
        List<ProtectedResource> records = new ArrayList<>();
        records.add(protectedResource);
        PageListResponse<ProtectedResource> response= new PageListResponse<>();
        response.setTotalCount(256);
        response.setRecords(records);
        Map<String, Object> conditions = new HashMap<>();
        conditions.put("subType", Arrays.asList(ResourceSubTypeEnum.OPENSTACK_DOMAIN.getType(),
                ResourceSubTypeEnum.OPENSTACK_CONTAINER.getType()));
        conditions.put("sourceType", ResourceConstants.SOURCE_TYPE_REGISTER);
        Mockito.when(resourceService.query(0, 1,conditions)).thenReturn(response);
        LegoCheckedException exception = Assert.assertThrows(LegoCheckedException.class,
                () -> openstackCommonService.checkOpenStackAndDomainMaxNum());
        Assert.assertEquals(CommonErrorCode.ENV_COUNT_OVER_LIMIT, exception.getErrorCode());
    }
}