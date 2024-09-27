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
#ifndef APPPLUGIN_NAS_APPSERVICE_H
#define APPPLUGIN_NAS_APPSERVICE_H
#include "ApplicationService.h"

namespace AppServiceExport {


 /**
     * query application list by one apptype in the host, no authentication, synchronization function
     * .eg query the oracle database list in the host
     * appType: query application type
     *
     * @param appType
    */
    void DiscoverApplications(std::vector<Application>& returnValue, const std::string& appType);

    /**
     * check application exist with authentication, synchronization function
     * application: check application information
     * auth: application authentication
     *
     * @param application
     * @param auth
    */
    void CheckApplication(
        ActionResult& returnValue, const ApplicationEnvironment& appEnv, const Application& application);

    /**
     * list appliation resource, synchronization function
     * application: check application information
     * parentResource: query child resource of the parentResource
     * auth: application authentication
     *
     * @param application
     * @param parentResource
     * @param auth
    */
    void ListApplicationResource(
        std::vector<ApplicationResource>& returnValue, const ApplicationEnvironment& appEnv,
        const Application& application, const ApplicationResource& parentResource);
     /**
    * Function description
    *     list appliation resource by page, synchronization function, .eg query data file of oracle database
    * Parameters
    *     appEnv : protect application environment
    *     application : check application information
    *     parentResource : query child resource of the parentResource
    *     condition : query conditions
    * Return value
    *     application resource list in the host, list is empty when no application resource exists
    *
    * @param appEnv
    * @param application
    * @param parentResource
    * @param condition
    */
    void ListApplicationResourceV2(std::vector<ApplicationResource>& returnValue,
        const ApplicationEnvironment& appEnv,
        const Application& application, const ApplicationResource& parentResource,
        const QueryByPage& condition);
}

#endif // APPPLUGIN_NAS_APPSERVICE_H