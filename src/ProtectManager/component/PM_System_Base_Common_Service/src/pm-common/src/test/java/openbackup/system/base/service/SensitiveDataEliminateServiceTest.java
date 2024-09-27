package openbackup.system.base.service;

import openbackup.system.base.common.constants.TokenBo;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.service.SensitiveDataEliminateService;

import org.junit.Assert;
import org.junit.Test;

import java.util.Arrays;
import java.util.Collections;
import java.util.List;

/**
 * Sensitive Data Eliminate Service Test
 *
 * @author l00272247
 * @since 2021-03-06
 */
public class SensitiveDataEliminateServiceTest {
    /**
     * 测试敏感信息擦除
     */
    @Test
    public void testEliminateJsonObject() {
        SensitiveDataEliminateService service = new SensitiveDataEliminateService();
        String pattern = "%password%";
        List<String> fields = Collections.singletonList(pattern);
        JSONObject data = new JSONObject().set("username", "username")
            .set("password", "xxx")
            .set("oldPassword", "111")
            .set("newPassword", "222");
        JSONObject json = new JSONObject().set("data", data);
        service.eliminate(json, fields);
        Assert.assertFalse(data.containsKey("oldPassword"));
        Assert.assertFalse(data.containsKey("newPassword"));
        Assert.assertFalse(data.containsKey("password"));
        Assert.assertTrue(data.containsKey("username"));
        String result = service.eliminate(
            "{\"request_id\":\"f04c5cf0-a09c-4748-8f74-7b13878dd29a\",\"default_publish_topic\":\"ScanRequest\",\"response_topic\":\"ScanResponse\",\"snap_info\":{\"snap_id\":\"0598e82e-f15e-4199-bf56-10023e1c8566\",\"snap_type\":\"live_mount\",\"timestamp\":\"1612169684000000\",\"resource_id\":\"50168b4b-6d33-c5a3-235d-d228592d5471\",\"resource_name\":\"wjx_test\",\"resource_type\":\"vim.virtualmachine\",\"chain_id\":\"ca150edb-b4e6-45ba-b52c-d27b56697e25@live-mount\",\"gn\":9,\"snap_metadata\":\"{\\\"disk_info\\\":[{\\\"DSMOREF\\\":\\\"datastore-12\\\",\\\"BUSNUMBER\\\":\\\"SCSI(0:0)\\\",\\\"GUID\\\":\\\"6000c29d-f2ae-debe-5b1f-eb6431710a3b\\\",\\\"NAME\\\":\\\"Hard disk 1\\\",\\\"SIZE\\\":\\\"5242880\\\",\\\"DISKDEVICENAME\\\":\\\"18292075090936696648\\\",\\\"DISKSNAPSHOTDEVICENAME\\\":\\\"9812185766535599782\\\"}],\\\"vmx_datastore\\\":{\\\"uuid\\\":\\\"datastore-12:600ecd64-393f77af-2b66-7079909e1abf\\\",\\\"name\\\":\\\"datastore1\\\",\\\"mo_id\\\":\\\"datastore-12\\\"},\\\"net_work\\\":[\\\"Network adapter 1\\\"],\\\"runtime\\\":{\\\"host\\\":{\\\"name\\\":\\\"8.40.35.63\\\",\\\"uuid\\\":\\\"68d839ca-f83e-11e6-81cc-707990acb175\\\",\\\"version\\\":\\\"6.7.0\\\",\\\"mo_id\\\":\\\"host-9\\\"}},\\\"hardware\\\":{\\\"num_cpu\\\":1,\\\"num_cores_per_socket\\\":1,\\\"memory\\\":2048}}\"},\"storage_info\":{\"ip\":\"8.40.99.114\",\"port\":\"8088\",\"password\":\"Admin@123\",\"username\":\"admin\",\"storage_type\":\"DORADO\",\"protocol\":\"NAS\"}}",
            pattern);
        Assert.assertFalse(result.contains("password"));
        Assert.assertTrue(result.contains("username"));
    }

