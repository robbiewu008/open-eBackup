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

/**
 * 文件处理工厂类
 *
 */
public final class FileFactory {
    /**
     * 构造函数<br>
     * <br>
     *
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
