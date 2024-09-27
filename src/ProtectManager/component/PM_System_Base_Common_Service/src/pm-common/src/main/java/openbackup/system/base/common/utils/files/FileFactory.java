/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2020. All rights reserved.
 */

package openbackup.system.base.common.utils.files;

/**
 * 文件处理工厂类
 *
 * @author 郝云峰 00004536
 * @version [Lego V100R002C10, 2014-12-18]
 * @since 2019-10-31
 */
public final class FileFactory {
    /**
     * 构造函数<br>
     * <br>
     *
     * @author 郝云峰 00004536
     * @since 2019-10-31  LEGO V1R1, Aug 30, 2010
     */
    private FileFactory() {
        super();
    }

    /**
     * 获取文件处理类
     * <br>
     *
     * @param type type
     * @return FileUtil
     * @author 郝云峰 00004536
     * @since 2019-10-31  Lego V1R1, Aug 30, 2010
     */
    public static AbstractFileUtil getFileUtil(String type) {
        AbstractFileUtil fileUtil = null;

        if ("excel".equals(type)) {
            fileUtil = new ExcelFileUtil();
        } else if ("csv".equals(type)) {
            fileUtil = new CsvFileUtil();
        } else {
            fileUtil = new ExcelFileUtil();
        }
        return fileUtil;
    }
}
