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
package openbackup.system.base.util;

import openbackup.system.base.common.utils.files.AbstractFileUtil;
import openbackup.system.base.common.utils.files.FileFactory;

import com.google.common.collect.ImmutableList;

import org.apache.poi.hssf.usermodel.HSSFWorkbook;
import org.junit.Assert;
import org.junit.Test;
import org.springframework.mock.web.MockHttpServletResponse;

import java.io.ByteArrayInputStream;
import java.io.InputStream;
import java.nio.charset.StandardCharsets;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.List;

/**
 * The AbstractFileUtilTest
 *
 */
public class AbstractFileUtilTest {
    @Test
    public void download_excel_success() throws Exception {
        AbstractFileUtil fileUtil = FileFactory.getFileUtil("excel");
        List<String> columns = ImmutableList.of("Alarm ID", "Alarm Name", "Alarm Desc");
        fileUtil.fileSet("", columns, "mock_test_file");
        List<List<String>> dataList = new ArrayList<>();
        dataList.add(Arrays.asList("0x000803220001", "0x000803220001.alarm.name", "0x000803220001.alarm.desc"));
        dataList.add(Arrays.asList("0x2064032B0001", "0x2064032B0001.alarm.name", "0x2064032B0001.alarm.desc"));
        dataList.add(Arrays.asList("0x2064032B0002", "=1+3", "0x2064032B0002.alarm.desc"));
        fileUtil.writeFile(dataList);
        fileUtil.closeFile();
        List<List<String>> appendDataList = new ArrayList<>();
        appendDataList.add(Arrays.asList("0x000803220004", "0x000803220004.alarm.name", "0x000803220004.alarm.desc"));
        appendDataList.add(Arrays.asList("0x2064032B0005", "0x2064032B0005.alarm.name", "0x2064032B0005.alarm.desc"));
        appendDataList.add(Arrays.asList("0x2064032B0006", "0x2064032B0006.alarm.nam", "0x2064032B0006.alarm.desc"));
        fileUtil.writeFileAppendContent(appendDataList);
        fileUtil.closeFile();
        MockHttpServletResponse mockHttpServletResponse = new MockHttpServletResponse();
        fileUtil.downloadFile(mockHttpServletResponse);

        try (InputStream contentInStream = new ByteArrayInputStream(mockHttpServletResponse.getContentAsByteArray());
            HSSFWorkbook resultExcel = new HSSFWorkbook(contentInStream)) {
            Assert.assertEquals(0, resultExcel.getSheetAt(0).getFirstRowNum());
            Assert.assertEquals(7, resultExcel.getSheetAt(0).getLastRowNum());
            Assert.assertEquals(3, resultExcel.getSheetAt(0).getRow(1).getLastCellNum());
            Assert.assertEquals("\t=1+3", resultExcel.getSheetAt(0).getRow(4).getCell(1).toString());
        }
    }

    @Test
    public void download_csv_success() throws Exception {
        AbstractFileUtil fileUtil = FileFactory.getFileUtil("csv");
        List<String> columns = ImmutableList.of("Alarm ID", "Alarm Name", "Alarm Desc");
        fileUtil.fileSet("", columns, "mock_test_file");
        List<List<String>> dataList = new ArrayList<>();
        dataList.add(Arrays.asList("0x000803220001", "0x000803220001.alarm.name", "0x000803220001.alarm.desc"));
        dataList.add(Arrays.asList("0x2064032B0001", "0x2064032B0001.alarm.name", "0x2064032B0001.alarm.desc"));
        dataList.add(Arrays.asList("0x2064032B0002", "=1+3", "0x2064032B0002.alarm.desc"));
        fileUtil.writeFile(dataList);
        fileUtil.closeFile();
        MockHttpServletResponse mockHttpServletResponse = new MockHttpServletResponse();
        fileUtil.downloadFile(mockHttpServletResponse);

        String content = mockHttpServletResponse.getContentAsString(StandardCharsets.UTF_8);
        String[] split = content.split("\n");
        Assert.assertEquals(5, split.length);

        String[] split1 = split[4].split(",");
        Assert.assertEquals(3, split1.length);
        Assert.assertEquals("\"\"\"\t=1+3\"\"\"", split1[1]);
    }

    @Test
    public void write_excel_file_with_keep_index() {
        AbstractFileUtil fileUtil = FileFactory.getFileUtil("excel");
        List<List<String>> dataList = new ArrayList<>();
        dataList.add(Arrays.asList("12345", "12345", "12345"));
        dataList.add(Arrays.asList("12345", "12345", "12345"));
        dataList.add(Arrays.asList("12345", "12345", "12345"));
        fileUtil.writeFile(dataList, Collections.singleton(5));
    }
}
