import { Component, Input, OnDestroy, OnInit } from '@angular/core';
import { NavigationEnd, Router } from '@angular/router';
import {
  CommonConsts,
  CookieService,
  CopiesService,
  CopyControllerService,
  GlobalService,
  I18NService,
  ResourceService,
  SUB_APP_REFRESH_FLAG
} from 'app/shared';
import { AppUtilsService } from 'app/shared/services/app-utils.service';
import {
  assign,
  each,
  filter,
  find,
  first,
  includes,
  isArray,
  isEmpty,
  isUndefined,
  map
} from 'lodash';
import { Subject, Subscription } from 'rxjs';
import { takeUntil } from 'rxjs/operators';

@Component({
  selector: 'aui-sub-app-card',
  templateUrl: './sub-app-card.component.html',
  styleUrls: ['./sub-app-card.component.less']
})
export class SubAppCardComponent implements OnInit, OnDestroy {
  @Input() subApp: any[];
  @Input() typeTitle: string;
  @Input() routerType: string;
  activeId: string;
  isHcsUser = this.cookieService.get('userType') === CommonConsts.HCS_USER_TYPE;
  destroy$ = new Subject();
  showKerberos = false;
  isSort = false;
  timeout;
  summarySub$: Subscription;
  storageSub$: Subscription;
  routeSubscription: Subscription;
  hoveredItem = null;
  helpUrl = '';
  isEmpty = isEmpty;

  constructor(
    private router: Router,
    private globalService: GlobalService,
    private cookieService: CookieService,
    private copiesApiService: CopiesService,
    private appUtilsService: AppUtilsService,
    private resourceApiService: ResourceService,
    private copyControllerService: CopyControllerService,
    private i18n: I18NService
  ) {}

  ngOnDestroy() {
    this.destroy$.next(true);
    this.destroy$.complete();
    if (this.timeout) {
      clearTimeout(this.timeout);
    }
    if (this.routeSubscription) {
      this.routeSubscription.unsubscribe();
    }
  }

  ngOnInit() {
    this.showKerberos =
      this.isHcsUser &&
      this.routerType !== 'copy' &&
      !isEmpty(find(this.subApp, { id: 'hdfs' }));
    if (this.routerType === 'copy') {
      this.getCopyData();
      this.setActiveId();
    } else if (includes(['livemount', 'anti'], this.routerType)) {
      this.checked(first(this.subApp)?.id);
      this.isSort = true;
    } else {
      this.getSummaryProtection();
      this.getStorageState();
    }
    this.initLocalStorage();
    this.listenRouterChange();
    if (this.routerType !== 'copy') {
      this.helpUrl = this.appUtilsService.getHelpUrl();
    }
  }

  getStorageState() {
    // 存储设备单独监听获取数量
    if (!this.storageSub$) {
      this.storageSub$ = this.globalService
        .getState('emitStorage')
        .pipe(takeUntil(this.destroy$))
        .subscribe(res => {
          this.subApp = [...this.setStorageDevice(this.subApp, res)];
        });
    }
  }

  setStorageDevice(apps, res) {
    each(apps, app => {
      if (app.id === 'storage-device') {
        app.count = res['StorageEquipment'];
      }
    });
    return apps;
  }

  listenRouterChange() {
    if (isUndefined(this.routeSubscription)) {
      this.routeSubscription = this.router.events.subscribe(event => {
        if (event instanceof NavigationEnd) {
          const currentApp = find(this.subApp, { id: this.activeId });

          if (
            !currentApp ||
            !find(this.subApp, item => {
              return (
                item.copyUrl === this.router.url ||
                item.protectionUrl === this.router.url
              );
            })
          ) {
            return;
          }
          if (
            currentApp.protectionUrl !== this.router.url &&
            currentApp.copyUrl !== this.router.url
          ) {
            if (this.routerType === 'copy') {
              this.getCopyData();
              this.setActiveId();
            } else {
              this.getSummaryProtection(true);
            }
          }
        }
      });
    }
  }

  triggerEmitState() {
    this.timeout = setTimeout(() => {
      if (this.timeout) {
        clearTimeout(this.timeout);
      }
      this.getEmitState();
    }, CommonConsts.TIME_INTERVAL_FIVE);
  }

  gotoKerberos() {
    this.router.navigateByUrl('/system/security/kerberos');
  }

  getEmitState() {
    if (!this.summarySub$) {
      this.summarySub$ = this.globalService
        .getState('emitRefreshApp')
        .pipe(takeUntil(this.destroy$))
        .subscribe(res => {
          if (this.routerType === 'copy') {
            this.refreshCopyCount();
          } else {
            this.getSummaryProtection(false);
          }
        });
    }
  }

