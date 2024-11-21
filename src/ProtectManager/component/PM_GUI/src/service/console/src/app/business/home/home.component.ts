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
import { Component, OnInit, ViewChild } from '@angular/core';
import { Router } from '@angular/router';
import { MessageService } from '@iux/live';
import {
  ApiMultiClustersService,
  CapacityApiService,
  CookieService,
  DataMap,
  GlobalService,
  PerformanceApiDescService,
  I18NService,
  UsersApiService,
  LANGUAGE,
  timeZones
} from 'app/shared';
import { DataReductionComponent } from 'app/shared/components';
import { CapacityForecastChartComponent } from 'app/shared/components/charts/capacity-forecast-chart/capacity-forecast-chart.component';
import { SystemCapacityChartComponent } from 'app/shared/components/charts/system-capacity-chart/system-capacity-chart.component';
import { AppUtilsService } from 'app/shared/services/app-utils.service';
import { eq, find, get, isNil } from 'lodash';

@Component({
  selector: 'old-home',
  templateUrl: './home.component.html',
  styleUrls: ['./home.component.css']
})
export class HomeComponent implements OnInit {
  systemCapacityInfo;
  isAllCluster = true;
  enableMonitor = false;
  isHyperdetect =
    this.i18n.get('deploy_type') === DataMap.Deploy_Type.hyperdetect.value;
  isCyberEngine =
    this.i18n.get('deploy_type') === DataMap.Deploy_Type.cyberengine.value;
  isSamlAdmin = false;
  curNode;

  @ViewChild(DataReductionComponent, { static: false })
  dataReductionComponent: DataReductionComponent;
  @ViewChild(CapacityForecastChartComponent, { static: false })
  capacityForecastChartComponent: CapacityForecastChartComponent;

  @ViewChild(SystemCapacityChartComponent, { static: false })
  systemCapacityChartComponent: SystemCapacityChartComponent;

  constructor(
    public router: Router,
    public i18n: I18NService,
    public cookieService: CookieService,
    private globalService: GlobalService,
    public usersApiService: UsersApiService,
    public capacityApiService: CapacityApiService,
    private multiClustersServiceApi: ApiMultiClustersService,
    private performanceApiService: PerformanceApiDescService,
    public appUtilsService: AppUtilsService,
    private message: MessageService
  ) {}

  ngOnInit() {
    localStorage.setItem(
      'doRefresh',
      JSON.stringify(this.cookieService.get('userId'))
    );
    localStorage.removeItem('doRefresh');
    if (this.isCyberEngine) {
      return;
    }
    this.showLoginInfo();
    this.getCurrentUser();
    this.getClusterStateChange();
  }

  showLoginInfo() {
    const adfsLogUrl = this.router.url;
    const lastLoginInfo = {
      lastLoginIp: this.getUrlParam('lastLoginIp', adfsLogUrl),
      lastLoginTime: this.getUrlParam('lastLoginTime', adfsLogUrl),
      lastLoginZone: this.getUrlParam('lastLoginZone', adfsLogUrl)
    };
    if (
      isNil(lastLoginInfo.lastLoginIp) ||
      isNil(lastLoginInfo.lastLoginTime)
    ) {
      return;
    }

    const currentTimeZones = get(timeZones, [
      eq(this.i18n.language, LANGUAGE.CN) ? 'zh' : 'en'
    ]);
    const lastLoginZone =
      lastLoginInfo.lastLoginZone &&
      get(
        find(currentTimeZones, ['value', lastLoginInfo.lastLoginZone]),
        'label'
      );
    const lastLoginTimeContent = isNil(lastLoginZone)
      ? lastLoginInfo.lastLoginTime
      : lastLoginInfo.lastLoginTime + ' ' + lastLoginZone;
    const loginInfoContent = `${this.i18n.get(
      'common_last_login_time_label'
    )}: ${lastLoginTimeContent}\n${this.i18n.get(
      'common_last_login_ip_label'
    )}: ${lastLoginInfo.lastLoginIp}`;
    this.message.info(loginInfoContent, {
      lvMessageKey: 'lastLoginInfoMsg',
      lvPosition: 'topRight',
      lvShowCloseButton: true
    });
  }
  getUrlParam(name, url) {
    const reg = new RegExp('(^|&)' + name + '=([^&]*)(&|$)', 'i');
    const num = url.indexOf('?');
    const str = url.substring(num + 1);
    const r = str.match(reg);
    if (r !== null && r[2] !== '') {
      return decodeURIComponent(r[2]);
    }
  }

  getCurrentUser() {
    const userId = this.cookieService.get('userId');
    this.usersApiService.getUsingGET2({ userId }).subscribe(
      res => {
        this.isSamlAdmin =
          res.userType === DataMap.loginUserType.saml.value &&
          res.rolesSet[0].roleId === 2;

        if (!this.isSamlAdmin) {
          this.getClusterNodeShow();
          this.getCapcacity();
        }
      },
      err => {
        this.getClusterNodeShow();
        this.getCapcacity();
      }
    );
  }

  getClusterNodeShow() {
    const clusterObj = JSON.parse(
      decodeURIComponent(this.cookieService.get('currentCluster'))
    );
    this.isAllCluster =
      (!clusterObj ||
        (clusterObj && clusterObj['icon'] === 'aui-icon-all-cluster')) &&
      !this.cookieService.isCloudBackup;
    if (!this.isAllCluster) {
      this.performanceApiService
        .getPerformanceConfigUsingGET({
          akLoading: false
        })
        .subscribe(res => {
          this.enableMonitor = res === 'true';
        });
    }
  }

  getClusterStateChange() {
    this.globalService.getState('checkedCluster').subscribe(cluster => {
      this.performanceApiService
        .getPerformanceConfigUsingGET({
          akLoading: false,
          clustersId: cluster.clusterId,
          memberEsn: cluster.memberEsn,
          clustersType: cluster.clusterType
        })
        .subscribe(res => {
          this.enableMonitor = res === 'true';
        });
      this.curNode = cluster;
    });
  }

  getCapcacity() {
    if (this.cookieService.isCloudBackup) {
      return;
    }
    if (this.isAllCluster) {
      this.multiClustersServiceApi
        .getMultiClusterCapacity({ akLoading: false })
        .subscribe(res => {
          this.dataReductionComponent?.initCapacityInfo(res);
          this.systemCapacityChartComponent?.createCapacityChart(res);
        });
    } else {
      this.capacityApiService
        .queryClusterStorageUsingGET({ akLoading: false })
        .subscribe(res => {
          this.dataReductionComponent?.initCapacityInfo(res);
          this.systemCapacityChartComponent?.createCapacityChart(res);
        });
    }
  }
}