    /**
     * 测试敏感信息擦除，不影响原来的数据
     */
    @Test
    public void testEliminateJsonObjectNoAffect() {
        SensitiveDataEliminateService service = new SensitiveDataEliminateService();
        List<String> fields = Arrays.asList("%pass%", "%pwd%", "%key%", "%crypto%", "%session%", "%token%",
            "%fingerprint%", "%auth%", "%enc%", "%dec%", "%tgt%", "%iqn%", "%initiator%", "%secret%", "%cert%",
            "%salt%", "%private%", "%verfiycode%", "%email%", "%phone%", "%rand%", "%safe%", "%user_info%", "%PKCS1%",
            "%base64%", "%AES128%", "%AES256%", "%RSA%", "%SHA1%", "%SHA256%", "%SHA384%", "%SHA512%", "%algorithm%",
            "%AccountNumber%", "%bank%", "%cvv%", "%checkno%", "%mima%", "%CardPinNumber%", "%IDNumber%", "ak", "iv",
            "mk");
        JSONObject data = new JSONObject().set("username", "username")
            .set("password", "xxx")
            .set("oldPassword", "111")
            .set("newPassword", "222")
            .set("pwd", "333")
            .set("privKey", "444")
            .set("crypto", "555")
            .set("sessionId", "666")
            .set("localToken", "777")
            .set("fingerprint", "888")
            .set("authInfo", "999")
            .set("encodeInfo", "000")
            .set("decodeInfo", "101010")
            .set("tgt", "111111")
            .set("iqnList", Collections.emptyList())
            .set("initiators", Collections.singleton("121212"))
            .set("secretInfo", "131313")
            .set("certInfo", "141414")
            .set("salt", "151515")
            .set("verfiycode", "161616")
            .set("MyEmail", "171717@huawei.com")
            .set("myPhone", "326544554")
            .set("randomNumber", "6532222")
            .set("safeNumber", "424454")
            .set("user_info", new TokenBo.UserInfo())
            .set("PKCS1", "5442142")
            .set("base64Str", "75632253")
            .set("AES128", "snanskcmnas")
            .set("AES256", "54321241321")
            .set("RSASalt", "454322323200")
            .set("sha1", "1121321")
            .set("sha256", "465223300")
            .set("SHa384", "453321212")
            .set("ShA512", "542132121")
            .set("algorithm", "jsklaskckals")
            .set("MyAccountNumber", "53454343")
            .set("bankInfo", "5643513212")
            .set("cvv", "454532131")
            .set("checkNo", "42331321")
            .set("mima", "5231212")
            .set("CardPinNumbers", "45321")
            .set("HisIDNumber", "snckascas")
            .set("ak", "4531331")
            .set("iv", "scklasmcas")
            .set("mk", "scaklscsa");
        JSONObject json = new JSONObject().set("data", data);
        JSONObject returnJson = service.eliminate(json, false, fields);
        Assert.assertFalse(((JSONObject) returnJson.get("data")).containsKey("password"));
        Assert.assertFalse(((JSONObject) returnJson.get("data")).containsKey("oldPassword"));
        Assert.assertFalse(((JSONObject) returnJson.get("data")).containsKey("newPassword"));
        Assert.assertFalse(((JSONObject) returnJson.get("data")).containsKey("pwd"));
        Assert.assertFalse(((JSONObject) returnJson.get("data")).containsKey("privKey"));
        Assert.assertFalse(((JSONObject) returnJson.get("data")).containsKey("crypto"));
        Assert.assertFalse(((JSONObject) returnJson.get("data")).containsKey("sessionId"));
        Assert.assertFalse(((JSONObject) returnJson.get("data")).containsKey("localToken"));
        Assert.assertFalse(((JSONObject) returnJson.get("data")).containsKey("fingerprint"));
        Assert.assertFalse(((JSONObject) returnJson.get("data")).containsKey("authInfo"));
        Assert.assertFalse(((JSONObject) returnJson.get("data")).containsKey("encodeInfo"));
        Assert.assertFalse(((JSONObject) returnJson.get("data")).containsKey("decodeInfo"));
        Assert.assertFalse(((JSONObject) returnJson.get("data")).containsKey("tgt"));
        Assert.assertFalse(((JSONObject) returnJson.get("data")).containsKey("iqnList"));
        Assert.assertFalse(((JSONObject) returnJson.get("data")).containsKey("initiators"));
        Assert.assertFalse(((JSONObject) returnJson.get("data")).containsKey("secretInfo"));
        Assert.assertFalse(((JSONObject) returnJson.get("data")).containsKey("certInfo"));
        Assert.assertFalse(((JSONObject) returnJson.get("data")).containsKey("salt"));
        Assert.assertFalse(((JSONObject) returnJson.get("data")).containsKey("verfiycode"));
        Assert.assertFalse(((JSONObject) returnJson.get("data")).containsKey("MyEmail"));
        Assert.assertFalse(((JSONObject) returnJson.get("data")).containsKey("myPhone"));
        Assert.assertFalse(((JSONObject) returnJson.get("data")).containsKey("randomNumber"));
        Assert.assertFalse(((JSONObject) returnJson.get("data")).containsKey("safeNumber"));
        Assert.assertFalse(((JSONObject) returnJson.get("data")).containsKey("user_info"));
        Assert.assertFalse(((JSONObject) returnJson.get("data")).containsKey("PKCS1"));
        Assert.assertFalse(((JSONObject) returnJson.get("data")).containsKey("base64Str"));
        Assert.assertFalse(((JSONObject) returnJson.get("data")).containsKey("AES128"));
        Assert.assertFalse(((JSONObject) returnJson.get("data")).containsKey("AES256"));
        Assert.assertFalse(((JSONObject) returnJson.get("data")).containsKey("RSASalt"));
        Assert.assertFalse(((JSONObject) returnJson.get("data")).containsKey("sha1"));
        Assert.assertFalse(((JSONObject) returnJson.get("data")).containsKey("sha256"));
        Assert.assertFalse(((JSONObject) returnJson.get("data")).containsKey("SHa384"));
        Assert.assertFalse(((JSONObject) returnJson.get("data")).containsKey("ShA512"));
        Assert.assertFalse(((JSONObject) returnJson.get("data")).containsKey("algorithm"));
        Assert.assertFalse(((JSONObject) returnJson.get("data")).containsKey("MyAccountNumber"));
        Assert.assertFalse(((JSONObject) returnJson.get("data")).containsKey("bankInfo"));
        Assert.assertFalse(((JSONObject) returnJson.get("data")).containsKey("cvv"));
        Assert.assertFalse(((JSONObject) returnJson.get("data")).containsKey("checkNo"));
        Assert.assertFalse(((JSONObject) returnJson.get("data")).containsKey("mima"));
        Assert.assertFalse(((JSONObject) returnJson.get("data")).containsKey("CardPinNumbers"));
        Assert.assertFalse(((JSONObject) returnJson.get("data")).containsKey("HisIDNumber"));
        Assert.assertFalse(((JSONObject) returnJson.get("data")).containsKey("ak"));
        Assert.assertFalse(((JSONObject) returnJson.get("data")).containsKey("mk"));
        Assert.assertFalse(((JSONObject) returnJson.get("data")).containsKey("iv"));
        Assert.assertTrue(((JSONObject) json.get("data")).containsKey("password"));
        Assert.assertTrue(((JSONObject) json.get("data")).containsKey("oldPassword"));
        Assert.assertTrue(((JSONObject) json.get("data")).containsKey("newPassword"));
    }
}
