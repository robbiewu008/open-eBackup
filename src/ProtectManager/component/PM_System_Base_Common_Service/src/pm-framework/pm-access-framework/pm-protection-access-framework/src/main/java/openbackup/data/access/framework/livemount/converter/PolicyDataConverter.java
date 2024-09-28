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
package openbackup.data.access.framework.livemount.converter;

import openbackup.data.access.framework.livemount.dao.LiveMountPolicyEntityDao;
import openbackup.system.base.common.aspect.DataConverter;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Component;

import java.util.Collection;
import java.util.List;
import java.util.stream.Collectors;

/**
 * Live Mount Policy Converter
 *
 */
@Component
public class PolicyDataConverter implements DataConverter {
    @Autowired
    private LiveMountPolicyEntityDao liveMountPolicyEntityDao;

    /**
     * converter name
     *
     * @return converter name
     */
    @Override
    public String getName() {
        return "live_mount_policy";
    }

    /**
     * convert data
     *
     * @param data data
     * @return result
     */
    @Override
    public Collection<?> convert(Collection<?> data) {
        List<String> list =
            data.stream().map(item -> item != null ? item.toString() : null).collect(Collectors.toList());
        return liveMountPolicyEntityDao.selectBatchIds(list);
    }
}
