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
package openbackup.system.base.common.utils.files;

import openbackup.system.base.common.utils.ExceptionUtil;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.security.exterattack.ExterAttack;

import org.apache.commons.csv.CSVFormat;
import org.apache.commons.csv.CSVParser;
import org.apache.commons.csv.CSVPrinter;
import org.apache.commons.csv.CSVRecord;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.OutputStreamWriter;
import java.io.PrintWriter;
import java.io.Reader;
import java.nio.charset.Charset;
import java.util.List;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

/**
 * csv文件处理类
 *
 * @author 郝云峰 00004536
 * @version [Lego V100R002C10, 2014-12-18]
 * @since 2014-12-18
 */
public class CsvFileUtil extends AbstractFileUtil {
    private static final String SIGN_COMMA = ",";

    private static final Logger LOG = LoggerFactory.getLogger(CsvFileUtil.class);

    private static final Pattern QUOTATION_MARKS_PATTERN = Pattern.compile("\"");

    private CSVPrinter csvPrinter;

    /**
     * 构造函数<br>
     * <br>
     *
     * @author 郝云峰 00004536
     * @since Lego V1R1, Aug 30, 2010
     */
    public CsvFileUtil() {
        super();
        this.setStrExtend("csv");
    }

    /**
     * 关闭文件 覆盖方法/实现方法(选择其一)<br>
     * <br>
     *
     * @throws IOException IOException
     * @author 郝云峰 00004536
     * @since Lego V1R1, Aug 30, 2010
     */
    @Override
    public void closeFile() throws IOException {
        csvPrinter.close();
    }

    /**
     * Encode for csv cell.
     *
     * <pre>
     * =2+5   转换后内容  "  =2+5"    <br>
     * "=2+5  转换后内容  """=2+5"     <br>
     * ,=2+5  转换后内容  ",=2+5"      <br>
     * ",=2+5 转换后内容  """,=2+5"    <br>
     * =2+5,  转换后内容  "  =2+5,"     <br>
     * </pre>
     *
     * @param value the value
     * @return the string
     */
    @Override
    protected String encodeForCell(final String value) {
        if (VerifyUtil.isEmpty(value)) {
            return value;
        }

        String valueTemp = value;
        if (!isSpecialChar(valueTemp.substring(0, 1))) {
            return valueTemp;
        }
        valueTemp = "\t" + valueTemp;

        StringBuffer encodedValue = new StringBuffer();
        encodedValue.append('\"');
        Matcher matcher = QUOTATION_MARKS_PATTERN.matcher(valueTemp);
        while (matcher.find()) {
            matcher.appendReplacement(encodedValue, "\"\"");
        }
        matcher.appendTail(encodedValue);
        encodedValue.append('\"');
        return encodedValue.toString();
    }

    private void closeStream(PrintWriter pw, OutputStreamWriter osw, FileOutputStream fos) {
        if (pw != null) {
            pw.close();
        }
        if (osw != null) {
            try {
                osw.close();
            } catch (IOException e) {
                LOG.error("error.", ExceptionUtil.getErrorMessage(e));
            }
        }
        if (fos != null) {
            try {
                fos.close();
            } catch (IOException e) {
                LOG.error("error.", ExceptionUtil.getErrorMessage(e));
            }
        }
    }

    /**
     * 读取csv行数
     *
     * @param file File
     * @return int
     * @author q50016449
     * @since 2020-09-04
     */
    @ExterAttack
    public static int readCsv(File file) {
        int count = 0;
        InputStream is = null;
        InputStreamReader isr = null;
        Reader reader = null;
        try {
            is = new FileInputStream(file);
            isr = new InputStreamReader(is, "GBK");
            reader = new BufferedReader(isr);
            CSVParser parser = CSVFormat.DEFAULT.withHeader(SIGN_COMMA).parse(reader);
            List<CSVRecord> lists = parser.getRecords();
            for (CSVRecord record : lists) {
                count++;
            }
            parser.close();
            return count;
        } catch (IOException e) {
            LOG.error("error.", ExceptionUtil.getErrorMessage(e));
            return 0;
        } finally {
            if (is != null) {
                try {
                    is.close(); // 关闭流
                } catch (IOException e) {
                    LOG.debug("inputStream close IOException.", ExceptionUtil.getErrorMessage(e));
                }
            }
            if (isr != null) {
                try {
                    isr.close(); // 关闭流
                } catch (IOException e) {
                    LOG.debug("inputStream close IOException.", ExceptionUtil.getErrorMessage(e));
                }
            }
            if (reader != null) {
                try {
                    reader.close(); // 关闭流
                } catch (IOException e) {
                    LOG.debug("inputStream close IOException.", ExceptionUtil.getErrorMessage(e));
                }
            }
        }
    }

    @Override
    protected void writeToFile(List<List<String>> dataLst) {
        FileOutputStream fos = null;
        OutputStreamWriter osw = null;
        PrintWriter pw = null;
        CSVFormat format = CSVFormat.DEFAULT.withHeader(SIGN_COMMA).withSkipHeaderRecord();
        try {
            fos = new FileOutputStream(this.getWriteFileName());
            osw = new OutputStreamWriter(fos, Charset.forName(this.getStrChar()));
            pw = new PrintWriter(osw);

            csvPrinter = new CSVPrinter(pw, format);

            int index = 0;

            String[] arrTemps = new String[this.getColTitle().size()];
            csvPrinter.printRecord(new String[] {this.getStrUsrName(), getNowTime()});
            for (String strTemp : this.getColTitle()) {
                arrTemps[index++] = strTemp;
            }

            csvPrinter.printRecord(arrTemps);
            for (List<String> lstStr : dataLst) {
                csvPrinter.printRecord(lstStr);
            }
            csvPrinter.flush();
        } catch (IOException e) {
            LOG.error("error: ", ExceptionUtil.getErrorMessage(e));
        } finally {
            try {
                if (csvPrinter != null) {
                    csvPrinter.close();
                }
            } catch (Exception e) {
                LOG.error("closer writer failed, errMsg.", ExceptionUtil.getErrorMessage(e));
            }
            closeStream(pw, osw, fos);
        }
    }

    @Override
    protected void writeToFileAppendContent(List<List<String>> dataLst) {
    }
}
