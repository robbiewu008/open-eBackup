package openbackup.system.base.common.utils;

import static org.junit.Assert.*;

import openbackup.system.base.common.utils.JSONObject;

import org.junit.Test;

/**
 * JSONObject的测试用例集合.
 *
 * @author y00559272
 * @version [OceanProtect A8000 1.1.0]
 * @since 2022/4/16
 **/
public class JSONObjectTest {
    /**
     * 用例名称：验证合法json字符串成功<br/>
     * 前置条件：无<br/>
     * check点：方法返回值为true<br/>
     */
    @Test
    public void should_return_true_when_isValidJson_given_json_str() {
        String json = "{'status': 'SUCCESS', 'progress': 100, 'endTime': 1650016211924}";
        assertTrue(JSONObject.isValidJson(json));
    }

    /**
     * 用例名称：验证合法json字符串失败<br/>
     * 前置条件：无<br/>
     * check点：方法返回值为false<br/>
     */
    @Test
    public void should_return_flase_when_isValidJson_given_not_json_str() {
        String json = "{fabsd,gdfd,3545646,gfgf";
        assertFalse(JSONObject.isValidJson(json));
    }
}
