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
package openbackup.system.base.sdk.cluster.request;

import lombok.Data;

import java.util.ArrayList;
import java.util.List;

/**
 * RecentJobQueryReq
 *
 */
@Data
public class RecentJobQueryReq {
    private List<String> statusList = new ArrayList<>();

    private int startPage = 0;

    private int pageSize = 10;

    private String orderType = "desc";

    private String orderBy = "start_time";

    private boolean isVisible = true;

    private boolean isSystem = false;
}
