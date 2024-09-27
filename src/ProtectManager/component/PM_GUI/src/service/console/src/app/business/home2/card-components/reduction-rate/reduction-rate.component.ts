import { Component, OnInit, ViewChild, Input } from '@angular/core';
import { Router } from '@angular/router';
import { MessageService } from '@iux/live';
import {
  ApiMultiClustersService,
  CapacityApiService,
  CookieService,
  DataMap,
  PerformanceApiDescService,
  I18NService,
  UsersApiService,
  LANGUAGE,
  timeZones,
  CAPACITY_UNIT
} from 'app/shared';
import { CapacityForecastChartComponent } from 'app/shared/components/charts/capacity-forecast-chart/capacity-forecast-chart.component';
import { AppUtilsService } from 'app/shared/services/app-utils.service';
import { eq, find, get, isNil } from 'lodash';
import { CapacityCalculateLabel } from 'app/shared';

@Component({
  selector: 'reduction-rate',
  templateUrl: './reduction-rate.component.html',
  styleUrls: ['./reduction-rate.component.less'],
  providers: [CapacityCalculateLabel]
})
export class ReductionRateComponent implements OnInit {
  @Input() cardInfo: any = {};
  isAllCluster = true;
  enableMonitor = false;
  isHyperdetect =
    this.i18n.get('deploy_type') === DataMap.Deploy_Type.hyperdetect.value;
  isCyberEngine =
    this.i18n.get('deploy_type') === DataMap.Deploy_Type.cyberengine.value;
  isSamlAdmin = false;
  reductionInfo: any = {};

  @ViewChild(CapacityForecastChartComponent, { static: false })
  capacityForecastChartComponent: CapacityForecastChartComponent;

  constructor(
    public router: Router,
    public i18n: I18NService,
    public cookieService: CookieService,
    public usersApiService: UsersApiService,
    public capacityApiService: CapacityApiService,
    private multiClustersServiceApi: ApiMultiClustersService,
    private performanceApiService: PerformanceApiDescService,
    public appUtilsService: AppUtilsService,
    private message: MessageService,
    public capacityCalculateLabel: CapacityCalculateLabel
  ) {}

  ngOnInit() {
    this.cardInfo.loading = true;
    this.getCurrentUser();
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
    const PARAM_VALUE_INDEX = 2;
    if (r !== null && r[PARAM_VALUE_INDEX] !== '') {
      return decodeURIComponent(r[PARAM_VALUE_INDEX]);
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
          Promise.all([this.getClusterNodeShow(), this.getCapcacity()]).then(
            () => {
              this.cardInfo.loading = false;
            }
          );
        }
      },
      err => {
        this.getClusterNodeShow();
        this.getCapcacity();
      }
    );
  }

  getClusterNodeShow() {
    return new Promise(resolve => {
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
            resolve(true);
          });
      } else {
        resolve(true);
      }
    });
  }

  getCapcacity() {
    if (this.cookieService.isCloudBackup) {
      return;
    }
    return new Promise(resolve => {
      if (this.isAllCluster) {
        this.multiClustersServiceApi
          .getMultiClusterCapacity({ akLoading: false })
          .subscribe(res => {
            let consumedCapacityInfo = this.transformKBUnit(
              res.consumedCapacity
            ); //物理用量
            let writeCapacityInfo = this.transformKBUnit(res.writeCapacity); //逻辑用量
            this.reductionInfo = {
              spaceReductionRate: `${res.spaceReductionRate}:1`,
              consumedCapacity: consumedCapacityInfo.split(' ')[0],
              consumedCapacityUnit: consumedCapacityInfo.split(' ')[1],
              writeCapacity: writeCapacityInfo.split(' ')[0],
              writeCapacityUnit: writeCapacityInfo.split(' ')[1]
            };
            resolve(true);
          });
      } else {
        this.capacityApiService
          .queryClusterStorageUsingGET({ akLoading: false })
          .subscribe(res => {
            let consumedCapacityInfo = this.transformKBUnit(
              res.consumedCapacity
            ); //物理用量
            let writeCapacityInfo = this.transformKBUnit(res.writeCapacity); //逻辑用量
            this.reductionInfo = {
              spaceReductionRate: `${res.spaceReductionRate}:1`,
              consumedCapacity: consumedCapacityInfo.split(' ')[0],
              consumedCapacityUnit: consumedCapacityInfo.split(' ')[1],
              writeCapacity: writeCapacityInfo.split(' ')[0],
              writeCapacityUnit: writeCapacityInfo.split(' ')[1]
            };
            resolve(true);
          });
      }
    });
  }

  private transformKBUnit(capacity) {
    return this.capacityCalculateLabel.transform(
      capacity,
      '1.2-2',
      CAPACITY_UNIT.KB,
      false
    );
  }
}
