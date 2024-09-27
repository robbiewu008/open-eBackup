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
package openbackup.system.base.common.aspect;

import openbackup.system.base.common.constants.TokenBo;

import java.util.List;

/**
 * resource domain based verifier
 *
 * @author l00272247
 * @since 2020-11-14
 */
public interface DomainBasedOwnershipVerifier {
    /**
     * verifier type
     *
     * @return verifier type
     */
    String getType();

    /**
     * verify
     *
     * @param userBo    user bo
     * @param resources resource uuid list
     */
    void verify(TokenBo.UserBo userBo, List<String> resources);
}
