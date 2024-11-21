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
import { Component, OnInit } from '@angular/core';
import { Router } from '@angular/router';
import {
  CommonConsts,
  CookieService,
  ResourceService,
  RouterUrl,
  SUB_APP_REFRESH_FLAG
} from 'app/shared';
import { DataMap } from 'app/shared/consts/data-map.config';
import { AppUtilsService } from 'app/shared/services/app-utils.service';
import { I18NService } from 'app/shared/services/i18n.service';
import { ResourceCatalogsService } from 'app/shared/services/resource-catalogs.service';
import {
  assign,
  defer,
  each,
  filter,
  find,
  first,
  includes,
  isArray,
  isEmpty
} from 'lodash';
import { combineLatest } from 'rxjs';

@Component({
  selector: 'aui-summary',
  templateUrl: './summary.component.html',
  styleUrls: ['./summary.component.less']
})
export class SummaryComponent implements OnInit {
  activeIndex = 'all';
  resourceType = [];
  colors = [[0, '#6C92FA']];
  registerItems = [];
  unRegisterItems = [];
  hoveredItem = null;

  showNoData = false;

  PROTECT_STATUS = {
    PROTECTED: DataMap.Protection_Status.protected.value,
    UNPROTECTED: DataMap.Protection_Status.not_protected.value
  };

  isHcsUser = this.cookieService.get('userType') === CommonConsts.HCS_USER_TYPE;

  constructor(
    private router: Router,
    private i18n: I18NService,
    private cookieService: CookieService,
    private appUtilsService: AppUtilsService,
    private resourceApiService: ResourceService,
    private resourceCatalogsService: ResourceCatalogsService
  ) {}

  ngOnInit() {
    this.getSummaryResourceType([]);
    this.getProtectionCounts();
    this.initLocalStorage();
  }

  onChange() {
    this.ngOnInit();
  }

  getProtectionCounts() {
    combineLatest([
      this.resourceApiService.summaryProtectionResourceV1ResourceProtectionSummaryGet(
        {}
      ),
      this.resourceCatalogsService.getResourceCatalog()
    ]).subscribe(result => {
      const res = result[0];
      this.getSummaryResourceType(result[1]);

      let allProtectedCount = 0;
      let allCount = 0;
      const allSubType = [];
      each(this.resourceType, resource => {
        if (resource.subType && !isEmpty(resource.subType)) {
          let resourceCount = 0;
          let resourceProtectedCount = 0;
          each(resource.subType, app => {
            let findArr;
            if (isArray(app.key)) {
              const filterTmp = filter(res.summary, item => {
                return includes(app.key, item.resource_sub_type);
              });
              let pro = 0,
                unPro = 0;
              each(filterTmp, i => {
                pro += i.protected_count;
                unPro += i.unprotected_count;
              });
              findArr = {
                protected_count: pro,
                unprotected_count: unPro
              };
            } else {
              findArr = find(res.summary, item => {
                return item.resource_sub_type === app.key;
              });
            }
            if (findArr) {
              assign(app, {
                protected_count: findArr.protected_count,
                count: findArr.protected_count + findArr.unprotected_count
              });
              resourceProtectedCount += findArr.protected_count;
            } else {
              assign(app, {
                protected_count: 0,
                count: 0
              });
            }
            assign(app, {
              parentLabel: resource.label
            });
            allSubType.push(app);
            resourceCount += app.count;
          });
          resource.protected_count = resourceProtectedCount;
          resource.count = resourceCount;
        }
        allProtectedCount += resource.protected_count;
        allCount += resource.count;
      });
      assign(this.resourceType[0], {
        protected_count: allProtectedCount,
        count: allCount,
        subType: allSubType
      });
    });
  }