  onChange() {
    this.ngOnInit();
  }

  setActiveId() {
    if (!SUB_APP_REFRESH_FLAG.emit) {
      return;
    }
    const preApp = find(
      this.subApp,
      item =>
        item.copyUrl === this.router.url ||
        item.protectionUrl === this.router.url
    );
    if (preApp && preApp.id !== this.activeId) {
      this.checked(preApp.id);
    }
    SUB_APP_REFRESH_FLAG.emit = false;
  }

  checked(appId: string) {
    const app = find(this.subApp, { id: appId });
    if (!app) {
      return;
    }
    this.activeId = appId;
    this.triggerEmitState();
    if (this.routerType === 'copy') {
      this.router.navigateByUrl(app.copyUrl);
    } else if (this.routerType === 'livemount') {
      this.router.navigateByUrl(app.livemountUrl);
    } else if (this.routerType === 'anti') {
      this.router.navigateByUrl(app.antiUrl);
    } else {
      this.router.navigateByUrl(app.protectionUrl);
    }
  }

  getSummaryProtection(loading = true) {
    this.resourceApiService
      .summaryProtectionResourceV1ResourceProtectionSummaryGet({
        akLoading: loading,
        akDoException: false
      })
      .subscribe(res => {
        each(this.subApp, app => {
          let findArr;
          const filterTmp = filter(res.summary, item => {
            return isArray(app.key)
              ? includes(app.key, item.resource_sub_type)
              : app.key === item.resource_sub_type;
          });
          if (!isEmpty(filterTmp)) {
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
          }
          if (findArr) {
            assign(app, {
              protected_count: findArr.protected_count,
              count: findArr.protected_count + findArr.unprotected_count
            });
          } else {
            if (app.id !== 'storage-device') {
              assign(app, {
                protected_count: 0,
                count: 0
              });
            }
          }
          assign(app, {
            parentLabel: this.typeTitle
          });
        });
        this.sortApps();
        this.isSort = true;
        if (loading) {
          const appId = this.appUtilsService.getCacheValue('hcsUserRouterApp');
          if (this.isHcsUser && appId) {
            const jumpApp = find(this.subApp, { id: appId });
            if (jumpApp) {
              this.checked(jumpApp.id);
            }
          } else {
            if (includes(map(this.subApp, 'protectionUrl'), this.router.url)) {
              this.checked(first(this.subApp)?.id);
              this.setActiveId();
            }
          }
        }
      });
  }

  initLocalStorage() {
    this.subApp.forEach(subItem => {
      if (!localStorage.getItem(`${subItem.id}ToTopTime`)) {
        localStorage.setItem(`${subItem.id}ToTopTime`, '0');
      }
    });
  }

  sortApps() {
    this.subApp.sort((a, b) => {
      if (!!this.compareToTopTime(a, b)) {
        return this.compareToTopTime(a, b);
      }
      if (a.id === 'storage-device' || b.id === 'storage-device') {
        return 0;
      }
      return (
        b.count - a.count ||
        (a.label < b.label ? -1 : a.label > b.label ? 1 : 0)
      );
    });
  }

  compareToTopTime(a, b) {
    const aTime = parseInt(localStorage.getItem(`${a.id}ToTopTime`));
    const bTime = parseInt(localStorage.getItem(`${b.id}ToTopTime`));
    return bTime - aTime;
  }

  getCopyData() {
    this.isSort = true;
    this.getCommonCopyData();
  }

  getCommonCopyData() {
    this.copyControllerService
      .QueryCopyCount({ akDoException: false })
      .subscribe(res => {
        each(this.subApp, app => {
          let total = 0;
          const filterTmp = filter(res, item => {
            return isArray(app.key)
              ? includes(app.key, item.resourceSubType)
              : app.key === item.resourceSubType;
          });
          each(filterTmp, i => {
            total += i.copyCount;
          });
          assign(app, {
            count: total
          });
          each(this.subApp, item => {
            if (item.id === app.id) {
              assign(item, {
                count: app.count
              });
            }
          });
        });
        this.sortApps();
        this.checked(first(this.subApp)?.id);
      });
  }

  refreshCopyCount() {
    each(this.subApp, item => {
      if (item.id !== this.activeId) {
        return;
      }
      this.copiesApiService
        .queryResourcesV1CopiesGet({
          akLoading: false,
          akDoException: false,
          pageNo: CommonConsts.PAGE_START,
          pageSize: CommonConsts.PAGE_SIZE,
          conditions: JSON.stringify({
            resource_sub_type: isArray(item.key) ? item.key : [item.key]
          })
        })
        .subscribe(res => {
          if (res.total) {
            assign(item, {
              count: res.total
            });
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
    this.sortApps();
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
}
