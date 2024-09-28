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
package openbackup.system.base.sdk.license;

/**
 * license check error code
 *
 */
public class ErrorCode {
    /**
     * license 未导入
     */
    public static final long LICENSE_NOT_IMPORT = 1677930497L;

    /**
     * license 已过期
     */
    public static final long LICENSE_EXPIRED = 1677930498L;

    /**
     * license文件解析失败
     */
    public static final long LICENSE_FILE_PARSE_ERROR = 1677930506L;

    /**
     * license文件信息校验不匹配
     */
    public static final long LICENSE_VERIFY_INFO_MISMATCH = 1677930507L;

    /**
     * 已用容量超过license容量
     */
    public static final long LICENSE_CAPACITY_NOT_ENOUGH = 1677930501L;

    /**
     * license文件格式错误
     */
    public static final long LICENSE_FILE_FORMAT_ERROR = 1677930504L;

    /**
     * license单个文件大于2m
     */
    public static final long LICENSE_FILE_OVER_SIZE = 1677930505L;

    /**
     * 不允许商用License转为测试License
     */
    public static final long NOT_ALLOW_COMMERCIAL_LICENSE_CONVERT_TO_TEST_LICENSE = 1677930518L;

    /**
     * 上传的华为license为空。
     */
    public static final long CYBER_ENGINE_HUAWEI_LICENSE_UPLOAD_EMPTY = 1677930513L;
}
