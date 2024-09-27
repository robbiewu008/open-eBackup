/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

package openbackup.data.access.framework.protection.controller.v2;

import org.springframework.boot.test.autoconfigure.OverrideAutoConfiguration;
import org.springframework.boot.test.autoconfigure.filter.TypeExcludeFilters;
import org.springframework.boot.test.autoconfigure.web.servlet.AutoConfigureMockMvc;
import org.springframework.boot.test.autoconfigure.web.servlet.AutoConfigureWebMvc;
import org.springframework.boot.test.autoconfigure.web.servlet.WebMvcTypeExcludeFilter;

/**
 * 功能描述
 *
 * @description:
 * @author: y00559272
 * @version: [OceanProtect A8000 1.1.0]
 * @since: 2021/11/30
 **/
@AutoConfigureWebMvc
@AutoConfigureMockMvc
@OverrideAutoConfiguration(enabled = false)
@TypeExcludeFilters(WebMvcTypeExcludeFilter.class)
public abstract class BaseControllerTest { }
