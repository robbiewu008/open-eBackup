package com.huawei.oceanprotect.system.base.converter;

import com.huawei.oceanprotect.system.base.initialize.network.common.ExpansionIpSegment;
import com.huawei.oceanprotect.system.base.initialize.network.common.NetworkExpansionBody;

import org.junit.Assert;
import org.junit.Test;

import java.io.IOException;
import java.util.ArrayList;
import java.util.Collection;

/**
 * Converter Test
 */
public class ConverterTest {

    @Test
    public void expansionNetworkConverterTest() {
        ExpansionIpSegment backupPlane = new ExpansionIpSegment();
        backupPlane.setStartIp("192.168.132.100");
        backupPlane.setEndIp("192.168.132.150");
        ExpansionIpSegment backupPlane1 = new ExpansionIpSegment();
        backupPlane1.setStartIp("192.168.132.250");
        backupPlane1.setEndIp("192.168.132.254");
        ArrayList<ExpansionIpSegment> backupPlanes = new ArrayList<>();
        backupPlanes.add(backupPlane);
        backupPlanes.add(backupPlane1);
        ExpansionIpSegment archivePlane = new ExpansionIpSegment();
        archivePlane.setStartIp("192.168.132.50");
        archivePlane.setEndIp("192.168.132.99");
        ArrayList<ExpansionIpSegment> archivePlanes = new ArrayList<>();
        archivePlanes.add(archivePlane);
        NetworkExpansionBody networkExpansionBody = new NetworkExpansionBody();
        networkExpansionBody.setBackupPlane(backupPlanes);
        networkExpansionBody.setArchivePlane(archivePlanes);
        networkExpansionBody.setController(2);
        ExpansionNetworkConverter expansionNetworkConverter = new ExpansionNetworkConverter();
        ArrayList<NetworkExpansionBody> networkExpansionBodies = new ArrayList<>();
        networkExpansionBodies.add(networkExpansionBody);
        Collection<?> convert = expansionNetworkConverter.convert(networkExpansionBodies);
        Assert.assertEquals(
            "[[2, 192.168.132.100-192.168.132.150; 192.168.132.250-192.168.132.254, 192.168.132.50-192.168.132.99]]",
            convert.toString());
    }

    @Test
    public void expansionNetworkConverterTest1111() throws IOException {
        String key = "A.IOM0.P0".substring(0, "A.IOM0.P0".length() - 2);
        String value = "A.IOM0.P0".substring("A.IOM0.P0".length() - 2);
        System.out.println(key);
        System.out.println(value);
        // FileInputStream fileInputStream = new FileInputStream("D:\\APIData\\1.5.0\\支持前后端口统一\\LLD_Design.xls");
        // HSSFWorkbook workBook = new HSSFWorkbook(fileInputStream);
        // HSSFSheet sheet = workBook.getSheetAt(11);
        // String sheetName = sheet.getSheetName();
        //
        // HSSFRow row = sheet.getRow(3);
        //
        // HSSFCell cell = row.getCell(2);
        // String stringCellValue = cell.getStringCellValue();
        // System.out.println(stringCellValue);
        // if (!cell.getStringCellValue().contains("X6000&X8000")) {
        //     // 报错
        //     System.out.println(cell.getStringCellValue());
        // }
        //
        // List<ExcelNetworkConfigBean> excelNetworkConfigBeanList = new ArrayList<>();
        // for (int i = 3; i < 9; i++) {
        //     HSSFRow row1 = sheet.getRow(i);
        //     if (row1.getCell(4).getStringCellValue().isEmpty()) {
        //         continue;
        //     }
        //     excelNetworkConfigBeanList.add(ExcelNetworkConfigType.updateToNetworkConfigBean(row1));
        // }
        // for (ExcelNetworkConfigBean excelNetworkConfigBean : excelNetworkConfigBeanList) {
        //     System.out.println(excelNetworkConfigBean);
        // }
        //
        // List<ExcelNetworkConfigBean> excelNetworkConfigBeanListX3000 = new ArrayList<>();
        // for (int i = 9; i < 12; i++) {
        //     HSSFRow row1 = sheet.getRow(i);
        //     if (row1.getCell(4).getStringCellValue().isEmpty()) {
        //         continue;
        //     }
        //     excelNetworkConfigBeanListX3000.add(ExcelNetworkConfigType.updateToNetworkConfigBean(row1));
        // }
        // for (ExcelNetworkConfigBean excelNetworkConfigBean : excelNetworkConfigBeanListX3000) {
        //     System.out.println(excelNetworkConfigBean);
        // }
    }

    private int getIpByHex(String hex) {
        String[] split = hex.split("\\.");
        int num =0;
        for (String s : split) {
            String s1 = Integer.toBinaryString(Integer.parseInt(s));
            char[] chars = s1.toCharArray();
            for (char aChar : chars) {
                if (aChar == '0') {
                    break;
                }
                num++;
            }
        }
        return num;
    }
}
