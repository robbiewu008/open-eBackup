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
package openbackup.access.framework.resource.service.kerberos;

import openbackup.data.protection.access.provider.sdk.base.PageListResponse;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.constants.IsmNumberConstant;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.sdk.kerberos.KerberosListener;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Service;

import java.util.HashMap;
import java.util.Map;

/**
 * kerberos监听服务实现
 *
 * @author c30016231
 * @since 2021-12-01
 */
@Service
public class KerberosListenerImpl implements KerberosListener {
    @Autowired
    private ResourceService resourceService;

    @Override
    public void beforeDelete(String kerberosId) {
        Map<String, Object> condition = new HashMap<>();
        condition.put("kerberosId", kerberosId);
        PageListResponse<ProtectedResource> response = resourceService.query(IsmNumberConstant.ZERO,
                IsmNumberConstant.ONE, condition);
        if (response.getTotalCount() != IsmNumberConstant.ZERO) {
            String resourceId = response.getRecords().get(0).getUuid();
            throw new LegoCheckedException(CommonErrorCode.KERBEROS_IS_IN_USE, new String[]{resourceId},
                    "kerberos in use");
        }
    }
}
