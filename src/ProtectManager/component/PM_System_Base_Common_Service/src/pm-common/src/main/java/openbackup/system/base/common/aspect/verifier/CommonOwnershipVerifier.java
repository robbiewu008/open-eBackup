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
package openbackup.system.base.common.aspect.verifier;

import openbackup.system.base.common.aspect.DomainBasedOwnershipVerifier;
import openbackup.system.base.common.constants.TokenBo;
import openbackup.system.base.common.utils.VerifyUtil;

import java.util.List;
import java.util.function.BiConsumer;

/**
 * Common Ownership Verifier
 *
 */
public class CommonOwnershipVerifier implements DomainBasedOwnershipVerifier {
    private final String type;

    private final BiConsumer<String, List<String>> verifier;

    /**
     * constructor
     *
     * @param type     type
     * @param verifier verifier
     */
    public CommonOwnershipVerifier(String type, BiConsumer<String, List<String>> verifier) {
        this.type = type;
        this.verifier = verifier;
    }

    /**
     * verifier type
     *
     * @return verifier type
     */
    @Override
    public String getType() {
        return type;
    }

    /**
     * verify
     *
     * @param userBo    user bo
     * @param resources resource uuid list
     */
    @Override
    public void verify(TokenBo.UserBo userBo, List<String> resources) {
        if (VerifyUtil.isEmpty(resources)) {
            return;
        }
        verifier.accept(userBo.getId(), resources);
    }
}
