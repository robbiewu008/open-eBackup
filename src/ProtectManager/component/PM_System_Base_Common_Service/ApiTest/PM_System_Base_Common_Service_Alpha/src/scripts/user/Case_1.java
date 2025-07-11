/*
* This file is a part of the open-eBackup project.
* This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
* If a copy of the MPL was not distributed with this file, You can obtain one at
* http://mozilla.org/MPL/2.0/.
*
* Copyright (c) [2024] Huawei Technologies Co.,Ltd.
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*/
package scripts.user;

import java.io.PrintWriter;
import java.io.StringWriter;
import java.math.BigDecimal;
import java.net.URLEncoder;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Date;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.concurrent.Callable;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.Future;
import java.util.concurrent.ThreadFactory;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.atomic.AtomicInteger;
import org.testng.annotations.AfterMethod;
import org.testng.annotations.BeforeMethod;
import org.testng.annotations.Test;
import org.testng.Assert;
import org.testng.asserts.SoftAssert;
import com.alibaba.fastjson.JSON;
import com.alibaba.fastjson.JSONArray;
import com.alibaba.fastjson.JSONObject;
import com.huawei.hutaf.apitest.aw.AWResponse;
import com.huawei.hutaf.apitest.aw.Config;
import com.huawei.hutaf.apitest.aw.HttpClient;
import com.huawei.hutaf.apitest.aw.JSONHelper;
import com.huawei.hutaf.apitest.aw.FunctionSet;
import com.huawei.hutaf.apitest.aw.VariableMapper;
import com.huawei.hutaf.apitest.aw.Reporter;
import com.huawei.hutaf.apitest.aw.FileUtil;
import com.huawei.hutaf.apitest.aw.DynamicGlobalVariable;

public class Case_1 {

    private Map<String,List<String> > m_commonHeader=new HashMap<String,List<String> >();

    private VariableMapper localVariable = new VariableMapper();

    private Throwable tearDownThrowable = null;

    private String IGNORE_CHECKPOINT = "!###!IgnoreCheckPoint!###!";

    DynamicGlobalVariable dynamicGlobalVariable = new DynamicGlobalVariable();

    Case_1 () {
    }

    @BeforeMethod
    public void setUp() throws Throwable {
        try{
        localVariable.put("TESTCASE_HEADER", m_commonHeader);
        localVariable.put("TESTCASE_URI", "00csdv16ig11");
        localVariable.put("TESTCASE_NAME", "TestGetToken");
        localVariable.put("TESTCASE_NUMBER", "1");
        } catch (Exception e) {
        Reporter.log("1#ExceptionLog#" + getExceptionStackTrace(e), true);
        throw e;}

        try{
        Reporter.log("1#AWProgress#0/1", true);Reporter.log("1#AWProgressCount#0", true);
        Reporter.log("0#begin#1. genTokenUsingPOST", true);
        long aw1ExeTime = new Date().getTime();
        JSONObject aw1param1 = new JSONObject();
        aw1param1.put("authType", "DORADO");
        aw1param1.put("password", "Huawei@123");
        aw1param1.put("userName", "sysadmin");
        AWResponse awp1 =genTokenUsingPOST(Config.getValue("host_test"), aw1param1.toString());
        try{dynamicGlobalVariable.setString("Token",String.valueOf(JSONHelper.getJSONValue(awp1.responseBody, "$body.token", Object.class).toString()));}catch(Throwable e){}
        Reporter.log("1#AWVariable#" + "Token=" + dynamicGlobalVariable.getString("Token"), true);
        Reporter.log("1#AWProgress#1/1", true);Reporter.log("1#AWProgressCount#100", true);
        Reporter.log("0#end#" + (new Date().getTime() - aw1ExeTime) + " ms", true);

        } catch (Throwable e) {
        Reporter.log("1#ExceptionLog#" + getExceptionStackTrace(e), true);
        throw e;}
    
    }

    @Test
    public void test() throws Throwable {
        try{
        } catch (Throwable e) {
        Reporter.log("1#ExceptionLog#" + getExceptionStackTrace(e), true);
        throw e;}
            
    }

    @AfterMethod(alwaysRun = true)
    public void tearDown() throws Throwable {
        if (tearDownThrowable != null) {
        Throwable e = tearDownThrowable;
        Reporter.log("1#ExceptionLog#" + getExceptionStackTrace(e), true);
        throw e;}

    }        


    // aw methods
    public AWResponse genTokenUsingPOST(String hostURL,String authRequest) throws Exception
    {
        HttpClient.Response y_response;
        String y_responseBody = null;
        AWResponse y_result=new AWResponse();

        String y_destURL;
        String y_baseURL=String.format("http://%s/v1/auth/token",hostURL);
        if(hostURL.startsWith("http")&&y_baseURL.startsWith("https://")){
            y_baseURL = y_baseURL.substring(8);
        }else if(hostURL.startsWith("http")&&y_baseURL.startsWith("http://")){
            y_baseURL= y_baseURL.substring(7);
        }
        y_destURL=y_baseURL;

        Reporter.log("1#Time#" + FunctionSet.getCurrentDate("yyyy-MM-dd HH:mm:ss SSS"), true);
        Reporter.log("1#Url#" + y_destURL, true);
        Reporter.log("1#Method#post", true);

        //设置RequestHeader的代码片段
        Map<String,List<String> > y_reqHeader=new HashMap<String,List<String> >();
        if(!m_commonHeader.isEmpty())y_reqHeader.putAll(m_commonHeader);
        Reporter.log("1#RequestHeader#" + y_reqHeader, true);

         //设置RequestBody的代码片段
        String y_jsonRequestBody=authRequest;
        Reporter.log("1#RequestBody#" + y_jsonRequestBody, true);

        // 执行HTTP请求
         y_response = HttpClient.post(y_destURL, y_jsonRequestBody, y_reqHeader);

        int y_httpStatus =y_response.getCode();
        y_result.setReturnCode(y_httpStatus);
        y_responseBody=y_response.asString();
                //new String(y_response.getBody());
        y_result.setResponseBody(y_responseBody);
        y_result.setHeaders(y_response.getHeaders());
        Reporter.log("1#ResponseCode#" + y_httpStatus, true);
        Reporter.log("1#ResponseHeader#" + y_response.getHeaders(), true);
        Reporter.log("1#ResponseBody#" + y_responseBody, true);


        Reporter.log("1#EndTime#" + FunctionSet.getCurrentDate("yyyy-MM-dd HH:mm:ss SSS"), true);
        return y_result;
    }



    private static String getExceptionStackTrace(Throwable throwable) {
            final StringWriter sw = new StringWriter();
            final PrintWriter pw = new PrintWriter(sw, true);
            throwable.printStackTrace(pw);
            return sw.getBuffer().toString();
     }
}
