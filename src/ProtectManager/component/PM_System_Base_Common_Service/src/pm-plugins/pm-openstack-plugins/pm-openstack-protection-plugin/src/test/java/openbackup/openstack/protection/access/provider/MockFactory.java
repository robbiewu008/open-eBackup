package openbackup.openstack.protection.access.provider;

import openbackup.data.protection.access.provider.sdk.base.Authentication;
import openbackup.data.protection.access.provider.sdk.resource.ActionResult;
import openbackup.data.protection.access.provider.sdk.resource.CheckReport;
import openbackup.data.protection.access.provider.sdk.resource.CheckResult;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.openstack.protection.access.constant.KeyStoneConstant;
import openbackup.openstack.protection.access.constant.OpenstackConstant;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.constants.Constants;
import openbackup.system.base.common.constants.TokenBo;

import org.powermock.api.mockito.PowerMockito;

import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.UUID;

/**
 * 通用mock工厂
 *
 * @author c30016231
 * @version [OceanProtect X8000 1.3.0]
 * @since 2023-01-10
 */
public class MockFactory {
    public static ProtectedEnvironment mockAgentResource() {
        ProtectedEnvironment agent = new ProtectedEnvironment();
        agent.setUuid(UUID.randomUUID().toString());
        agent.setEndpoint("127.0.0.1");
        agent.setPort(8888);
        return agent;
    }

    public static void mockTokenBo() {
        TokenBo.UserBo userBo = new TokenBo.UserBo();
        userBo.setId("user_id");
        userBo.setName("user_name");
        List<TokenBo.RoleBo> roles = Collections.singletonList(
            TokenBo.RoleBo.builder().name(Constants.Builtin.ROLE_DP_ADMIN).build());
        userBo.setRoles(roles);
        TokenBo buildToken = TokenBo.builder().user(userBo).build();
        PowerMockito.mockStatic(TokenBo.class);
        PowerMockito.when(TokenBo.get()).thenReturn(buildToken);
    }

    public static ProtectedEnvironment mockEnvironment() {
        ProtectedEnvironment environment = new ProtectedEnvironment();
        Map<String, List<ProtectedResource>> dependencies = new HashMap<>();
        environment.setDependencies(dependencies);
        environment.setUuid("env_id");
        environment.setName("env_name");
        environment.setEndpoint("https://keystone_url:443/identity/v3");
        Map<String, String> authExtendInfo = new HashMap<>();
        authExtendInfo.put(OpenstackConstant.CERTIFICATION, "cert content");
        authExtendInfo.put(OpenstackConstant.REVOCATION_LIST, "crl content");
        Authentication auth = new Authentication();
        auth.setAuthKey("test_key");
        auth.setAuthPwd("test_pwd");
        auth.setExtendInfo(authExtendInfo);
        environment.setAuth(auth);
        Map<String, String> extendInfo = new HashMap<>();
        extendInfo.put(OpenstackConstant.REGISTER_SERVICE, "0");
        extendInfo.put(OpenstackConstant.CERT_NAME, "cert1");
        extendInfo.put(OpenstackConstant.ENABLE_CERT, OpenstackConstant.CERT_ENABLE);
        extendInfo.put(OpenstackConstant.CERT_SIZE, "100");
        extendInfo.put(OpenstackConstant.CRL_NAME, "crl1");
        extendInfo.put(OpenstackConstant.CRL_SIZE, "100");
        extendInfo.put(KeyStoneConstant.CPS_IP, "1.1.1.1");
        environment.setExtendInfo(extendInfo);
        return environment;
    }

    public static List<CheckReport<Object>> buildCheckReport() {
        ActionResult actionResult = new ActionResult();
        actionResult.setCode(ActionResult.SUCCESS_CODE);
        actionResult.setMessage("SUCCESS_MSG");
        CheckResult<Object> checkResult = new CheckResult<>();
        checkResult.setResults(actionResult);

        ActionResult actionResult1 = new ActionResult();
        actionResult1.setCode(CommonErrorCode.AGENT_NETWORK_ERROR);
        CheckResult<Object> checkResult1 = new CheckResult<>();
        checkResult1.setResults(actionResult1);

        CheckReport<Object> checkReport = new CheckReport<>();
        List<CheckResult<Object>> results = new ArrayList<>();
        results.add(checkResult);
        results.add(checkResult1);
        checkReport.setResults(results);

        List<CheckReport<Object>> checkReports = new ArrayList<>();
        checkReports.add(checkReport);
        return checkReports;
    }

    public static ProtectedResource mockProtectedResource() {
        ProtectedResource resource = new ProtectedResource();
        resource.setEnvironment(new ProtectedEnvironment());
        resource.setUuid("resId1");
        resource.setRootUuid("envId1");
        resource.setParentUuid("projectId1");
        return resource;
    }
}
