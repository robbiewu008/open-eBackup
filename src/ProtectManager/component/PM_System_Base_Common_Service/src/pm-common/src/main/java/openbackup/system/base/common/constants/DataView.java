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
package openbackup.system.base.common.constants;

/**
 * Json Data View
 *
 * @author l00272247
 * @since 2019-11-28
 */
public class DataView {
    /**
     * 普通信息
     */
    public static class Normal {
        private Normal() {
        }
    }

    /**
     * 用于Email、电话号码等敏感信息
     */
    public static class Sensitive extends Normal {
        private Sensitive() {
        }
    }

    /**
     * 用于密码、Token等重要敏感信息
     */
    public static class Confidential extends Sensitive {
        private Confidential() {
        }
    }

    /**
     * 用于个人隐私等信息
     */
    public static class Personal extends Confidential {
        private Personal() {
        }
    }
}
