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

import io.jsonwebtoken.lang.Strings;
import openbackup.system.base.common.utils.ExceptionUtil;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.security.exterattack.ExterAttack;

import org.apache.poi.hssf.usermodel.HSSFCell;
import org.apache.poi.hssf.usermodel.HSSFCellStyle;
import org.apache.poi.hssf.usermodel.HSSFFont;
import org.apache.poi.hssf.usermodel.HSSFRow;
import org.apache.poi.hssf.usermodel.HSSFSheet;
import org.apache.poi.hssf.usermodel.HSSFWorkbook;
import org.apache.poi.hssf.util.HSSFColor;
import org.apache.poi.ss.usermodel.CellType;
import org.apache.poi.ss.usermodel.Sheet;
import org.apache.poi.ss.usermodel.Workbook;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.util.ArrayList;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

/**
 * Excel文件管理类
 *
 */
public class ExcelFileUtil extends AbstractFileUtil {
    /**
     * excel cell 的位置
     */
    private static final int NUM_2 = 2;

    private static final int COLUMNWIDTH = 8000;

    private static final int MAX_EXCEL_CELL_LENGTH = 32767;

    private static final Logger LOG = LoggerFactory.getLogger(ExcelFileUtil.class);

    /**
     * 创建新的Excel工作簿
     */
    private HSSFWorkbook workbook = new HSSFWorkbook();

    /* 输出文件流 */
    private FileOutputStream writeFileOut;

    /**
     * 构造函数<br>
     * <br>
     *
     */
    ExcelFileUtil() {
        super();
        this.setStrExtend("xls");
    }

    /**
     * Encode for excel cell.
     *
     * <pre>
     * =2+5   转换后内容    =2+5    <br>
     * "=2+5  转换后内容  "=2+5     <br>
     * ,=2+5  转换后内容  ,=2+5      <br>
     * ",=2+5 转换后内容  ",=2+5    <br>
     * =2+5,  转换后内容    =2+5,     <br>
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
        if (isSpecialChar(valueTemp.substring(0, 1))) {
            valueTemp = "\t" + valueTemp;
        }
        return valueTemp;
    }

    /**
     * 写文件
     *
     * @param dataLst 需要写入文件的数据
     */
    @Override
    protected void writeToFile(List<List<String>> dataLst) {
        this.writeToFile(dataLst, new HashSet<>());
    }

    /**
     * 写文件
     *
     * @param dataLst 需要写入文件的数据
     * @param keepIndexSet 截取后需要保留的列号集合
     */
    @Override
    protected void writeToFile(List<List<String>> dataLst, Set<Integer> keepIndexSet) {
        if (this.getWriteFileName() == null) {
            return;
        }

        try {
            writeFileOut = new FileOutputStream(this.getWriteFileName());
        } catch (FileNotFoundException e) {
            LOG.error("file not found , write file name: {}.", getWriteFileName());
            return;
        }
        int num = this.getColTitle().size();

        int rowNum = 0;
        // 在Excel工作簿中建一工作表，其名为缺省值
        HSSFSheet sheet = workbook.createSheet();

        if (this.getTitle() != null && !"".equals(this.getTitle())) {
            // 写入TITLE
            HSSFRow rowTitle = sheet.createRow(rowNum);
            HSSFCell cellTitle = rowTitle.createCell((short) (num / NUM_2));
            // 写入用户和时间
            cellTitle.setCellType(CellType.STRING);
            cellTitle.setCellValue(this.getTitle());
            rowNum++;
        }
        HSSFRow rowColUsr = sheet.createRow(rowNum);
        HSSFCell cellUsr = rowColUsr.createCell((short) 0);
        cellUsr.setCellValue(this.getStrUsrName());
        HSSFCell cellTime = rowColUsr.createCell((short) 1);
        cellTime.setCellValue(getNowTime());
        rowNum++;
        // 写入每列TITLE
        HSSFRow rowColTitle = sheet.createRow(rowNum);

        for (short i = 0; i < num; i++) {
            HSSFCell cellColTitle = rowColTitle.createCell(i);
            cellColTitle.setCellType(CellType.STRING);
            cellColTitle.setCellValue(this.getColTitle().get(i));
        }
        // 设置文字颜色
        HSSFFont fontRed = workbook.createFont();
        fontRed.setColor(HSSFColor.HSSFColorPredefined.RED.getIndex());
        HSSFFont fontGreen = workbook.createFont();
        fontGreen.setColor(HSSFColor.HSSFColorPredefined.GREEN.getIndex());
        // 设置样式
        HSSFCellStyle styleGreen = workbook.createCellStyle();
        HSSFCellStyle styleRed = workbook.createCellStyle();
        styleGreen.setFont(fontGreen);
        styleRed.setFont(fontRed);
        // 写入内容,从第三行开始写入
        rowNum++;
        int rowNumOfSheet = rowNum;
        for (List<String> lstStr : dataLst) {
            // 在索引rowNumOfSheet的位置创建行
            List<List<String>> rowLists = processRow(lstStr, keepIndexSet);
            for (List<String> rowStr : rowLists) {
                HSSFRow row = sheet.createRow(rowNumOfSheet);
                processCell(rowStr, row, sheet);
                rowNumOfSheet++;
            }
        }
    }

