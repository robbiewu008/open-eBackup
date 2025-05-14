/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

package com.huawei.oceanprotect.system.base.converter;

import static com.huawei.oceanprotect.system.base.converter.ConverterUtil.COMMA;
import static com.huawei.oceanprotect.system.base.converter.ConverterUtil.INITNETWORK_NUMBER;
import static com.huawei.oceanprotect.system.base.converter.ConverterUtil.NA;
import static com.huawei.oceanprotect.system.base.converter.ConverterUtil.SEMICOLON;
import static com.huawei.oceanprotect.system.base.converter.ConverterUtil.mutiNa;

import com.huawei.oceanprotect.system.base.initialize.network.common.NetworkExpansionBody;

import lombok.extern.slf4j.Slf4j;
import openbackup.system.base.common.aspect.DataConverter;
import openbackup.system.base.common.aspect.OperationLoggingList;

import org.springframework.stereotype.Component;

import java.util.Arrays;
import java.util.Collection;
import java.util.stream.Collectors;

/**
 * ArchiveNetwork Converter
 *
 * @author zwx1016945
 * @since 2021-07-07
 */
@Component
@Slf4j
public class ExpansionNetworkConverter implements DataConverter {
    /**
     * 转化名
     *
     * @return converter name
     */
    @Override
    public String getName() {
        return "expansionnetwork";
    }

    /**
     * 转化列表
     *
     * @param data data
     * @return 返回类型是Collection<List < String>>
     */
    @Override
    public Collection<?> convert(Collection<?> data) {
        return data.stream().map(this::handleData).collect(Collectors.toList());
    }

    /**
     * 处理数据
     *
     * @param item 进来的参数是
     * @return 返回一个指定的List，实现对应多个占位符
     */
    private <T> OperationLoggingList handleData(T item) {
        OperationLoggingList operationLoggingList = new OperationLoggingList();
        if (item instanceof NetworkExpansionBody) {
            NetworkExpansionBody params = (NetworkExpansionBody) item;
            int controller = params.getController();
            String backupPlane = params.getBackupPlane() == null ? NA : params.getBackupPlane()
                .stream()
                .map(obj -> obj == null ? NA : obj.getStartIp() + "-" + obj.getEndIp())
                .collect(Collectors.joining(SEMICOLON));
            String archivePlane = params.getArchivePlane() == null ? NA : params.getArchivePlane()
                .stream()
                .map(obj -> obj == null ? NA : obj.getStartIp() + "-" + obj.getEndIp())
                .collect(Collectors.joining(SEMICOLON));
            operationLoggingList.add(String.valueOf(controller));
            operationLoggingList.add(backupPlane);
            operationLoggingList.add(archivePlane);
        } else {
            operationLoggingList.addAll(Arrays.asList(mutiNa(INITNETWORK_NUMBER).split(COMMA)));
        }
        return operationLoggingList;
    }
}
