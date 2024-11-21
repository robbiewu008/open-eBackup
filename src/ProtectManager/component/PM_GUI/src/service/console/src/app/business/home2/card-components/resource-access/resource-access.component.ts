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
import { Component, OnInit, Input } from '@angular/core';
import { MenuItem } from '@iux/live';
import {
  ApiMultiClustersService,
  ResourceService,
  RouterUrl
} from 'app/shared';
import { DataMap } from 'app/shared/consts';
import { CookieService, I18NService } from 'app/shared/services';
import { AppUtilsService } from 'app/shared/services/app-utils.service';
import { Router } from '@angular/router';
import {
  assign,
  each,
  filter,
  find,
  get,
  includes,
  isArray,
  isEmpty,
  reduce
} from 'lodash';

@Component({
  selector: 'resource-access',
  templateUrl: './resource-access.component.html',
  styleUrls: ['./resource-access.component.less']
})
export class ResourceAccessComponent implements OnInit {
  @Input() cardInfo: any = {};
  resourceList = [];
  get filteredResourceList() {
    return this.resourceList.filter(item => item.count > 0);
  }
  get showResourceList() {
    return this.resourceList.slice(this.curPage * 4, (this.curPage + 1) * 4);
  }
  RouterUrl = RouterUrl;
  curPage = 0;
  resourceCount = 0;
  options: MenuItem[];
  resourceDetailUrl = ['/protection/summary'];
  resourceText = this.i18n.get('common_all_resource_label');
  protection = {
    protected: 0,
    unprotected: 0
  };
  resourceType = 'All';
  isMultiCluster = true;

  constructor(
    public router: Router,
    private i18n: I18NService,
    private resourceApiService: ResourceService,
    public cookieService: CookieService,
    private multiClustersServiceApi: ApiMultiClustersService,
    private appUtilsService: AppUtilsService
  ) {}

  ngOnInit() {
    this.getAllCusterShow();
    this.getProtectionStatus();
    this.resourceList = [
      {
        key: 'Database',
        label: this.i18n.get('common_database_label'),
        count: 0,
        subType: [...this.appUtilsService.getApplicationConfig().database],
        navigateParams: [RouterUrl.ProtectionHostAppOracle]
      },
      {
        key: 'BigData',
        label: this.i18n.get('common_bigdata_label'),
        count: 0,
        subType: [...this.appUtilsService.getApplicationConfig().bigData],
        navigateParams: [RouterUrl.ProtectionHostAppClickHouse]
      },
      {
        key: 'Virtualization',
        label: this.i18n.get('common_virtualization_label'),
        count: 0,
        subType: [
          ...this.appUtilsService.getApplicationConfig().virtualization
        ],
        navigateParams: [RouterUrl.ProtectionVirtualizationVmware]
      },
      {
        key: 'Container',
        label: this.i18n.get('common_container_label'),
        count: 0,
        subType: [...this.appUtilsService.getApplicationConfig().container],
        navigateParams: [RouterUrl.ProtectionVirtualizationKubernetesContainer]
      },
      {
        key: 'Cloud',
        label: this.i18n.get('common_huawei_clouds_label'),
        count: 0,
        subType: [...this.appUtilsService.getApplicationConfig().cloud],
        navigateParams: [RouterUrl.ProtectionCloudOpenstack]
      },
      {
        key: 'Application',
        label: this.i18n.get('common_application_label'),
        count: 0,
        subType: [...this.appUtilsService.getApplicationConfig().application],
        navigateParams: [RouterUrl.ProtectionActiveDirectory]
      },
      {
        key: 'FileSystem',
        label: this.i18n.get('common_file_system_label'),
        count: 0,
        subType: [...this.appUtilsService.getApplicationConfig().fileService],
        navigateParams: [RouterUrl.ProtectionStorageDeviceInfo]
      }
    ];
    this.formatResouceList();
  }

  handlePageChange({ pageIndex }) {
    this.curPage = pageIndex;
  }

  getAllCusterShow() {
    const clusterObj = JSON.parse(
      decodeURIComponent(this.cookieService.get('currentCluster'))
    );
    this.isMultiCluster =
      !clusterObj ||
      (clusterObj && clusterObj['icon'] === 'aui-icon-all-cluster');
  }

  getProtectionStatus(subType?) {
    this.cardInfo.loading = true;
    if (this.isMultiCluster) {
      this.handleMultiClusterCallback();
    } else {
      this.handleNotMultiClusterCallback(subType);
    }
  }

  handleMultiClusterCallback() {
    this.multiClustersServiceApi
      .getMultiClusterResources({
        akLoading: false,
        resourceType: this.resourceType
      })
      .subscribe(res => {
        this.resourceCount = res.protectedCount + res.unprotectedCount;
        res.resourceVoList.map(item => {
          let resouce = this.resourceList.find(resouce => {
            return resouce.key === item.resourceType;
          });
          if (resouce) {
            resouce.count = item.protectedCount + item.unprotectedCount;
          }
        });
        this.formatResouceList();
        this.cardInfo.loading = false;
      });
  }
  handleNotMultiClusterCallback(subType?) {
    const params = { akLoading: false };
    if (subType) {
      assign(params, {
        subType
      });
    }
    this.resourceApiService
      .summaryProtectionResourceV1ResourceProtectionSummaryGet(
        this.cookieService.isCloudBackup
          ? assign(params, {
              subType: [DataMap.Resource_Type.LocalFileSystem.value]
            })
          : params
      )
      .subscribe(res => {
        let summaryCon = 0;
        get(res, 'summary', []);
        each(this.resourceList, resource => {
          if (resource.subType && !isEmpty(resource.subType)) {
            const resourceCount = this.getResourceCount(resource, res);
            summaryCon += resourceCount;
            resource.count = resourceCount;
          }
        });
        this.formatResouceList();
        this.resourceCount = summaryCon;
        this.cardInfo.loading = false;
      });
  }

  getResourceCount(resource, res) {
    return reduce(
      resource.subType,
      (acc, app) => {
        if (isArray(app.key)) {
          const filterTmp = filter(res.summary, item =>
            includes(app.key, item.resource_sub_type)
          );
          return (
            acc +
            reduce(
              filterTmp,
              (count, i) => count + i.protected_count + i.unprotected_count,
              0
            )
          );
        } else {
          const findArr = find(
            res.summary,
            item => item.resource_sub_type === app.key
          );
          return (
            acc +
            (findArr ? findArr.protected_count + findArr.unprotected_count : 0)
          );
        }
      },
      0
    );
  }

  formatResouceList() {
    this.resourceList
      .sort((a, b) => {
        return b.count - a.count;
      })
      .forEach((item: any, index) => {
        item.clsName = `lv-color-chart${(index + 1)
          .toString()
          .padStart(2, '0')}`;
      });
  }

  navigate(navigateParams) {
    this.router.navigate(navigateParams);
  }
}
