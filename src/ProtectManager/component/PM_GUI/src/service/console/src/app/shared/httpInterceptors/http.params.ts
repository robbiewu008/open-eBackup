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
export class HttpExtParams {
  // 统一URL前缀
  akPrefix: string | 'none';

  // 是否显示loading效果
  akLoading = true;

  // 超时时间，默认30分钟
  akTimeout: number = 30 * 60 * 1e3;

  // 是否使用公共异常
  akDoException = true;

  // 是否逃逸注销
  akEscapeSession = false;
}
