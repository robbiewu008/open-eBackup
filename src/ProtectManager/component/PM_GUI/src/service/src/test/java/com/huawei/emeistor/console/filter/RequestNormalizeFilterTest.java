package com.huawei.emeistor.console.filter;

import org.junit.Test;
import org.junit.runner.RunWith;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.boot.test.context.SpringBootTest;
import org.springframework.boot.test.mock.mockito.MockBean;
import org.springframework.mock.web.MockFilterChain;
import org.springframework.mock.web.MockHttpServletRequest;
import org.springframework.mock.web.MockHttpServletResponse;
import org.springframework.test.context.junit4.SpringRunner;

import javax.servlet.ServletException;
import java.io.IOException;

/**
 * URL标准化处理 单元测试
 *
 * @author t30028453
 * @version [OceanProtect DataBackup 1.3.0]
 * @since 2023-02-28
 */
@RunWith(SpringRunner.class)
@SpringBootTest(classes = {RequestNormalizeFilter.class})
public class RequestNormalizeFilterTest {
    @Autowired
    private MockHttpServletRequest request;

    @Autowired
    private MockHttpServletResponse response;

    @MockBean
    private MockFilterChain filterChain;

    /**
     * 用例场景：配置Filter成功
     * 前置条件：mock
     * 检查点：不报错
     */
    @Test
    public void should_do_internal_filter_successful() throws ServletException, IOException {
        // init
        RequestNormalizeFilter filter = new RequestNormalizeFilter();
        filter.doFilterInternal(request, response, filterChain);
    }
}