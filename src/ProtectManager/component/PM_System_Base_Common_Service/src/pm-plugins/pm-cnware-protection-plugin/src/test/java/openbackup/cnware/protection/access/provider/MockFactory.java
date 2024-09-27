package openbackup.cnware.protection.access.provider;

import openbackup.cnware.protection.access.constant.CnwareConstant;
import openbackup.data.protection.access.provider.sdk.base.Authentication;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;

import java.util.HashMap;
import java.util.List;
import java.util.Map;

/**
 * 功能描述 通用mock工厂
 *
 * @author q30048244
 * @version [OceanProtect DataBackup 1.6.0]
 * @since 2024-02-05 10:04
 */
public class MockFactory {

    public static ProtectedEnvironment mockEnvironment() {
        ProtectedEnvironment environment = new ProtectedEnvironment();
        Map<String, List<ProtectedResource>> dependencies = new HashMap<>();
        environment.setDependencies(dependencies);
        environment.setUuid("env_id");
        environment.setName("env_name");
        environment.setEndpoint("https://keystone_url:443/identity/v3");
        Map<String, String> authExtendInfo = new HashMap<>();
        authExtendInfo.put(CnwareConstant.CERTIFICATION, "cert content");
        authExtendInfo.put(CnwareConstant.REVOCATION_LIST, "crl content");
        Authentication auth = new Authentication();
        auth.setAuthKey("test_key");
        auth.setAuthPwd("test_pwd");
        auth.setExtendInfo(authExtendInfo);
        environment.setAuth(auth);
        Map<String, String> extendInfo = new HashMap<>();
        extendInfo.put(CnwareConstant.CERT_NAME, "cert1");
        extendInfo.put(CnwareConstant.ENABLE_CERT, CnwareConstant.ENABLE);
        extendInfo.put(CnwareConstant.CERT_SIZE, "100");
        extendInfo.put(CnwareConstant.CRL_NAME, "crl1");
        extendInfo.put(CnwareConstant.CRL_SIZE, "100");
        environment.setExtendInfo(extendInfo);
        return environment;
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