    /**
     * 每个cell最多只能容纳32767个字符，超过则需要拆分为多行
     *
     * @param lstStr 需要处理的行
     * @param keepIndexSet 截取后需要保留的列号集合
     * @return res
     */
    private List<List<String>> processRow(List<String> lstStr, Set<Integer> keepIndexSet) {
        List<List<String>> res = new ArrayList<>();
        boolean cellSplited;
        do {
            List<String> canInsertToExcelRow = new ArrayList<>();
            cellSplited = isCellSplit(lstStr, canInsertToExcelRow, keepIndexSet);
            res.add(canInsertToExcelRow);
        } while (cellSplited);
        return res;
    }

    private boolean isCellSplit(List<String> lstStr, List<String> canInsertToExcelRow, Set<Integer> keepIndexSet) {
        boolean cellSplited = false;
        for (int cellNumOfRow = 0; cellNumOfRow < lstStr.size(); cellNumOfRow++) {
            if (lstStr.get(cellNumOfRow) != null && lstStr.get(cellNumOfRow).length() > MAX_EXCEL_CELL_LENGTH) {
                canInsertToExcelRow.add(lstStr.get(cellNumOfRow).substring(0, MAX_EXCEL_CELL_LENGTH));
                lstStr.set(cellNumOfRow, lstStr.get(cellNumOfRow).substring(MAX_EXCEL_CELL_LENGTH));
                cellSplited = true;
            } else {
                canInsertToExcelRow.add(lstStr.get(cellNumOfRow));
                if (!keepIndexSet.contains(cellNumOfRow)) {
                    lstStr.set(cellNumOfRow, Strings.EMPTY);
                }
            }
        }
        return cellSplited;
    }

    @Override
    protected void writeToFileAppendContent(List<List<String>> dataLst) {
        try (FileInputStream input = new FileInputStream(getWriteFileName())) {
            workbook = new HSSFWorkbook(input);

            // 获取第一个 sheet
            HSSFSheet sheet = workbook.getSheetAt(0);

            // 获取最后一行的行号
            int lastRowNum = sheet.getLastRowNum();
            int rowNumOfSheet = lastRowNum + 1;
            for (List<String> lstStr : dataLst) {
                // 在索引rowNumOfSheet的位置创建行
                HSSFRow row = sheet.createRow(rowNumOfSheet);
                processCell(lstStr, row, sheet);
                rowNumOfSheet++;
            }
        } catch (IOException e) {
            LOG.error("file not found , write file name: {}.", getWriteFileName(), ExceptionUtil.getErrorMessage(e));
        }
        try {
            writeFileOut = new FileOutputStream(getWriteFileName());
        } catch (FileNotFoundException e) {
            LOG.error("file not found , write file name: {}.", getWriteFileName());
        }
    }

    private void processCell(List<String> lstStr, HSSFRow row, HSSFSheet sheet) {
        for (short cellNumOfRow = 0; cellNumOfRow < lstStr.size(); cellNumOfRow++) {
            // 在索引cellNumOfRow的位置创建单元格
            HSSFCell cell = row.createCell(cellNumOfRow);
            // 定义单元格为字符串类型
            cell.setCellType(CellType.STRING);
            sheet.setColumnWidth(cellNumOfRow, (short) COLUMNWIDTH);
            // 在单元格中输入内容
            if (lstStr.get(cellNumOfRow) != null) {
                cell.setCellValue(lstStr.get(cellNumOfRow));
            } else {
                // 必须写入一个空值,否则将出现不可预知的情况
                cell.setCellValue("");
            }
        }
    }

    /**
     * 覆盖方法/实现方法(选择其一)<br>
     * <br>
     *
     * @throws IOException IOException
     */
    @Override
    public void closeFile() throws IOException {
        // 把相应的Excel 工作簿存盘
        workbook.write(this.getWriteFileOut());
        this.getWriteFileOut().flush();
        this.getWriteFileOut().close();
    }

    /**
     * 读取EXCEL行数
     *
     * @param file File
     * @return int
     */
    @ExterAttack
    public static int readExcel(File file) {
        try (FileInputStream fs = new FileInputStream(file);
            Workbook hwb = new HSSFWorkbook(fs)) {
            Sheet sheet = hwb.getSheetAt(0);
            int begin = sheet.getFirstRowNum();
            int end = sheet.getLastRowNum();
            int count = 0;
            for (int i = begin; i <= end; i++) {
                if (sheet.getRow(i) == null) {
                    continue;
                }
                count++;
            }
            return count;
        } catch (IOException e) {
            LOG.error("Exception: ", ExceptionUtil.getErrorMessage(e));
        }
        return 0;
    }

    private FileOutputStream getWriteFileOut() {
        return writeFileOut;
    }
}
