package openbackup.system.base.common.utils.files;

import openbackup.system.base.security.exterattack.ExterAttack;

import org.apache.curator.shaded.com.google.common.collect.ImmutableList;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.io.BufferedInputStream;
import java.io.BufferedOutputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.List;

import javax.servlet.http.HttpServletResponse;

/**
 * 文件管理基类
 *
 * @author w00448845
 * @version [CDM Integrated machine]
 * @since 2019-11-11
 */
public abstract class AbstractFileUtil {
    /**
     * LEGO平台前台日志编码起始值，Tomcat/Servlet/Struts 等
     */
    public static final long MODULE_LEGO_FE = 0x000252000FB80000L;

    /**
     * The illegal char list.
     */
    protected static final List<String> SPECIAL_LIST = ImmutableList.of("=", "+", "-", "@");

    private static final Logger LOG = LoggerFactory.getLogger(AbstractFileUtil.class);

    /**
     * 题目
     */
    private String title;

    /**
     * 列名列表
     */
    private List<String> colTitle;

    /**
     * 文件路径加文件名
     */
    private String writeFileName;

    /**
     * 文件扩展名
     */
    private String strExtend;

    /**
     * 用户名
     */
    private String strUsrName;

    /**
     * 编码
     */
    private String strChar = "UTF-8";

    /**
     * 导出文件显示名称
     */
    private String downFileName = "ExportResult";

    /**
     * 设置文件管理类参数 <br>
     *
     * @param strTitle         文件题目
     * @param lstColTitle      文件列名
     * @param strWriteFileName 文件路径加文件名
     */
    public void fileSet(String strTitle, List<String> lstColTitle, String strWriteFileName) {
        setTitle(strTitle);
        setColTitle(lstColTitle);
        setWriteFileName(strWriteFileName + "." + strExtend);
    }

    /**
     * 下载导出结果
     *
     * @param response response
     */
    @ExterAttack
    public void downloadFile(HttpServletResponse response) {
        // 界面提示下载框 显示下载的文件名
        String displayName = downFileName + "." + strExtend;
        String downloadFileName = this.getWriteFileName();
        // 设置为下载application/x-download
        response.setContentType("application/octet-stream");
        response.setCharacterEncoding("windows-1251");
        // for ssl ie 下面导出出错，IE 下面 Pragma 為 no-cache
        response.setHeader("Pragma", "no-cache");
        response.setHeader("Cache-Control", "no-store, must-revalidate");
        response.addHeader("Content-Disposition", "attachment;filename=" + displayName);
        BufferedInputStream bis = null;
        BufferedOutputStream bos = null;
        File delFile = new File(downloadFileName);
        FileInputStream fileInput = null;

        try {
            fileInput = new FileInputStream(downloadFileName);

            bis = new BufferedInputStream(fileInput);
            bos = new BufferedOutputStream(response.getOutputStream());
            byte[] buff = new byte[bis.available()];
            int bytesRead;

            while ((bytesRead = bis.read(buff, 0, buff.length)) != -1) {
                bos.write(buff, 0, bytesRead);
            }
        } catch (IOException e) {
            LOG.error("downloadFile", MODULE_LEGO_FE);
        } finally {
            close(bis, bos, fileInput);
            // 删除下载文件
            boolean isSuccess = delFile.delete();

            if (!isSuccess) {
                LOG.error("delete file error", MODULE_LEGO_FE);
            }
        }
    }

    /**
     * 下载完文件后，关闭所有流
     *
     * @param bis bis
     * @param bos bos
     * @param fileInput fileInput
     */
    private static void close(BufferedInputStream bis, BufferedOutputStream bos, FileInputStream fileInput) {
        if (fileInput != null) {
            try {
                fileInput.close();
            } catch (IOException e) {
                LOG.error("downloadFile", MODULE_LEGO_FE);
            }
        }

        if (bis != null) {
            try {
                bis.close();
            } catch (IOException e) {
                LOG.error("downloadFile", MODULE_LEGO_FE);
            }
        }

        if (bos != null) {
            try {
                bos.close();
            } catch (IOException e) {
                LOG.error("downloadFile", MODULE_LEGO_FE);
            }
        }
    }

    /**
     * 获取当前时间字符串
     *
     * @return 当前时间字符串
     */
    public static String getNowTime() {
        Date date = new Date();
        SimpleDateFormat dateFormat = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss");
        return dateFormat.format(date);
    }

    /**
     * 写文件
     *
     * @param dataList 需要写入文件的数据
     */
    public void writeFile(List<List<String>> dataList) {
        formatDataList(dataList);
        writeToFile(dataList);
    }

    /**
     * 追加写文件内容 目前excel已经实现
     *
     * @param dataList 需要写入文件的数据
     */
    public void writeFileAppendContent(List<List<String>> dataList) {
        formatDataList(dataList);
        writeToFileAppendContent(dataList);
    }

    private void formatDataList(List<List<String>> dataList) {
        for (List<String> items : dataList) {
            for (int index = 0; index < items.size(); index++) {
                String item = items.get(index);
                if (item != null && SPECIAL_LIST.stream().anyMatch(item::contains)) {
                    items.set(index, encodeForCell(item));
                }
            }
        }
    }

    /**
     * Checks if is special char.
     *
     * @param value the value
     * @return true, if checks is special char
     */
    protected boolean isSpecialChar(String value) {
        return SPECIAL_LIST.contains(value);
    }

    /**
     * 对写文件内容格式化，避免命令注入
     *
     * @param item 需要写入文件的数据
     * @return 格式化好的数据
     */
    protected abstract String encodeForCell(String item);

    /**
     * 写文件
     *
     * @param dataLst 需要写入文件的数据
     */
    protected abstract void writeToFile(List<List<String>> dataLst);

    /**
     * 追加写文件内容
     *
     * @param dataLst 需要写入文件的数据
     */
    protected abstract void writeToFileAppendContent(List<List<String>> dataLst);

    /**
     * 关闭文件
     *
     * @throws IOException IOException
     */
    public abstract void closeFile() throws IOException;

    public String getTitle() {
        return title;
    }

    public void setTitle(String title) {
        this.title = title;
    }

    public List<String> getColTitle() {
        return colTitle;
    }

    public void setColTitle(List<String> colTitle) {
        this.colTitle = colTitle;
    }

    public String getWriteFileName() {
        return writeFileName;
    }

    public void setWriteFileName(String writeFileName) {
        this.writeFileName = writeFileName;
    }

    public void setStrExtend(String strExtend) {
        this.strExtend = strExtend;
    }

    public String getStrUsrName() {
        return strUsrName;
    }

    public String getStrChar() {
        return strChar;
    }

    public void setStrChar(String strChar) {
        this.strChar = strChar;
    }

    public void setDownFileName(String downFileName) {
        this.downFileName = downFileName;
    }
}