  getSummaryResourceType(items) {
    this.resourceType = [
      {
        icon: '',
        id: 'all',
        label: this.i18n.get('common_all_label'),
        protected_count: 0,
        count: 0,
        subType: []
      },
      {
        icon: 'aui-icon-host-app',
        id: 'database',
        label: this.i18n.get('common_database_label'),
        protected_count: 0,
        count: 0,
        subType: [...this.appUtilsService.getApplicationConfig().database]
      },
      {
        icon: 'aui-icon-big-data',
        id: 'bigData',
        label: this.i18n.get('common_bigdata_label'),
        protected_count: 0,
        count: 0,
        subType: [...this.appUtilsService.getApplicationConfig().bigData]
      },
      {
        icon: 'aui-icon-virtualization',
        id: 'virtualization',
        label: this.i18n.get('common_virtualization_label'),
        protected_count: 0,
        count: 0,
        subType: [...this.appUtilsService.getApplicationConfig().virtualization]
      },
      {
        icon: 'aui-icon-container-app',
        id: 'container',
        label: this.i18n.get('common_container_label'),
        protected_count: 0,
        count: 0,
        subType: [...this.appUtilsService.getApplicationConfig().container]
      },
      {
        icon: 'aui-icon-cloud',
        id: 'cloud',
        label: this.i18n.get('common_huawei_clouds_label'),
        protected_count: 0,
        count: 0,
        subType: [...this.appUtilsService.getApplicationConfig().cloud]
      },
      {
        icon: 'aui-icon-application',
        id: 'application',
        label: this.i18n.get('common_application_label'),
        protected_count: 0,
        count: 0,
        subType: [...this.appUtilsService.getApplicationConfig().application]
      },
      {
        icon: 'aui-icon-file-service-app',
        id: 'file-service',
        label: this.i18n.get('common_file_systems_label'),
        protected_count: 0,
        count: 0,
        subType: [...this.appUtilsService.getApplicationConfig().fileService]
      }
    ];
  }

  initLocalStorage() {
    this.resourceType.forEach(item => {
      item.subType.forEach(subItem => {
        if (!localStorage.getItem(`${subItem.id}ToTopTime`)) {
          localStorage.setItem(`${subItem.id}ToTopTime`, '0');
        }
      });
    });
  }

  iconBackToTopClicked(event: Event, item: any) {
    event.stopPropagation();
    if (localStorage.getItem(`${item.id}ToTopTime`) === '0') {
      localStorage.setItem(
        `${item.id}ToTopTime`,
        new Date().getTime().toString()
      );
    } else {
      localStorage.setItem(`${item.id}ToTopTime`, '0');
    }
  }

  backToTopIcon(item) {
    return localStorage.getItem(`${item.id}ToTopTime`) !== '0'
      ? 'icon-back-to-top-blue'
      : item !== this.hoveredItem
      ? ''
      : 'icon-back-to-top';
  }

  toTopIconToolTip(item) {
    return this.i18n.get(
      localStorage.getItem(`${item.id}ToTopTime`) === '0'
        ? 'common_to_top_label'
        : 'common_cancel_to_top_label'
    );
  }

  updateRegisterItems(items) {
    this.registerItems = filter(items, item => !!item.count);
    this.registerItems.sort((a, b) => {
      return (
        this.compareToTopTime(a, b) ||
        b.count - a.count ||
        (a.label < b.label ? -1 : a.label > b.label ? 1 : 0)
      );
    });
    return this.registerItems;
  }

  updateUnRegisterItems(items) {
    this.unRegisterItems = filter(items, item => !item.count);
    this.unRegisterItems.sort((a, b) => {
      return (
        this.compareToTopTime(a, b) ||
        (a.label < b.label ? -1 : a.label > b.label ? 1 : 0)
      );
    });
    return this.unRegisterItems;
  }

  compareToTopTime(a, b) {
    const aTime = parseInt(localStorage.getItem(`${a.id}ToTopTime`));
    const bTime = parseInt(localStorage.getItem(`${b.id}ToTopTime`));
    return bTime - aTime;
  }

  gotoResource(item) {
    if (this.isHcsUser && window.parent) {
      let parentUrl = window.parent.location.href;
      const routerUrl = includes(item?.protectionUrl, RouterUrl.ProtectionCloud)
        ? '/csbs/manager/tabs/backup'
        : item?.protectionUrl.substring(
            0,
            item?.protectionUrl.lastIndexOf('/')
          );
      window.parent.location.href = `${first(
        parentUrl.split('#')
      )}#${routerUrl}`;
      defer(() => {
        this.appUtilsService.setCacheValue('hcsUserRouterApp', item?.id);
      });
    } else {
      SUB_APP_REFRESH_FLAG.emit = true;
      this.router.navigateByUrl(item?.protectionUrl);
    }
  }
}
