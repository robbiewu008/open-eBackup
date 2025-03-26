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
#ifndef APPLICATION_SERVICE_IMP_H
#define APPLICATION_SERVICE_IMP_H

#include "ApplicationService.h"

#ifdef WIN32
#include "define/Defines.h"

class AGENT_API ApplicationServiceImp;
#endif

#ifndef EXTER_ATTACK
#define EXTER_ATTACK
#endif

class ApplicationServiceImp : public AppProtect::ApplicationServiceIf {
public:
    ApplicationServiceImp();
    ~ApplicationServiceImp();

    /**
     * query application list by one apptype in the host, no authentication, synchronization function
     * .eg query the oracle database list in the host
     * appType: query application type
     *
     * @param appType
    */
    EXTER_ATTACK virtual void DiscoverApplications(
        std::vector<Application>& returnValue, const std::string& appType) override;

    /**
     * check application exist with authentication, synchronization function
     * application: check application information
     * auth: application authentication
     *
     * @param application
     * @param auth
    */
    EXTER_ATTACK virtual void CheckApplication(
        ActionResult& returnValue, const ApplicationEnvironment& appEnv, const Application& application) override;

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
    EXTER_ATTACK virtual void ListApplicationResource(
        std::vector<ApplicationResource>& returnValue, const ApplicationEnvironment& appEnv,
        const Application& application, const ApplicationResource& parentResource) override;
    /**
    * Function description
    *     list appliation resource by page, synchronization function, .eg query data file of database
    * Parameters
    *     request : list resource request
    * Return value
    *     application resource list in the host with one page, list is empty when no application resource exists
    *
    * @param request
    */
    EXTER_ATTACK virtual void ListApplicationResourceV2(
        AppProtect::ResourceResultByPage& page, const AppProtect::ListResourceRequest& request) override;
    /**
    * Function description
    *     query application information
    * Parameters
    *     request : ListResourceRequest struct （can be find in thrift file）
    * Return value
    *     returnValue return value
    *
    * @param request
    */
    EXTER_ATTACK void AsyncListApplicationResource(
        ActionResult& returnValue, const AppProtect::ListResourceRequest& request);
    /**
    * Function description
    *     query host cluster information
    * Parameters
    *     appEnv : application environment information which is cluster member
    * Return value
    *     remote host list, list is empty when no application exists
    *
    * @param appEnv
    */
    EXTER_ATTACK virtual void DiscoverHostCluster(
        ApplicationEnvironment& returnEnv, const ApplicationEnvironment& appEnv) override;
    /**
    * Function description
    *     query application cluster information
    * Parameters
    *     appEnv : application environment information which is cluster member
    *     application : application information
    * Return value
    *     remote host list, list is empty when no application exists
    *
    * @param appEnv
    * @param application
    */
    EXTER_ATTACK virtual void DiscoverAppCluster(
        ApplicationEnvironment& returnEnv,
        const ApplicationEnvironment& appEnv,
        const Application& application) override;
    /**
    * Function description
    *     list support resource info
    * Parameters
    *     fillResourceReq : query resource request
    * Return value
    *     list of supported resources
    *
    * @param script
    */
    EXTER_ATTACK virtual void ListApplicationConfig(
        std::map<std::string, std::string>& resources, const std::string& script) override;
		
    /**
     * Function description
     *     For Oracle services, check the archive area of the database.
     * Parameters
     *    appType : query application type
     *    dbInfoList : oracle db info list
     * Return value
     *     successful if the ActionResult.code is 0
     *     failed ActionResult.message=dbname1&dbname2,80
     *
     * @param appType
    */
    EXTER_ATTACK virtual void OracleCheckArchiveArea(ActionResult& _return,
        const std::string& appType, const std::vector<AppProtect::OracleDBInfo>& dbInfoList) override;

    /**
     * Function description
     *     When remove resource protect policy, application would do something
     * Parameters
     *     appEnv : protect application environment
     *     application : check application information
     * Return value
     *     successful if the ActionResult.code is 0, failed otherwise
     *
     * @param appEnv
     * @param application
    */
    EXTER_ATTACK virtual void RemoveProtect(ActionResult& returnValue,
        const ApplicationEnvironment& appEnv, const Application& application) override;

    EXTER_ATTACK virtual void FinalizeClear(ActionResult& _return, const AppProtect::ApplicationEnvironment& appEnv,
        const AppProtect::Application& application, const std::map<std::string, std::string>& extendInfo) override;
};

#endif // _APPLICATION_SERVICE_IMP_H_