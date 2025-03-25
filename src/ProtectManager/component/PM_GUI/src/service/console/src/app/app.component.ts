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
import {
  AfterViewChecked,
  ChangeDetectorRef,
  Component,
  ElementRef,
  NgZone,
  OnDestroy,
  OnInit,
  TemplateRef,
  ViewChild
} from '@angular/core';
import { Title } from '@angular/platform-browser';
import { ActivatedRoute, NavigationEnd, Router } from '@angular/router';
import {
  MenuComponent,
  MessageboxService,
  MessageService,
  ModalService
} from '@iux/live';
import {
  ALARM_NAVIGATE_STATUS,
  BaseUtilService,
  CommonConsts,
  CookieService,
  CUSTOM_VERSION,
  DataMap,
  DataMapService,
  EMIT_TASK,
  getAccessibleMenu,
  getAccessibleViewList,
  getAppTheme,
  GlobalService,
  LANGUAGE,
  LogoutType,
  RoleType,
  RouterUrl,
  SUB_APP_REFRESH_FLAG,
  SupportLicense,
  SYSTEM_TIME,
  ThemeEnum,
  THEME_TRIGGER_ACTION
} from 'app/shared';
import { GuideService } from 'app/shared/services/new-comer-guidance.service';
import {
  IMAGE_PATH_PREFIX,
  IoemInfo,
  WhiteboxService
} from 'app/shared/services/whitebox.service';
import {
  assign,
  cloneDeep,
  each,
  find,
  get,
  includes,
  isArray,
  isEmpty,
  isUndefined,
  set,
  size,
  toString
} from 'lodash';
import { Observable, Observer, Subject } from 'rxjs';
import { distinctUntilChanged, finalize, map, switchMap } from 'rxjs/operators';
import { JobTableComponent } from './business/insight/job/job-table/job-table.component';
import { GROUP_COMMON, I18NService, MODAL_COMMON } from './shared';
import {
  ADFSService,
  AlarmAndEventApiService,
  ApiMultiClustersService,
  ClustersApiService,
  CopiesDetectReportService,
  JobAPIService,
  SecurityApiService,
  SlaApiService,
  SysVersionServiceService,
  UsersApiService
} from './shared/api/services';
import { ExportQueryResultsComponent } from './shared/components/export-query-results/export-query-results.component';
import { ModifyPasswordComponent } from './shared/components/user-manager/modify-password.component';
import { clearUserGuideCache } from './shared/consts/guide-config';
import { AppUtilsService } from './shared/services/app-utils.service';
import { AuthApiService } from './shared/services/auth-api.service';
import { BrowserActionService } from './shared/services/browser-action.service';
import { CopyActionService } from './shared/services/copy-action.service';
import { DrawModalService } from './shared/services/draw-modal.service';
import { RememberColumnsService } from './shared/services/remember-columns.service';
import { ResourceCatalogsService } from './shared/services/resource-catalogs.service';
@Component({
  selector: 'app-root',
  templateUrl: './app.component.html',
  styleUrls: ['./app.component.less']
})
export class AppComponent implements OnInit, OnDestroy, AfterViewChecked {
  isZh: boolean;
  isLogin = true;
  isErrorPage = true;
  isReportDetail = false;
  isMultiCluster = true;
  popoverShow = false;
  guideTipsMenu = [];
  ulElementRefArray: HTMLLIElement[];
  menus = [];
  jobsData = [];
  taskData = [];
  alarmData = [];
  roleType = RoleType;
  currentCluster = {};
  runningTaskCount = 0;
  runningTotal = 0;
  criticalAlarmCount = 0;
  MENU_BOTTOM_HEIGHT = 48; // 菜单栏底部高度
  MENU_HEADER_HEIGHT = 64; // 菜单栏顶部高度
  SCROLL_WIDTH = 16; // 菜单栏滚动条高度
  originScrollHeight; // 记录激活的菜单节点
  taskBadgeVisible = true;
  alarmBadgeVisible = true;
  alarmSeverityType = DataMap.Alarm_Severity_Type;
  userName;
  userType;
  language;
  activeId;
  websiteLabel;
  groupOptions = GROUP_COMMON;
  jobStatus = DataMap.Job_status;
  _includes = includes;
  sessionTimeout;
  jobTimeout;
  criticalAlarmTimeout;
  recentjobTimeout;
  SESSION_TIMER = CommonConsts.TIME_INTERVAL_SESSION_OUT;
  versionLabel = this.whitebox.isWhitebox
    ? this.i18n.get('common_version_label', [], true) +
      this.i18n.get('common_whitebox_product_version_label', [
        (this.whitebox.oem as IoemInfo).productModel
      ])
    : this.i18n.get('common_version_label', [], true) +
      this.i18n.get(
        this.i18n.get('deploy_type') === DataMap.Deploy_Type.cyberengine.value
          ? 'common_oceancyber_product_version_label'
          : 'common_product_version_label'
      );
  warningLabel = '';
  copyRightLabel = '';
  noDataLabel = this.i18n.get('common_no_data_label');
  recentTaskLabel = this.i18n.get('common_recent_jobs_label');
  recentAlarmLabel = this.i18n.get('common_recent_alarms_label');
  showAllTaskLabel = this.i18n.get('common_view_all_jobs_label');
  showAllAlarmLabel = this.i18n.get('common_view_all_alarms_label');
  isHyperdetect =
    this.i18n.get('deploy_type') === DataMap.Deploy_Type.hyperdetect.value;
  isCyberEngine =
    this.i18n.get('deploy_type') === DataMap.Deploy_Type.cyberengine.value;
  isDataBackup = this.appUtilsService.isDataBackup;
  tempToken;
  latest = 'latest';
  taskLoading = false;
  seesionTimeOut = 60;
  isCloudBackup = includes(
    [
      DataMap.Deploy_Type.cloudbackup2.value,
      DataMap.Deploy_Type.cloudbackup.value,
      DataMap.Deploy_Type.hyperdetect.value
    ],
    this.i18n.get('deploy_type')
  );
  isHcsUser = false;
  isDmeUser = false;
  isV1Alarm =
    this.appUtilsService.isDecouple || this.appUtilsService.isDistributed;
  clusterOptions;
  notsupportModifyPwd = [
    DataMap.loginUserType.saml.value,
    DataMap.loginUserType.ldap.value,
    DataMap.loginUserType.ldapGroup.value,
    DataMap.loginUserType.hcs.value,
    DataMap.loginUserType.adfs.value
  ];

  collapsed = false;
  title = this.baseUtilService.getProductName();
  isHcsEnvir =
    this.cookieService.get('serviceProduct') === CommonConsts.serviceProduct;
  needGuideProduct =
    this.appUtilsService.isDataBackup ||
    this.appUtilsService.isDecouple ||
    this.appUtilsService.isDistributed;
  showGuide = false;
  guideTipShow = false;
  hasGuideTipShow = false;

  cyberDarkHeader = false;
  customVeriosnLoaded = false;
  supportChangeTheme =
    this.appUtilsService.isDataBackup ||
    this.appUtilsService.isDecouple ||
    this.appUtilsService.isDistributed;
  appTheme = ThemeEnum.light;
  themeKey = ThemeEnum.light;
  themeTypes = [
    {
      value: ThemeEnum.light,
      icon: 'aui-theme-light',
      label: this.i18n.get('common_theme_light_label')
    },
    {
      value: ThemeEnum.dark,
      icon: 'aui-theme-dark',
      label: this.i18n.get('common_theme_dark_label')
    },
    {
      value: ThemeEnum.auto,
      icon: 'aui-theme-auto',
      label: this.i18n.get('common_theme_auto_label')
    }
  ];
  taskPopoverClass = this.supportChangeTheme
    ? 'themePopoverClass'
    : 'taskPopoverClass';

  protectionRouterUrlList = [
    RouterUrl.ProtectionDatabase,
    RouterUrl.ProtectionBigData,
    RouterUrl.ProtectionContainer,
    RouterUrl.ProtectionCloud,
    RouterUrl.ProtectionVirtualization,
    RouterUrl.ProtectionFileService,
    RouterUrl.ProtectionApplication,
    RouterUrl.ProtectionBareMetal
  ];

  // 同一地址开启多个页签，只操作其中一个，另外页签超时退出，导致正在操作的页面也超时退出。不能通过变量判断，通过存入cookie来判断。
  TIMEOUT_STATUS_KEY = `random_key_timeout`;

  @ViewChild('taskPopover', { static: false }) taskPopover;
  @ViewChild('alarmPopover', { static: false }) alarmPopover;
  @ViewChild('aboutHeaderTpl', { static: false })
  aboutHeaderTpl: TemplateRef<any>;
  @ViewChild('aboutContentTpl', { static: false })
  aboutContentTpl: TemplateRef<any>;
  @ViewChild('aboutFooterTpl', { static: false })
  aboutFooterTpl: TemplateRef<any>;
  @ViewChild('menu', { static: false, read: ElementRef })
  lvMenuElementRef: ElementRef; // 获取lv-menu整个DOM树
  @ViewChild('menu', { static: false, read: MenuComponent })
  lvMenuComponent: MenuComponent; // lv-menu组件，需要使用到里面的方法
  @ViewChild('menuIconTpl', { static: false, read: ElementRef })
  menuIconTpl: ElementRef;
  @ViewChild('globalSearchTpl', { static: false, read: ElementRef })
  globalSearchTpl: ElementRef;
  @ViewChild('intelliMateIconTpl', { static: false, read: ElementRef })
  intelliMateIconTpl: ElementRef;
  @ViewChild('shortcutOpTpl', { static: false, read: ElementRef })
  shortcutOpTpl: ElementRef;
  @ViewChild('overlay', { static: false }) overlayTemplate: ElementRef<any>; // 被激活的元素顶层的div，激活效果通过修改div实现
  originEle: ElementRef; // 新人指引气泡窗挂载源
  constructor(
    private cdr: ChangeDetectorRef,
    public i18n: I18NService,
    public router: Router,
    private route: ActivatedRoute,
    private messageBox: MessageboxService,
    private messageService: MessageService,
    public appUtilsService: AppUtilsService,
    private drawModalService: DrawModalService,
    private authApiService: AuthApiService,
    private usersApiService: UsersApiService,
    public cookieService: CookieService,
    private globalService: GlobalService,
    public jobAPIService: JobAPIService,
    public alarmApiService: AlarmAndEventApiService,
    private multiClustersServiceApi: ApiMultiClustersService,
    public dataMapService: DataMapService,
    public securityApiService: SecurityApiService,
    public slaApiService: SlaApiService,
    public guideService: GuideService,
    private resourceCatalogsService: ResourceCatalogsService,
    private rememberColumnsService: RememberColumnsService,
    private modalService: ModalService,
    private ngZone: NgZone,
    private browserActionService: BrowserActionService,
    private clusterApiService: ClustersApiService,
    private titleService: Title,
    private baseUtilService: BaseUtilService,
    private whitebox: WhiteboxService,
    private copyActionService: CopyActionService,
    private copiesDetectReportService: CopiesDetectReportService,
    public adfsService: ADFSService,
    private sysVersionService: SysVersionServiceService
  ) {}

  ngOnInit() {
    this.browserActionService.checkBrowserZoom();
    this.getUser()
      .pipe(switchMap(res => this.setPermission(res)))
      .subscribe(() => {
        this.routeChange();
        this.setPollingFn();
      });
    this.globalService
      .getState('queryExportFilesResult')
      .subscribe(res => this.exportQuery());
    // 监听智能助手中点击'新手指引'
    this.globalService
      .getState('new_comer_guider')
      .pipe(distinctUntilChanged())
      .subscribe(res => {
        this.popoverShow = res === 'true';
        if (this.popoverShow) {
          this.startGuide();
          this.getLiElementArray();
        } else {
          this.endGuide();
        }
      });
    this.globalService
      .getState(EMIT_TASK)
      .subscribe(() => this.runningTaskPolling());
    this.globalService.getUserInfo().subscribe(res => {
      this.userName = res.state.userName;
      this.userType = res.state.userType;
      this.cookieService.set('userType', res.state.userType);
      this.routeChange();
      this.rememberColumnsService.setUser(res.state.userName);
      this.setPollingFn();
    });
    this.currentCluster = JSON.parse(
      decodeURIComponent(this.cookieService.get('currentCluster'))
    ) || {
      clusterId: DataMap.Cluster_Type.local.value,
      clusterName: this.i18n.get('common_all_clusters_label'),
      clusterType: DataMap.Cluster_Type.local.value,
      icon: 'aui-icon-all-cluster' // 进入首页时默认为'所有集群'
    };
    this.isMultiCluster =
      !this.currentCluster ||
      (this.currentCluster &&
        this.currentCluster['icon'] === 'aui-icon-all-cluster');
    this.initAboutInfo()
      .setLanguage()
      .titleService.setTitle(this.baseUtilService.getProductName());
    this.setFavicon();
    this.listenStoragechange();
    this.getAppThemeKey();
  }

  ngAfterViewChecked() {
    if (this.isDataBackup && this.menuIconTpl && !this.originEle) {
      // 这里的代码只会进入一次
      const needShowGuide = localStorage.getItem('new_comer_guider');
      if (!isUndefined(needShowGuide)) {
        if (needShowGuide === 'false') {
          return;
        }
      }
      this.processMenuGuideData(); // 获取需要展示的菜单
      this.popoverShow = true;
      this.initHeaderTips();
      this.startGuide();
    }
  }

  private initHeaderTips() {
    const iconTplArr = [
      'menuIconTpl',
      'globalSearchTpl',
      'intelliMateIconTpl',
      'shortcutOpTpl'
    ];
    iconTplArr.forEach(item =>
      this.guideService.setGuideMountOriginById(item, this[item])
    );
  }

  startGuide() {
    this.collapsed = false; // 展开菜单栏
    this.guideService.startGuide();
    this.overlayTemplate.nativeElement.style.display = 'block';
    this.originEle = this.guideService.getGuideStep()?.mountOrigin;
    this.updateOverlayProperty();
    this.originScrollHeight = this.lvMenuElementRef.nativeElement.scrollTop; // 记住原始滚动条高度
    if (isEmpty(this.guideTipsMenu)) {
      this.processMenuGuideData(); // 获取需要展示的菜单
    }
  }

  updateOverlayProperty() {
    this.cdr.detectChanges();
    const overlayElement = this.overlayTemplate.nativeElement; // 高亮凸出效果，使用了一个div覆盖在上方。
    if (isUndefined(this.originEle)) {
      this.initHeaderTips();
      this.originEle = this.guideService.getGuideStep().mountOrigin;
    }
    const originElement = this.originEle.nativeElement;
    const style = overlayElement.style;
    const originElementInfo = originElement.getBoundingClientRect();
    // 设置custom-overlay的宽高与挂载源相同
    const targetElemParam = {
      width: originElement.offsetWidth,
      height: originElement.offsetHeight,
      top: Math.floor(originElementInfo.top - 4),
      left: Math.floor(originElementInfo.left - 4)
    };
    if (this.guideService.getGuideCurrentStep() > 4) {
      // 左侧菜单栏固定宽度220px
      targetElemParam.width = 220;
      targetElemParam.left = this.SCROLL_WIDTH;
    }
    if (targetElemParam.top + targetElemParam.height > window.innerHeight) {
      targetElemParam.height =
        window.innerHeight -
        (this.MENU_HEADER_HEIGHT + this.MENU_BOTTOM_HEIGHT);
    }
    // 参数设置完后，统一更新
    each(targetElemParam, (value, key) => (style[key] = `${value}px`));
  }

  nextStep() {
    this.guideService.nextStep();
    this.expandMenu(); // 展开单
    if (isEmpty(this.ulElementRefArray)) {
      this.getLiElementArray(); // 获取菜单li节点
    }
    this.originEle = this.guideService.getGuideStep()?.mountOrigin;
    this.autoScrollToTop(); // 自动滚动
    this.updateOverlayProperty(); // 更新overlay的样式和位置
  }

  private expandMenu() {
    // 提前展开菜单
    const stepIndex = this.guideService.getGuideCurrentStep();
    if (stepIndex === 4) {
      // 进入菜单引导之前需要提前展开所有菜单，否则渲染比代码执行慢导致气泡窗错位
      this.lvMenuComponent.lvMultiExpansion = true;
      this.menus = this.menus.map(item => {
        if (item?.items?.length > 0) {
          item.expanded = true;
        }
        return item;
      });
      this.menus = [...this.menus];
      this.cdr.detectChanges();
    }
  }

  private collapseMenu() {
    this.menus = this.menus.map(item => {
      if (item?.items?.length) {
        item.expanded = item.items.find(
          child => child.id === this.lvMenuComponent.lvActiveItemId
        );
      }
      return item;
    });
  }

  private autoScrollToTop() {
    if (this.guideService.getGuideCurrentStep() < 5) {
      return;
    }
    const liElement = this.originEle?.nativeElement;
    const scrollContainer = this.lvMenuElementRef?.nativeElement;
    // 自动将元素滚动到顶部
    if (
      liElement?.nodeName === 'LI' &&
      liElement?.parentElement ===
        this.lvMenuElementRef.nativeElement.children[0]
    ) {
      if (this.originEle.nativeElement.offsetTop > this.MENU_BOTTOM_HEIGHT) {
        scrollContainer.scrollTo(
          0,
          Math.floor(this.originEle.nativeElement.offsetTop) -
            this.MENU_HEADER_HEIGHT
        );
      }
    }
  }

  hideNewComerGuidance() {
    localStorage.setItem('new_comer_guider', 'false');
    this.globalService.emitStore({
      action: 'new_comer_guider',
      state: 'false'
    });
  }

  endGuide() {
    this.popoverShow = false;
    this.lvMenuComponent.lvMultiExpansion = false; // 设置菜单展开互斥
    this.collapseMenu();
    this.overlayTemplate.nativeElement.style.display = 'none';
    setTimeout(() => {
      this.messageService.info(
        this.i18n.get('common_comer_guidance_close_tips_label'),
        {
          lvDuration: 10 * 1e3,
          lvShowCloseButton: true,
          lvMessageKey: 'guidanceCloseKey'
        }
      );
      this.lvMenuElementRef.nativeElement.scrollTo(
        0,
        this.originScrollHeight || 0
      ); // 滚动到之前记录的位置
    }, 100);
  }

  private getLiElementArray() {
    // lvMenuElementRef获取到的是lv-menu标签，需要先查询下面的ul节点才能正确找到li列表
    const ulElementRefArray: HTMLLIElement[] = Array.from(
      this.lvMenuElementRef.nativeElement.querySelector('ul').children
    );
    // 首页不需要提示
    ulElementRefArray.shift();
    this.ulElementRefArray = Array.from(ulElementRefArray);
    // lv-menu中新增了id，这里使用id去匹配对应的li原生dom节点
    if (isEmpty(this.guideTipsMenu)) {
      this.processMenuGuideData();
    }
    this.ulElementRefArray.forEach((item, index) => {
      this.guideService.setGuideMountOriginById(
        find(this.guideTipsMenu, { id: item.id }).id,
        new ElementRef(item) // lvPopover的origin参数需要传递一个ElementRef
      );
    });
  }

  getAppThemeKey() {
    // 登录不需要深浅模式
    if (includes(['/', RouterUrl.Login], this.router.url)) {
      this.appTheme = ThemeEnum.light;
    } else {
      this.appTheme = getAppTheme(this.i18n);
    }
    if (localStorage.getItem('app_theme')) {
      this.themeKey = localStorage.getItem('app_theme') as any;
    }
  }

  // 主题切换
  themeChange() {
    localStorage.setItem('app_theme', this.themeKey);
    this.appTheme = getAppTheme(this.i18n);
    // 发布全局消息流
    this.globalService.emitStore({ action: THEME_TRIGGER_ACTION, state: true });
  }

  // 查询自定义的版本号
  getCustomVersion() {
    if (
      this.isCyberEngine ||
      this.customVeriosnLoaded ||
      this.router.url === RouterUrl.Init
    ) {
      return;
    }
    this.customVeriosnLoaded = true;
    this.sysVersionService
      .GetSysbackupVersion({ akDoException: false, akLoading: false })
      .subscribe((res: any) => {
        if (res.selfVersion) {
          this.versionLabel = `${this.i18n.get(
            'common_version_label',
            [],
            true
          )}${res.selfVersion}`;
          DataMap.Agent_File.fileName.value = `${res.selfVersion}_client.zip`;
          CUSTOM_VERSION.version = res.selfVersion;
        }
      });
  }

  showGuidePop() {
    if (
      this.needGuideProduct &&
      localStorage.getItem('user_guide') !== 'true' &&
      !this.hasGuideTipShow
    ) {
      setTimeout(() => {
        this.guideTipShow = !this.isDataBackup;
        this.hasGuideTipShow = true;
      }, 500);
    }
  }

  hasKnowGuide() {
    localStorage.setItem('user_guide', 'true');
    this.guideTipShow = false;
  }

  initAboutInfo() {
    this.isZh = this.i18n.language === LANGUAGE.CN;
    this.websiteLabel = this.whitebox.isWhitebox
      ? this.i18n.get('common_whitebox_about_website_label', [
          this.whitebox.oem[`website_${this.isZh ? 'zh' : 'en'}`]
        ])
      : this.i18n.get('common_about_website_label', [
          this.isZh ? 'cn' : 'en',
          this.isZh ? 'cn' : 'en'
        ]);
    this.warningLabel = this.whitebox.isWhitebox
      ? this.i18n.get('common_whitebox_about_warning_label', [
          this.whitebox.oem[`warn_${this.isZh ? 'zh' : 'en'}`]
        ])
      : this.i18n.get('common_about_warning_label');
    this.copyRightLabel = this.whitebox.isWhitebox
      ? this.whitebox.oem[`copyright_${this.isZh ? 'zh' : 'en'}`]
      : this.i18n.get('common_copy_right_label', [
          new Date().getFullYear() === 2021
            ? 2021
            : `2021-${new Date().getFullYear()}`
        ]);
    return this;
  }

  ngOnDestroy() {
    clearTimeout(this.jobTimeout);
    clearTimeout(this.recentjobTimeout);
    clearTimeout(this.sessionTimeout);
    clearTimeout(this.criticalAlarmTimeout);
  }

  getMenus() {
    this.resourceCatalogsService.getResourceCatalog().subscribe(items => {
      const menus = [
        {
          id: 'home',
          icon: 'aui-menu-home',
          label: this.i18n.get('common_home_label'),
          routerLink: '/home'
        },
        {
          id: 'protection',
          icon: 'aui-menu-protection',
          label: this.i18n.get('common_protection_label'),
          items: [
            {
              id: 'resource',
              label: this.i18n.get('common_resource_label'),
              type: 'group',
              description: this.i18n.get(
                'common_comer_guidance_resource_tips_label'
              ),
              items: [
                {
                  id: 'summary',
                  label: this.i18n.get('common_summary_label'),
                  routerLink: '/protection/summary'
                },
                {
                  id: 'database',
                  label: this.i18n.get('common_database_label'),
                  routerLink: RouterUrl.ProtectionDatabase,
                  childrenLink: [
                    RouterUrl.ProtectionDatabaseAntDB,
                    RouterUrl.ProtectionHostAppOracle,
                    RouterUrl.ProtectionHostAppMySQL,
                    RouterUrl.ProtectionHostAppSQLServer,
                    RouterUrl.ProtectionHostAppPostgreSQL,
                    RouterUrl.ProtectionHostAppDB2,
                    RouterUrl.ProtectionHostAppInformix,
                    RouterUrl.ProtectionOpenGauss,
                    RouterUrl.ProtectionHostAppGaussDBT,
                    RouterUrl.ProtectionHostAppTidb,
                    RouterUrl.ProtectionHostAppOceanBase,
                    RouterUrl.ProtectionHostAppTdsql,
                    RouterUrl.ProtectionHostAppKingBase,
                    RouterUrl.ProtectionHostAppDameng,
                    RouterUrl.ProtectionHostAppGoldendb,
                    RouterUrl.ProtectionHostGeneralDatabase,
                    RouterUrl.ProtectionGbase,
                    RouterUrl.ProtectionHostApLightCloudGaussDB
                  ]
                },
                {
                  id: 'big-data',
                  label: this.i18n.get('common_bigdata_label'),
                  routerLink: RouterUrl.ProtectionBigData,
                  childrenLink: [
                    RouterUrl.ProtectionHostAppMongoDB,
                    RouterUrl.ProtectionHostAppRedis,
                    RouterUrl.ProtectionHostAppGaussDBDWS,
                    RouterUrl.ProtectionHostAppClickHouse,
                    RouterUrl.ProtectionBigDataHdfs,
                    RouterUrl.ProtectionBigDataHbase,
                    RouterUrl.ProtectionBigDataHive,
                    RouterUrl.ProtectionBigDataElasticsearch
                  ]
                },
                {
                  id: 'virtualization',
                  label: this.i18n.get('common_virtualization_label'),
                  routerLink: RouterUrl.ProtectionVirtualization,
                  childrenLink: [
                    RouterUrl.ProtectionVirtualizationVmware,
                    RouterUrl.ProtectionVirtualizationCnware,
                    RouterUrl.ProtectionVirtualizationFusionCompute,
                    RouterUrl.ProtectionVirtualizationHyperV,
                    RouterUrl.ProtectionVirtualizationFusionOne,
                    RouterUrl.ProtectionVirtualizationNutanix
                  ]
                },
                {
                  id: 'container',
                  label: this.i18n.get('common_container_label'),
                  routerLink: RouterUrl.ProtectionContainer,
                  hidden: this.appUtilsService.isDistributed,
                  childrenLink: [
                    RouterUrl.ProtectionVirtualizationKubernetes,
                    RouterUrl.ProtectionVirtualizationKubernetesContainer
                  ]
                },
                {
                  id: 'cloud',
                  label: this.i18n.get('common_huawei_clouds_label'),
                  routerLink: RouterUrl.ProtectionCloud,
                  childrenLink: [
                    RouterUrl.ProtectionCloudHuaweiStack,
                    RouterUrl.ProtectionCloudOpenstack,
                    RouterUrl.ProtectionHostAppGaussDBForOpengauss,
                    RouterUrl.ProtectionApsaraStack
                  ]
                },
                {
                  id: 'application',
                  label: this.i18n.get('common_application_label'),
                  routerLink: RouterUrl.ProtectionApplication,
                  hidden: this.appUtilsService.isDistributed,
                  childrenLink: [
                    RouterUrl.ProtectionActiveDirectory,
                    RouterUrl.ProtectionHostAppExchange,
                    RouterUrl.ProtectionHostAppSapHana,
                    RouterUrl.ProtectionHostAppSaponoracle
                  ]
                },
                {
                  id: 'file-service',
                  label: this.i18n.get('common_file_systems_label'),
                  routerLink: RouterUrl.ProtectionFileService,
                  childrenLink: [
                    RouterUrl.ProtectionStorageDeviceInfo,
                    RouterUrl.ProtectionDoradoFileSystem,
                    RouterUrl.ProtectionNasShared,
                    RouterUrl.ProtectionNdmp,
                    RouterUrl.ProtectionCommonShare,
                    RouterUrl.ProtectionObject,
                    RouterUrl.ProtectionHostAppFilesetTemplate,
                    RouterUrl.ProtectionHostAppVolume
                  ]
                }
              ]
            },
            {
              id: 'client-group',
              label: this.i18n.get('protection_clients_label'),
              type: 'group',
              description: this.i18n.get(
                'common_comer_guidance_client_tips_label'
              ),
              items: [
                {
                  id: 'client',
                  label: this.i18n.get('protection_clients_label'),
                  routerLink: RouterUrl.ProtectionHostAppHost,
                  childrenLink: [
                    RouterUrl.ProtectionHostAppHost,
                    RouterUrl.ProtectionHostAppHostRegister
                  ]
                }
              ]
            },
            {
              id: 'cloudStorage',
              hidden: !includes(
                [
                  DataMap.Deploy_Type.cloudbackup.value,
                  DataMap.Deploy_Type.cloudbackup2.value
                ],
                this.i18n.get('deploy_type')
              ),
              label: this.i18n.get('common_storage_label'),
              type: 'group',
              items: [
                {
                  id: 'local-file-system-resource',
                  label: this.i18n.get('common_local_file_system_label'),
                  routerLink: RouterUrl.ProtectionLocalFileSystem
                }
              ]
            },
            {
              id: 'hyperStorage',
              hidden: !includes(
                [DataMap.Deploy_Type.hyperdetect.value],
                this.i18n.get('deploy_type')
              ),
              label: this.i18n.get('common_storage_label'),
              type: 'group',
              items: [
                {
                  id: 'local-resource',
                  label: this.i18n.get('protection_local_resource_label'),
                  routerLink: RouterUrl.ProtectionLocalResource
                }
              ]
            },
            {
              id: 'protection-policy',
              label: this.i18n.get('protection_policy_label'),
              type: 'group',
              description: this.i18n.get(
                'common_comer_guidance_protection_policy_tips_label'
              ),
              items: [
                {
                  id: 'sla',
                  label: this.i18n.get('common_sla_label'),
                  routerLink: '/protection/policy/sla'
                },
                {
                  id: 'limit-rate-policy',
                  label: this.i18n.get('common_limit_rate_policy_label'),
                  routerLink: '/protection/policy/limit-rate-policy'
                }
              ]
            }
          ]
        },
        {
          id: 'explore',
          icon: 'aui-menu-explore',
          label: this.i18n.get('common_explore_label'),
          items: [
            {
              id: 'copy-data',
              label: this.i18n.get('common_copy_data_label'),
              description: this.i18n.get(
                'common_comer_guidance_copy_data_tips_label'
              ),
              hidden: includes(
                [
                  DataMap.Deploy_Type.hyperdetect.value,
                  DataMap.Deploy_Type.cloudbackup.value,
                  DataMap.Deploy_Type.cloudbackup2.value
                ],
                this.i18n.get('deploy_type')
              ),
              type: 'group',
              items: [
                {
                  id: 'copy-database',
                  label: this.i18n.get('common_database_label'),
                  routerLink: '/explore/copy-data/database',
                  childrenLink: [
                    RouterUrl.ExploreCopyDataAntDB,
                    RouterUrl.ExploreCopyDataOracle,
                    RouterUrl.ExploreCopyDataMySQL,
                    RouterUrl.ExploreCopyDataSQLServer,
                    RouterUrl.ExploreCopyDataPostgreSQL,
                    RouterUrl.ExploreCopyDataDB2,
                    RouterUrl.ExploreCopyDataInformix,
                    RouterUrl.ExploreCopyDataOpenGauss,
                    RouterUrl.ExploreCopyDataGaussDBT,
                    RouterUrl.ExploreCopyDataTiDB,
                    RouterUrl.ExploreCopyDataOceanBase,
                    RouterUrl.ExploreCopyDataTDSQL,
                    RouterUrl.ExploreCopyDataKingBase,
                    RouterUrl.ExportCopyDataDameng,
                    RouterUrl.ExploreCopyDataGoldendb,
                    RouterUrl.ExploreCopyDataGeneralDatabase,
                    RouterUrl.ExploreCopyDataDatabaseGbase,
                    RouterUrl.ExploreCopyDataLightCloudGaussdb
                  ]
                },
                {
                  id: 'copy-big-data',
                  label: this.i18n.get('common_bigdata_label'),
                  routerLink: '/explore/copy-data/big-data',
                  childrenLink: [
                    RouterUrl.ExploreCopyDataMongoDB,
                    RouterUrl.ExploreCopyDataRedis,
                    RouterUrl.ExploreCopyDataGaussDBDWS,
                    RouterUrl.ExploreCopyDataClickHouse,
                    RouterUrl.ExploreCopyDataHdfs,
                    RouterUrl.ExploreCopyDataHbase,
                    RouterUrl.ExploreCopyDataHive,
                    RouterUrl.ExploreCopyDataElasticsearch
                  ]
                },
                {
                  id: 'copy-virtualization',
                  label: this.i18n.get('common_virtualization_label'),
                  routerLink: '/explore/copy-data/virtualization',
                  childrenLink: [
                    RouterUrl.ExploreCopyDataVMware,
                    RouterUrl.ExploreCopyDataCNware,
                    RouterUrl.ExploreCopyDataHyperv,
                    RouterUrl.ExploreCopyDataFusionCompute,
                    RouterUrl.ExploreCopyDataFusionOne,
                    RouterUrl.ExploreCopyDataNutanix
                  ]
                },
                {
                  id: 'copy-container',
                  label: this.i18n.get('common_container_label'),
                  routerLink: '/explore/copy-data/container',
                  hidden: this.appUtilsService.isDistributed,
                  childrenLink: [
                    RouterUrl.ExploreCopyDataKubernetes,
                    RouterUrl.ExploreCopyDataKubernetesContainer
                  ]
                },
                {
                  id: 'copy-cloud',
                  label: this.i18n.get('common_huawei_clouds_label'),
                  routerLink: '/explore/copy-data/cloud',
                  childrenLink: [
                    RouterUrl.ExploreCopyDataHuaweiStack,
                    RouterUrl.ExploreCopyDataOpenStack,
                    RouterUrl.ExploreCopyDataGaussdbForOpengauss,
                    RouterUrl.ExploreCopyDataApsaraStack
                  ]
                },
                {
                  id: 'copy-application',
                  label: this.i18n.get('common_application_label'),
                  routerLink: '/explore/copy-data/application',
                  hidden: this.appUtilsService.isDistributed,
                  childrenLink: [
                    RouterUrl.ExploreCopyDataActiveDirectory,
                    RouterUrl.ExploreCopyDataDatabaseExchange,
                    RouterUrl.ExploreCopyDataSapHana,
                    RouterUrl.ExploreCopyDataSaponoracle
                  ]
                },
                {
                  id: 'copy-file-service',
                  label: this.i18n.get('common_file_systems_label'),
                  routerLink: '/explore/copy-data/file-service',
                  childrenLink: [
                    RouterUrl.ExploreCopyDataFileSystem,
                    RouterUrl.ExploreCopyDataNasShared,
                    RouterUrl.ExploreCopyDataNdmp,
                    RouterUrl.ExploreCopyDataCommonShare,
                    RouterUrl.ExploreCopyDataObject,
                    RouterUrl.ExploreCopyDataFileset,
                    RouterUrl.ExploreCopyDataVolume
                  ]
                }
              ]
            },
            {
              id: 'local-file-system-copy-data-group',
              label: this.i18n.get('common_copies_data_label'),
              type: 'group',
              items: [
                {
                  id: 'local-file-system-copy-data',
                  label: this.i18n.get('common_local_file_system_label'),
                  routerLink: RouterUrl.ExploreCopyLocalFileSystem
                }
              ],
              hidden: !includes(
                [
                  DataMap.Deploy_Type.cloudbackup.value,
                  DataMap.Deploy_Type.cloudbackup2.value
                ],
                this.i18n.get('deploy_type')
              )
            },
            {
              id: 'live-mount',
              label: this.i18n.get('common_live_mount_label'),
              type: 'group',
              description: this.i18n.get(
                'common_comer_guidance_live_mount_tips_label'
              ),
              items: [
                {
                  id: 'live-mount-app',
                  label: this.i18n.get('common_application_type_label'),
                  routerLink: RouterUrl.ExploreLiveMountApplication,
                  childrenLink: [
                    RouterUrl.ExploreLiveMountApplicationFileset,
                    RouterUrl.ExploreLiveMountApplicationVolume,
                    RouterUrl.ExploreLiveMountApplicationOracle,
                    RouterUrl.ExploreLiveMountApplicationMysql,
                    RouterUrl.ExploreLiveMountApplicationTdsql,
                    RouterUrl.ExploreLiveMountApplicationVmware,
                    RouterUrl.ExploreLiveMountApplicationCnware,
                    RouterUrl.ExploreLiveMountApplicationFileSystem,
                    RouterUrl.ExploreLiveMountApplicationNasshare
                  ]
                },
                {
                  id: 'policy-mount-update-policy',
                  label: this.i18n.get('common_mount_update_policy_label'),
                  routerLink: RouterUrl.ExplorePolicyMountUpdatePolicy
                }
              ]
            },
            {
              id: 'recovery-drill-group',
              label: this.i18n.get('explore_recovery_drill_label'),
              hidden:
                !this.isDataBackup &&
                !this.appUtilsService.isDistributed &&
                !this.appUtilsService.isDecouple &&
                !this.appUtilsService.isOpenOem &&
                !this.appUtilsService.isOpenServer,
              type: 'group',
              description: this.i18n.get(
                'common_comer_guidance_recovery_drill_tips_label'
              ),
              items: [
                {
                  id: 'recovery-drill',
                  label: this.i18n.get('explore_recovery_drill_label'),
                  routerLink: RouterUrl.ExploreRecoveryDrill,
                  hidden:
                    !this.isDataBackup &&
                    !this.appUtilsService.isDistributed &&
                    !this.appUtilsService.isDecouple,
                  childrenLink: [
                    RouterUrl.ExploreRecoveryDrill,
                    RouterUrl.ExploreCreateDrill,
                    RouterUrl.ExploreModifyDrill,
                    RouterUrl.ExploreDrillDetail,
                    RouterUrl.ExploreDrillExecuteLog
                  ]
                }
              ]
            }
          ]
        },
        {
          id: 'data-security',
          icon: 'aui-menu-security',
          label: this.i18n.get('protection_data_security_label'),
          hidden: [
            DataMap.Deploy_Type.cloudbackup.value,
            DataMap.Deploy_Type.cloudbackup2.value,
            DataMap.Deploy_Type.e6000.value
          ].includes(this.i18n.get('deploy_type')),
          items: [
            {
              id: 'file-interception',
              label: this.i18n.get('explore_file_block_label'),
              routerLink:
                RouterUrl.ExploreAntiRansomwareProtectionFileInterception
            },
            {
              id: 'real-time-detection',
              label: this.i18n.get('explore_real_time_detection_new_label'),
              routerLink:
                RouterUrl.ExploreAntiRansomwareProtectionRealTimeDetection
            },
            {
              id: 'data-backup',
              label: this.i18n.get('explore_intelligent_detection_label'),
              routerLink: RouterUrl.ExploreAntiRansomwareProtectionDataBackup
            },
            {
              id: 'detection-model',
              label: this.i18n.get('explore_detection_models_new_label'),
              routerLink: RouterUrl.ExploreAntiRansomwareProtectionModel
            },
            {
              id: 'anti-ransomware',
              label:
                this.i18n.get('deploy_type') === DataMap.Deploy_Type.x3000.value
                  ? this.i18n.get('common_worm_policy_label')
                  : this.i18n.get('common_anti_ransomware_label'),
              hidden: [
                DataMap.Deploy_Type.cloudbackup.value,
                DataMap.Deploy_Type.cloudbackup2.value
              ].includes(this.i18n.get('deploy_type')),
              description:
                this.i18n.get('deploy_type') === DataMap.Deploy_Type.x3000.value
                  ? this.i18n.get(
                      'common_comer_guidance_worm_policy_tips_label'
                    )
                  : this.i18n.get(
                      'common_comer_guidance_anti-ransomware_tips_label'
                    ),
              type: 'group',
              items: [
                {
                  id: 'anti-ransomware-overview',
                  label: this.i18n.get('explore_detection_overview_label'),
                  hidden:
                    this.i18n.get('deploy_type') ===
                    DataMap.Deploy_Type.x3000.value,
                  routerLink: RouterUrl.ExploreAntiRansomware
                },
                {
                  id: 'anti-ransomware-cloud-backup-overview',
                  label: this.i18n.get('common_summary_label'),
                  hidden:
                    this.i18n.get('deploy_type') ===
                    DataMap.Deploy_Type.x3000.value,
                  routerLink: RouterUrl.ExploreRansomwareCloudBackupOverview
                },
                {
                  id: 'anti-ransomware-detection-setting',
                  label: this.i18n.get(
                    'explore_ransomware_dectetion_setting_label'
                  ),
                  hidden:
                    this.i18n.get('deploy_type') ===
                    DataMap.Deploy_Type.x3000.value,
                  routerLink: RouterUrl.ExploreRansomwareDetectionSetting
                },
                {
                  id: 'anti-ransomware-blocking-rule-list',
                  label: this.i18n.get('explore_file_blocking_label'),
                  hidden:
                    !(this.isHyperdetect && SupportLicense.isFile) ||
                    this.i18n.get('deploy_type') ===
                      DataMap.Deploy_Type.x3000.value,
                  routerLink: RouterUrl.ExploreRansomwareBlockingRuleList
                },
                {
                  id: 'anti-ransomware-real-time-detect',
                  label: this.i18n.get('explore_real_time_detection_label'),
                  hidden:
                    !(this.isHyperdetect && SupportLicense.isFile) ||
                    this.i18n.get('deploy_type') ===
                      DataMap.Deploy_Type.x3000.value,
                  routerLink: RouterUrl.ExploreRansomwareRealtimeDetection
                },
                {
                  id: 'anti-ransomware-detection-model-list',
                  label: this.i18n.get('explore_detection_model_label'),
                  hidden:
                    this.i18n.get('deploy_type') ===
                    DataMap.Deploy_Type.x3000.value,
                  routerLink: RouterUrl.ExploreRansomwareDetectionModelList
                },
                {
                  id: 'anti-ransomware-policy',
                  label:
                    this.i18n.get('deploy_type') ===
                    DataMap.Deploy_Type.x3000.value
                      ? this.i18n.get('common_worm_policy_label')
                      : this.i18n.get('common_anti_policy_setting_label'),
                  routerLink: RouterUrl.ExplorePolicyAntiPolicySetting
                }
              ]
            },
            {
              id: 'anti-detection-result-group',
              label: this.i18n.get('explore_ransomware_detection_result_label'),
              type: 'group',
              items: [
                {
                  id: 'anti-detection-result',
                  label: this.i18n.get(
                    'explore_ransomware_detection_result_label'
                  ),
                  routerLink: RouterUrl.ExploreRansomwareLocalFileSystem
                }
              ],
              hidden: [
                DataMap.Deploy_Type.cloudbackup.value,
                DataMap.Deploy_Type.cloudbackup2.value,
                DataMap.Deploy_Type.x3000.value
              ].includes(this.i18n.get('deploy_type'))
            },
            {
              id: 'airgap-group',
              label: this.i18n.get('Air Gap'),
              hidden: this.isCyberEngine, // OceanCyber的AirGap独立成一级菜单了
              type: 'group',
              description: this.i18n.get(
                'common_comer_guidance_air_gap_tips_label'
              ),
              items: [
                {
                  id: 'airgap',
                  label: this.i18n.get('Air Gap'),
                  hidden: this.isCyberEngine,
                  routerLink: RouterUrl.ExplorePolicyAirgap
                }
              ]
            },
            {
              id: 'data-desensitization',
              label: this.i18n.get('common_data_desensitization_label'),
              description: this.i18n.get(
                'common_comer_guidance_data_desensitization_tips_label'
              ),
              hidden:
                this.i18n.get('deploy_type') ===
                DataMap.Deploy_Type.x3000.value,
              type: 'group',
              items: [
                {
                  id: 'data-desensitization-oracle',
                  label: this.i18n.get('common_oracle_label'),
                  routerLink: RouterUrl.ExploreDataDesensitizationOracle
                },
                {
                  id: 'policy-desensitization-policy',
                  label: this.i18n.get('common_desensitization_policy_label'),
                  hidden:
                    this.i18n.get('deploy_type') ===
                    DataMap.Deploy_Type.x3000.value,
                  routerLink: RouterUrl.ExplorePolicyDesensitizationPolicy
                }
              ]
            }
          ]
        },
        {
          id: 'cyber-airgap',
          icon: 'aui-menu-air-gap',
          label: this.i18n.get('Air Gap'),
          hidden: !this.isCyberEngine,
          routerLink: RouterUrl.ExplorePolicyAirgap
        },
        {
          id: 'snapshot-data',
          icon: 'aui-menu-snapshot-management',
          hidden: !this.isCyberEngine,
          label: this.i18n.get('common_snapshot_management_restoration_label'),
          routerLink: RouterUrl.ExploreSnapShotData
        },
        {
          id: 'detection-report',
          icon: 'aui-menu-report',
          hidden: !this.isCyberEngine,
          label: this.i18n.get('explore_desensitization_reports_label'),
          routerLink: RouterUrl.ExploreDetectionReport
        },
        {
          id: 'jobs',
          icon: 'aui-menu-job',
          label: this.i18n.get('common_jobs_label'),
          routerLink: '/insight/jobs',
          description: this.i18n.get(
            'common_comer_guidance_menu_job_tips_label'
          )
        },
        {
          id: 'alarms',
          icon: 'aui-menu-alarm',
          label: this.i18n.get('common_alarms_events_label'),
          routerLink: '/insight/alarms',
          description: this.i18n.get(
            'common_comer_guidance_menu_alarm_tips_label'
          )
        },
        {
          id: 'reports',
          icon: 'aui-menu-report',
          label: this.i18n.get('common_report_label'),
          routerLink: '/insight/reports',
          description: this.i18n.get(
            'common_comer_guidance_menu_report_tips_label'
          ),
          hidden: !includes(
            [
              DataMap.Deploy_Type.x3000.value,
              DataMap.Deploy_Type.x6000.value,
              DataMap.Deploy_Type.x8000.value,
              DataMap.Deploy_Type.a8000.value,
              DataMap.Deploy_Type.e6000.value,
              DataMap.Deploy_Type.x9000.value,
              DataMap.Deploy_Type.decouple.value,
              DataMap.Deploy_Type.openOem.value,
              DataMap.Deploy_Type.openServer.value
            ],
            this.i18n.get('deploy_type')
          )
        },
        {
          id: 'performance',
          label: this.i18n.get('common_performance_label'),
          icon: 'aui-menu-performance',
          routerLink: '/insight/performance',
          description: this.i18n.get(
            'common_comer_guidance_performance_tips_label'
          ),
          hidden:
            this.i18n.get('deploy_type') ===
            DataMap.Deploy_Type.hyperdetect.value
        },
        {
          id: 'storage-device',
          icon: 'aui-menu-storage-device',
          hidden: !this.isCyberEngine,
          label: this.i18n.get('protection_storage_devices_label'),
          routerLink: RouterUrl.ExploreStorageDevice
        },
        {
          id: 'system',
          icon: 'aui-menu-system',
          label: this.i18n.get('common_system_label'),
          items: [
            {
              id: 'infrastructure',
              label: this.i18n.get('common_infrastructure_label'),
              type: 'group',
              description: this.i18n.get(
                'common_comer_guidance_infrastructure_tips_label'
              ),
              items: [
                {
                  id: 'cluster-management',
                  label: this.i18n.get('system_cluster_management_label'),
                  routerLink: '/system/infrastructure/cluster-management'
                },
                {
                  id: 'local-storage',
                  label: this.i18n.get('system_local_storage_label'),
                  hidden: !includes(
                    [
                      DataMap.Deploy_Type.hyperdetect.value,
                      DataMap.Deploy_Type.cloudbackup.value,
                      DataMap.Deploy_Type.cloudbackup2.value
                    ],
                    this.i18n.get('deploy_type')
                  ),
                  routerLink: '/system/infrastructure/local-storage'
                },
                {
                  id: 'archive-storage',
                  label: this.i18n.get(
                    'system_archive_storage_warehouse_label'
                  ),
                  routerLink: '/system/infrastructure/archive-storage'
                },
                {
                  id: 'backup-storage',
                  label: this.i18n.get('common_backup_storage_label'),
                  routerLink: '/system/infrastructure/backup-storage'
                },
                {
                  id: 'nas-backup-storage',
                  label: this.i18n.get(
                    'system_backup_storage_unit_group_label'
                  ),
                  routerLink: '/system/infrastructure/nas-backup-storage'
                },
                {
                  id: 'hcs-storage',
                  label: this.i18n.get('system_hcs_storage_label'),
                  routerLink: '/system/infrastructure/hcs-storage',
                  hidden: !this.isHcsEnvir
                }
              ]
            },
            {
              id: 'security',
              label: this.i18n.get('common_security_label'),
              type: 'group',
              description: this.i18n.get(
                'common_comer_guidance_security_tips_label'
              ),
              items: [
                {
                  id: 'rbac',
                  label: this.i18n.get('system_rbac_label'),
                  hidden: this.isCloudBackup || this.isCyberEngine,
                  routerLink: '/system/security/rbac'
                },
                {
                  id: 'userrole',
                  label: this.i18n.get('system_user_role_label'),
                  hidden: !(this.isCloudBackup || this.isCyberEngine),
                  routerLink: '/system/security/userrole'
                },
                {
                  id: 'userQuota',
                  label: this.i18n.get('system_user_quota_label'),
                  routerLink: '/system/security/user-quota'
                },
                {
                  id: 'securitypolicy',
                  label: this.i18n.get('system_security_policy_label'),
                  routerLink: '/system/security/securitypolicy'
                },
                {
                  id: 'certificate',
                  label: this.i18n.get('system_certificate_label'),
                  routerLink: '/system/security/certificate'
                },
                {
                  id: 'kerberos',
                  label: this.i18n.get('Kerberos'),
                  routerLink: '/system/security/kerberos'
                },
                {
                  id: 'dataSecurity',
                  label: this.i18n.get('system_data_security_label'),
                  hidden: this.appUtilsService.isDistributed,
                  routerLink: '/system/security/dataSecurity'
                },
                {
                  id: 'hostTrustworthiness',
                  label: this.i18n.get('common_host_trustworthiness_op_label'),
                  routerLink: '/system/security/hostTrustworthiness'
                },
                {
                  id: 'ldapConfig',
                  label: this.i18n.get('system_ldap_service_config_label'),
                  routerLink: '/system/security/ldapService'
                },
                {
                  id: 'samlSsoConfig',
                  label: this.i18n.get('system_saml_sso_config_label'),
                  routerLink: '/system/security/samlSsoConfig',
                  hidden: this.appUtilsService.isDistributed
                },
                {
                  id: 'adfsConfig',
                  label: this.i18n.get('system_adfs_label'),
                  routerLink: '/system/security/adfsConfig'
                }
              ]
            },
            {
              id: 'license-group',
              hidden: !includes(
                [
                  DataMap.Deploy_Type.decouple.value,
                  DataMap.Deploy_Type.openServer.value
                ],
                this.i18n.get('deploy_type')
              ),
              type: 'group',
              items: [
                {
                  id: 'license',
                  label: this.i18n.get('common_license_label'),
                  hidden: this.isCyberEngine,
                  routerLink: '/system/license'
                }
              ]
            },
            {
              id: 'log-management-group',
              label: this.i18n.get('common_log_label'),
              type: 'group',
              description: this.i18n.get(
                'common_comer_guidance_log_management_tips_label'
              ),
              items: [
                {
                  id: 'log-management',
                  label: this.i18n.get('common_log_management_label'),
                  routerLink: '/system/log-management'
                },
                {
                  id: 'export-query',
                  label: this.i18n.get('common_export_query_label'),
                  routerLink: '/system/export-query'
                }
              ]
            },
            {
              id: 'settings',
              label: this.i18n.get('common_setting_label'),
              type: 'group',
              description: this.i18n.get(
                'common_comer_guidance_settings_tips_label'
              ),
              items: [
                {
                  id: 'cyber-license-management',
                  label: this.i18n.get('common_license_management_label'),
                  hidden: !this.isCyberEngine,
                  routerLink: RouterUrl.SystemLicense
                },
                {
                  id: 'cyber-network-config',
                  label: this.i18n.get('common_network_config_label'),
                  hidden: !this.isCyberEngine,
                  routerLink: RouterUrl.SystemNetworkConfig
                },
                {
                  id: 'tag-management',
                  label: this.i18n.get('system_tag_management_label'),
                  routerLink: RouterUrl.SystemTagManagement
                },
                {
                  id: 'system-backup',
                  label: this.i18n.get('common_management_data_backup_label'),
                  routerLink: '/system/settings/system-backup'
                },
                {
                  id: 'alarm-notify',
                  label: this.i18n.get('system_alarm_notification_label'),
                  routerLink: '/system/settings/alarm-notify'
                },
                {
                  id: 'alarm-notify-settings',
                  hidden: !includes(
                    [
                      DataMap.Deploy_Type.cyberengine.value,
                      DataMap.Deploy_Type.decouple.value,
                      DataMap.Deploy_Type.e6000.value
                    ],
                    this.i18n.get('deploy_type')
                  ),
                  label: this.i18n.get('system_alarm_term_notify_label'),
                  routerLink: '/system/settings/alarm-notify-settings'
                },
                {
                  id: 'alarm-settings',
                  hidden: !includes(
                    [
                      DataMap.Deploy_Type.hyperdetect.value,
                      DataMap.Deploy_Type.cloudbackup.value,
                      DataMap.Deploy_Type.cloudbackup2.value
                    ],
                    this.i18n.get('deploy_type')
                  ),
                  label: this.i18n.get('system_alarm_settings_label'),
                  routerLink: '/system/settings/alarm-settings'
                },
                {
                  id: 'alarm-dump',
                  label: this.i18n.get('system_event_dump_label'),
                  routerLink: '/system/settings/alarm-dump'
                },
                {
                  id: 'snmp-trap',
                  hidden: !includes(
                    [
                      DataMap.Deploy_Type.x8000.value,
                      DataMap.Deploy_Type.a8000.value,
                      DataMap.Deploy_Type.x3000.value,
                      DataMap.Deploy_Type.x6000.value,
                      DataMap.Deploy_Type.x9000.value,
                      DataMap.Deploy_Type.cyberengine.value,
                      DataMap.Deploy_Type.e6000.value,
                      DataMap.Deploy_Type.decouple.value,
                      DataMap.Deploy_Type.openOem.value,
                      DataMap.Deploy_Type.openServer.value
                    ],
                    this.i18n.get('deploy_type')
                  ),
                  label: this.i18n.get('system_snmp_trap_label'),
                  routerLink: '/system/settings/snmp-trap'
                },
                {
                  id: 'sftp-service',
                  label: this.i18n.get('system_sftp_label'),
                  routerLink: '/system/settings/sftp-service',
                  hidden: includes(
                    [
                      DataMap.Deploy_Type.x3000.value,
                      DataMap.Deploy_Type.e6000.value,
                      DataMap.Deploy_Type.decouple.value,
                      DataMap.Deploy_Type.openServer.value
                    ],
                    this.i18n.get('deploy_type')
                  )
                },
                {
                  id: 'ibmc',
                  label: this.i18n.get('iBMC'),
                  routerLink: '/system/settings/ibmc',
                  hidden: !includes(
                    [DataMap.Deploy_Type.cyberengine.value],
                    this.i18n.get('deploy_type')
                  )
                },
                {
                  id: 'system-time',
                  label: this.i18n.get('system_device_time_label'),
                  hidden:
                    this.cookieService.role === RoleType.SysAdmin &&
                    !includes(
                      [
                        DataMap.Deploy_Type.hyperdetect.value,
                        DataMap.Deploy_Type.cloudbackup.value,
                        DataMap.Deploy_Type.cloudbackup2.value,
                        DataMap.Deploy_Type.cyberengine.value
                      ],
                      this.i18n.get('deploy_type')
                    ),
                  routerLink: '/system/settings/system-time'
                },
                {
                  id: 'config-network',
                  label: this.i18n.get('common_network_config_label'),
                  hidden: !(
                    this.cookieService.role === RoleType.SysAdmin &&
                    includes(
                      [
                        DataMap.Deploy_Type.x8000.value,
                        DataMap.Deploy_Type.a8000.value,
                        DataMap.Deploy_Type.x3000.value,
                        DataMap.Deploy_Type.x6000.value,
                        DataMap.Deploy_Type.x9000.value,
                        DataMap.Deploy_Type.openOem.value
                      ],
                      this.i18n.get('deploy_type')
                    )
                  ),
                  routerLink: '/system/settings/config-network'
                },
                {
                  id: 'service-oriented-nms',
                  label: this.i18n.get('system_dme_access_setting_label'),
                  routerLink: '/system/settings/service-oriented-nms',
                  hidden: !includes(
                    [
                      DataMap.Deploy_Type.x8000.value,
                      DataMap.Deploy_Type.x3000.value,
                      DataMap.Deploy_Type.x6000.value,
                      DataMap.Deploy_Type.x9000.value,
                      DataMap.Deploy_Type.e6000.value,
                      DataMap.Deploy_Type.decouple.value
                    ],
                    this.i18n.get('deploy_type')
                  )
                }
              ]
            },
            {
              id: 'external-associated-systems-group',
              hidden: !(
                this.isDataBackup ||
                this.appUtilsService.isDecouple ||
                this.appUtilsService.isDistributed ||
                this.appUtilsService.isOpenOem ||
                this.appUtilsService.isOpenServer
              ),
              type: 'group',
              description: this.i18n.get(
                'common_comer_guidance_external_associated_systems_tips_label'
              ),
              items: [
                {
                  id: 'external-associated-systems',
                  label: this.i18n.get(
                    'common_external_associated_systems_label'
                  ),
                  routerLink: '/system/external-associated-systems'
                }
              ]
            }
          ]
        }
      ];
      this.menus = getAccessibleMenu(menus, this.cookieService, this.i18n);
    });
  }

  private processMenuGuideData() {
    if (!isEmpty(this.guideTipsMenu)) {
      return;
    }
    let currentStepIndex = this.guideService.getGuideStepSize() + 1;
    const createGuideItem = (
      id: string,
      step: number,
      name: string,
      description: string | any[]
    ) => {
      let tempName = name;
      if (id === 'external-associated-systems-group') {
        // 外部关联系统虽然单独是一个分组，但是没有设置分组标题，所以需要特殊处理一下
        tempName = this.i18n.get('common_external_associated_systems_label');
      }
      return {
        id,
        step,
        name: tempName,
        description,
        isGroup: isArray(description)
      };
    };
    const menus = this.menus.filter(item => !item?.hidden);
    menus.forEach(menu => {
      if (menu?.description) {
        this.guideTipsMenu.push(
          createGuideItem(
            menu.id,
            currentStepIndex++,
            menu.label,
            menu.description
          )
        );
      } else if (menu?.items?.length) {
        const childItems = menu.items
          .filter(
            childItem =>
              !isUndefined(childItem.description) && !childItem?.hidden
          )
          .map(child =>
            createGuideItem(
              child.id,
              currentStepIndex,
              child.label,
              child.description
            )
          );
        this.guideTipsMenu.push(
          createGuideItem(menu.id, currentStepIndex++, menu.label, childItems)
        );
      }
    });
    this.guideService.addStepData(this.guideTipsMenu);
  }

  setPollingFn() {
    clearTimeout(this.jobTimeout);
    clearTimeout(this.recentjobTimeout);
    clearTimeout(this.sessionTimeout);
    clearTimeout(this.criticalAlarmTimeout);
    this.getRecentAlarms();
    this.getSessionOut();
    this.runningTaskPolling();
    this.criticalAlarmPolling();
  }

  routeChange() {
    this.router.events.subscribe(event => {
      if (event instanceof NavigationEnd) {
        this.cyberDarkHeader =
          this.isCyberEngine && includes([RouterUrl.Home], this.router.url);
        // Oceancyber首页加载字体是浅色，通过动态添加类实现
        if (includes([RouterUrl.Home], this.router.url) && this.isCyberEngine) {
          document.body.classList?.add('light-loading');
        } else {
          document.body.classList?.remove('light-loading');
        }
        this.drawModalService.destroyAllModals();
        this.getAppThemeKey();
        this.isLogin = event.urlAfterRedirects.includes('login');
        this.isErrorPage = event.urlAfterRedirects.includes('error-page');
        this.isReportDetail = event.urlAfterRedirects.includes('report-detail');
        if (this.isLogin) {
          this.messageService.destroy();
          return;
        }

        const currentCluster = JSON.parse(
          decodeURIComponent(this.cookieService.get('currentCluster'))
        );
        if (
          !this.isDmeUser &&
          !this.isHcsUser &&
          this.isDataBackup &&
          event.urlAfterRedirects !== '/home' &&
          (isEmpty(currentCluster) || currentCluster?.isAllCluster === true)
        ) {
          if (isEmpty(this.clusterOptions)) {
            this.clusterApiService
              .getClustersInfoUsingGET({
                startPage: CommonConsts.PAGE_START,
                pageSize: CommonConsts.PAGE_SIZE * 10,
                akLoading: false,
                clustersId: '1',
                clustersType: '1',
                roleList: [
                  DataMap.Target_Cluster_Role.managed.value,
                  DataMap.Target_Cluster_Role.management.value
                ]
              })
              .subscribe(res => {
                const localCluster = cloneDeep(
                  find(res.records, {
                    clusterType: DataMap.Cluster_Type.local.value
                  })
                );
                if (
                  !includes(
                    localCluster.clusterName,
                    `(${this.i18n.get('common_local_label')})`
                  )
                ) {
                  const tmpClusterName =
                    localCluster.clusterName +
                    `(${this.i18n.get('common_local_label')})`;
                  set(localCluster, 'clusterName', tmpClusterName);
                }

                if (localCluster) {
                  this.currentCluster = localCluster;
                }
                this.clusterOptions = res.records;
              });
          } else {
            const localCluster = cloneDeep(
              find(this.clusterOptions, {
                clusterType: DataMap.Cluster_Type.local.value
              })
            );

            if (
              !includes(
                localCluster.clusterName,
                `(${this.i18n.get('common_local_label')})`
              )
            ) {
              const tmpClusterName =
                localCluster.clusterName +
                `(${this.i18n.get('common_local_label')})`;
              set(localCluster, 'clusterName', tmpClusterName);
            }

            if (localCluster) {
              this.currentCluster = localCluster;
            }
          }
          this.isMultiCluster = false;
        }

        if (
          this.isDataBackup &&
          event.urlAfterRedirects === '/home' &&
          (isEmpty(currentCluster) || currentCluster?.isAllCluster === true)
        ) {
          this.currentCluster = {
            clusterId: DataMap.Cluster_Type.local.value,
            clusterName: this.i18n.get('common_all_clusters_label'),
            clusterType: DataMap.Cluster_Type.local.value
          };
          this.isMultiCluster = true;
        }

        if (
          !find(this.protectionRouterUrlList, item =>
            includes(event.urlAfterRedirects, item)
          )
        ) {
          SUB_APP_REFRESH_FLAG.emit = false;
        }

        this.tempToken = this.cookieService.get('_OP_TOKEN_');
        this.isHcsEnvir =
          this.cookieService.get('serviceProduct') ===
          CommonConsts.serviceProduct;
        this.needGuideProduct =
          this.needGuideProduct && this.cookieService.role !== RoleType.Auditor;
        this.getMenus();
        this.closeTaskPopover();
        this.showGuidePop();
        // 获取自定义版本
        this.getCustomVersion();
      }
    });
  }

  setPermission(res): Observable<void> {
    return new Observable<void>((observer: Observer<void>) => {
      this.cookieService.setRole(res.rolesSet[0].roleId);
      this.cookieService.set('role', res.rolesSet[0].roleId);
      // 发布用户信息流
      this.globalService.emitBehaviorStore({
        action: 'userInfo',
        state: res
      });

      // 发布用户操作权限
      this.globalService.setViewPermission({
        action: 'viewPermission',
        state: getAccessibleViewList(this.cookieService.role)
      });
      observer.next();
      observer.complete();
    });
  }

  getUser(): Observable<any> {
    return new Observable<any>((observer: Observer<any>) => {
      const userId = this.cookieService.get('userId');
      if (isEmpty(userId)) {
        return;
      }
      this.usersApiService
        .getUsingGET2({ userId, akDoException: false })
        .pipe(map(res => res || { rolesSet: [{ roleId: RoleType.Null }] }))
        .subscribe({
          next: res => {
            this.userName = res.userName;
            this.userType = res.userType;
            this.isHcsUser = res.userType === CommonConsts.HCS_USER_TYPE;
            this.isDmeUser = res.userType === CommonConsts.DME_USER_TYPE;
            observer.next(res);
            observer.complete();
          },
          error: () => {
            // HCS服务化
            if (
              !isEmpty(get(window, 'parent.hcsData.ProjectName', '')) ||
              this.cookieService.get('projectId') ||
              !isEmpty(get(window, 'parent.hcsData.AgencyId', ''))
            ) {
              this.messageService.error(
                this.i18n.get('common_exception_label'),
                {
                  lvMessageKey: 'hcsLoginError',
                  lvShowCloseButton: true
                }
              );
              return;
            }
            this.timeoutLogout();
          }
        });
    });
  }

  listenStoragechange() {
    window.addEventListener('storage', () => {
      if (this.isLogin || this.isErrorPage) {
        return;
      }
      const nowToken = this.cookieService.get('_OP_TOKEN_');
      if (this.tempToken && nowToken && this.tempToken !== nowToken) {
        window.location.reload();
      }
    });
  }

  calculatTimeout(sessionTime) {
    const SESSION_TIME_OUT = sessionTime * 60 * 1e3;
    const startTime = Number(this.cookieService.get(this.TIMEOUT_STATUS_KEY));
    const endTime = new Date().getTime();
    if (endTime - startTime > SESSION_TIME_OUT) {
      clearTimeout(this.sessionTimeout);
      this.timeoutLogout();
      window.removeEventListener('mousedown', e => {
        this.cookieService.remove('sessionIdleTime');
      });
    }
  }

  getSessionOut() {
    window.addEventListener('mousedown', e => {
      this.cookieService.set(
        this.TIMEOUT_STATUS_KEY,
        `${new Date().getTime()}`
      );
    });
    this.cookieService.set(this.TIMEOUT_STATUS_KEY, `${new Date().getTime()}`);
    this.getSessionTime();
  }

  getSessionTime() {
    if (this.isHcsUser || this.isDmeUser) {
      return;
    }
    const userId = this.cookieService.get('userId');
    if (!userId) {
      clearTimeout(this.sessionTimeout);
      this.timeoutLogout();
    }
    this.securityApiService
      .getUsingGET1({ akLoading: false, akDoException: false })
      .subscribe({
        next: res => {
          this.calculatTimeout(res.sessionTime);
          this.seesionTimeOut = res.sessionTime;
          this.sessionTimeout = setTimeout(() => {
            clearTimeout(this.sessionTimeout);
            this.getSessionTime();
          }, this.SESSION_TIMER);
        },
        error: () => {
          this.calculatTimeout(this.seesionTimeOut);
          this.sessionTimeout = setTimeout(() => {
            clearTimeout(this.sessionTimeout);
            this.getSessionTime();
          }, this.SESSION_TIMER);
        }
      });
  }

  toLogin() {
    clearTimeout(this.jobTimeout);
    clearTimeout(this.recentjobTimeout);
    clearTimeout(this.sessionTimeout);
    clearTimeout(this.criticalAlarmTimeout);
    this.cookieService.removeAll(this.i18n.languageKey);
    localStorage.removeItem(this.userName);
    this.messageBox.error({
      lvModalKey: 'errorMsgKey',
      lvHeader: this.i18n.get('common_error_label'),
      lvContent: this.i18n.get('common_timeout_logout_label'),
      lvAfterOpen: modal => {
        // tslint:disable-next-line: no-shadowed-variable
        let timer = 10;
        modal.lvOkText = `${this.i18n.get('common_ok_label')}(` + timer + `s)`;
        const interval = setInterval(() => {
          timer--;
          modal.lvOkText =
            `${this.i18n.get('common_ok_label')}(` + timer + `s)`;
        }, 1e3);

        const timeOut = setTimeout(() => {
          clearInterval(interval);
          clearTimeout(timeOut);
          modal.close();
        }, 10 * 1e3);
      },
      lvAfterClose: () => {
        this.router.navigateByUrl('/login').then(() => {
          window.location.reload();
        });
      }
    });
  }

  timeoutLogout() {
    this.authApiService
      .logoutUsingPOST({
        akOperationTips: false,
        logoutType: LogoutType.Timeout,
        clustersType: toString(DataMap.Cluster_Type.local.value),
        clustersId: toString(DataMap.Cluster_Type.local.value)
      })
      .subscribe({
        next: () => this.toLogin(),
        error: () => this.toLogin()
      });
  }

  setLanguage() {
    this.language = this.getLanguage() === LANGUAGE.CN ? 'English' : '简体中文';
    return this;
  }

  /**
   * 替换 favicon 图标
   */
  setFavicon() {
    const links = document.head.getElementsByTagName('link');
    const faviconRel = 'shortcut icon';
    each(links, link => {
      if (link.rel === faviconRel) {
        link.href = this.whitebox.isWhitebox
          ? `${IMAGE_PATH_PREFIX}logo.ico`
          : this.appUtilsService.isOpenOem || this.appUtilsService.isOpenServer
          ? '/console/assets/img/open-eBackup.ico'
          : '/console/favicon.ico';
      }
    });
  }

  toggleLanguage() {
    this.i18n.changeLanguage(
      this.getLanguage() === LANGUAGE.CN ? LANGUAGE.EN : LANGUAGE.CN
    );
  }

  getLanguage() {
    return this.i18n.language.toLowerCase();
  }

  parseJob(res, params) {
    if (
      (params && this.latest === 'latest') ||
      (!params && this.latest === 'unfinished')
    ) {
      return;
    }
    this.taskData = res.records;
    if (params) {
      this.runningTotal = res.totalCount;
    }
    this.taskData.forEach(task => {
      assign(task, {
        jobStatus:
          task.status === this.jobStatus.failed.value ? 'error' : 'normal'
      });
    });
  }

  runningTaskPolling() {
    if (this.isHcsUser || this.isDmeUser) {
      return;
    }
    clearTimeout(this.jobTimeout);
    const taskApiCall: Observable<any> =
      this.isMultiCluster && this.isDataBackup
        ? this.multiClustersServiceApi.getMultiClusterJobs({
            akLoading: false,
            jobPeriod: 'all'
          })
        : this.jobAPIService.summaryUsingGET({
            akLoading: false,
            akDoException: false
          });
    taskApiCall.subscribe(
      (res: any) => {
        this.runningTaskCount =
          res.running + res.pending + res.ready + (res.aborting || 0);
        this.runningTotal = this.runningTaskCount;
      },
      () => {
        this.jobTimeout = setTimeout(() => {
          clearTimeout(this.jobTimeout);
          this.runningTaskPolling();
        }, CommonConsts.TIME_INTERVAL_JOB_COUNT);
      }
    );
  }

  getRecentTasks(loading?) {
    if (this.isHcsUser || this.isDmeUser) {
      return;
    }
    const statusList = [
      DataMap.Job_status.running.value,
      DataMap.Job_status.pending.value,
      DataMap.Job_status.initialization.value,
      DataMap.Job_status.aborting.value
    ];
    if (this.isDataBackup) {
      statusList.push(
        DataMap.Job_status.dispatching.value,
        DataMap.Job_status.redispatch.value
      );
    }
    const params = this.latest === 'unfinished' ? { statusList } : null;
    clearTimeout(this.recentjobTimeout);
    if (loading) {
      this.taskLoading = true;
    }
    const jobParams = assign(
      {
        akDoException: false,
        akLoading: false,
        isSystem: false,
        isVisible: true,
        orderBy: 'start_time',
        orderType: 'desc',
        pageSize: 10,
        startPage: 1
      },
      params
    );
    const apiService: Observable<any> =
      this.isMultiCluster && this.isDataBackup
        ? this.multiClustersServiceApi.getMultiClusterJobList(jobParams)
        : this.jobAPIService.queryJobsUsingGET(jobParams);
    apiService
      .pipe(
        finalize(() => {
          if (loading) {
            this.taskLoading = false;
          }
          this.recentjobTimeout = setTimeout(() => {
            clearTimeout(this.recentjobTimeout);
            this.getRecentTasks();
          }, CommonConsts.TIME_INTERVAL_JOB_COUNT);
        })
      )
      .subscribe(res => {
        if (loading) {
          this.taskLoading = false;
        }
        this.parseJob(res, params);
      });
  }

  refreshRecentAlarms() {
    this.getRecentAlarms();
    this.criticalAlarmPolling();
  }

  getRecentAlarms(params?) {
    if (this.isHcsUser || this.isDmeUser) {
      return;
    }
    if (this.isCyberEngine || this.isV1Alarm) {
      this.alarmApiService
        .getAlarmListUsingGET({
          pageNum: CommonConsts.PAGE_START,
          pageSize: 10,
          language: this.i18n.language === 'zh-cn' ? 'zh' : 'en',
          akLoading: false,
          akDoException: false
        })
        .subscribe(res => {
          each(res.records, item => {
            assign(item, {
              description: item.desc
            });
          });
          this.alarmData = res.records || [];
        });
    } else {
      this.alarmApiService
        .findPageUsingGET(
          assign(
            {
              startIndex: 0,
              pageSize: 10,
              pageNo: 0,
              isVisible: true,
              akLoading: false,
              akDoException: false,
              shouldAllNodes: true,
              language: this.i18n.language === 'zh-cn' ? 'ZH' : 'EN'
            },
            params
          )
        )
        .subscribe(res => {
          if (isArray(res?.records) && size(res.records) > 10) {
            res.records = res.records.slice(0, 10);
          }
          this.alarmData = res.records || [];
        });
    }
  }

  private _getCriticalAlarm(): Observable<any> {
    const commonParams = {
      akLoading: false,
      akDoException: false
    };
    return this.isCyberEngine || this.isV1Alarm
      ? this.alarmApiService.getAlarmListUsingGET({
          pageNum: CommonConsts.PAGE_START,
          pageSize: CommonConsts.PAGE_SIZE,
          language: this.i18n.language === 'zh-cn' ? 'zh' : 'en',
          ...commonParams
        })
      : this.isDataBackup
      ? this.isMultiCluster
        ? this.multiClustersServiceApi
            .getMultiClusterAlarms({
              akLoading: false
            })
            .pipe(
              map(res => {
                res['total'] = res.critical + res.major + res.warning;
                return res;
              })
            )
        : this.alarmApiService
            .queryAlarmCountBySeverityUsingGET({
              akLoading: false
            })
            .pipe(
              map(res => {
                res['total'] = res.critical + res.major + res.warning;
                return res;
              })
            )
      : this.alarmApiService
          .findPageUsingGET({
            pageNo: CommonConsts.PAGE_START,
            pageSize: CommonConsts.PAGE_SIZE,
            language: this.i18n.language === 'zh-cn' ? 'ZH' : 'EN',
            ...commonParams
          })
          .pipe(
            map(res => {
              res.total = res['totalCount'];
              return res;
            })
          );
  }

  criticalAlarmPolling() {
    if (this.isHcsUser || this.isDmeUser) {
      return;
    }
    if (this.criticalAlarmTimeout) {
      clearTimeout(this.criticalAlarmTimeout);
    }
    this._getCriticalAlarm().subscribe(res => {
      this.criticalAlarmCount = res.total;
      if (this.criticalAlarmTimeout) {
        clearTimeout(this.criticalAlarmTimeout);
      }
      this.criticalAlarmTimeout = setTimeout(() => {
        this.criticalAlarmPolling();
      }, CommonConsts.TIME_INTERVAL_ALARM_COUNT);
    });
  }

  jobIndexChange(e) {
    this.runningTaskPolling();
    this.getRecentTasks(true);
  }

  taskExternalTrigger() {
    clearTimeout(this.recentjobTimeout);
  }

  menuItemClick(event) {
    if (event.data.routerLink) {
      this.router.navigateByUrl(event.data.routerLink);
    }
  }

  toggleSearch() {
    this.router.navigate([{ outlets: { primary: ['search'] } }]);
  }

  activeMenuChange(id) {
    setTimeout(() => {
      this.activeId = id;
    });
  }

  logout() {
    this.messageBox.confirm({
      lvOkType: 'primary',
      lvCancelType: 'default',
      lvFocusButtonId: 'cancel',
      lvHeader: this.i18n.get('common_confirm_label'),
      lvContent: this.i18n.get('common_logout_confirm_label'),
      lvOk: () => this.logoutUser(LogoutType.Manual)
    });
  }

  logoutUser(logoutType: LogoutType.Manual | LogoutType.Timeout) {
    this.authApiService
      .logoutUsingPOST({
        akOperationTips: false,
        logoutType,
        clustersType: toString(DataMap.Cluster_Type.local.value),
        clustersId: toString(DataMap.Cluster_Type.local.value)
      })
      .subscribe(res => {
        if (this.userType === DataMap.loginUserType.adfs.value) {
          this.adfsService.adfsLoginForward({}).subscribe({
            next: (resData: any) => {
              window.open(
                resData?.logoutForwardUrl,
                '_blank',
                'scrollbars=yes,resizable=yes,statebar=no,width=400,height=400,left=200, top=100'
              );
              this.rediectLoginPage();
            },
            error: () => {
              this.rediectLoginPage();
            }
          });
        } else {
          this.rediectLoginPage();
        }
      });
  }

  rediectLoginPage() {
    clearTimeout(this.jobTimeout);
    clearTimeout(this.sessionTimeout);
    this.cookieService.removeAll(this.i18n.languageKey);
    localStorage.removeItem(this.userName);
    this.router.navigateByUrl('/login').then(() => {
      window.location.reload();
    });
  }

  exportQuery() {
    this.drawModalService.create(
      assign({}, MODAL_COMMON.generateDrawerOptions(), {
        lvType: 'modal',
        lvWidth: 900,
        lvHeader: this.i18n.get('common_export_query_label'),
        lvContent: ExportQueryResultsComponent,
        lvComponentParams: {},
        lvFooter: [
          {
            label: this.i18n.get('common_close_label'),
            onClick: modal => {
              modal.close();
            }
          }
        ]
      })
    );
  }

  modifyPwd() {
    this.drawModalService.create(
      assign({}, MODAL_COMMON.generateDrawerOptions(), {
        lvType: 'modal',
        lvModalKey: 'modify-pwd',
        lvOkDisabled: true,
        lvWidth: 500,
        lvHeight: 400,
        lvHeader: this.i18n.get('common_update_password_label'),
        lvContent: ModifyPasswordComponent,
        lvAfterOpen: modal => {
          const content = modal.getContentComponent() as ModifyPasswordComponent;
          const modalIns = modal.getInstance();
          content.passwdFormGroup.statusChanges.subscribe(res => {
            modalIns.lvOkDisabled = res !== 'VALID';
          });
        },
        lvOk: modal => {
          return new Promise(resolve => {
            const content = modal.getContentComponent() as ModifyPasswordComponent;
            this.usersApiService
              .updatePasswordUsingPUT({
                userId: this.cookieService.get('userId'),
                passwordRequest: content.passwdFormGroup.value
              })
              .subscribe({
                next: () => {
                  resolve(true);
                  clearTimeout(this.jobTimeout);
                  clearTimeout(this.recentjobTimeout);
                  clearTimeout(this.sessionTimeout);
                  clearTimeout(this.criticalAlarmTimeout);
                  this.cookieService.removeAll(this.i18n.languageKey);
                  localStorage.removeItem(this.userName);
                  this.router.navigateByUrl('/login');
                },
                error: error => resolve(false)
              });
          });
        }
      })
    );
  }

  closeTaskPopover() {
    if (this.taskPopover) {
      this.taskPopover.hide();
    }
  }

  closeAlarmPopover() {
    if (this.alarmPopover) {
      this.alarmPopover.hide();
    }
  }

  alarmDetailClick(alarm) {
    if (this.alarmPopover) {
      this.alarmPopover.hide();
    }
    ALARM_NAVIGATE_STATUS.sequence = alarm.sequence.toString();
    if (this.router.url === '/insight/alarms') {
      this.globalService.emitStore({
        action: 'getAlarmDetail',
        state: true
      });
      return;
    }
    this.router.navigate(['insight/alarms']);
  }

  openAbout() {
    this.modalService.create({
      lvModalKey: 'about-create',
      lvHeader: this.aboutHeaderTpl,
      lvContent: this.aboutContentTpl,
      lvFooter: this.aboutFooterTpl,
      lvWidth: 510,
      lvHeight: 330,
      lvOuterClosable: true,
      lvCloseButtonDisplay: false
    });
  }

  closeAbout() {
    this.modalService.destroyModal('about-create');
  }

  getHelpPath(): string {
    if (this.whitebox.isWhitebox) {
      return 'oem';
    } else if (
      includes(
        [DataMap.Deploy_Type.cloudbackup2.value],
        this.i18n.get('deploy_type')
      )
    ) {
      return 'cloudbackup';
    } else if (
      includes(
        [DataMap.Deploy_Type.hyperdetect.value],
        this.i18n.get('deploy_type')
      )
    ) {
      return SupportLicense.isSan ? 'hyperdetect_d' : 'hyperdetect_o';
    } else if (
      includes(
        [DataMap.Deploy_Type.cyberengine.value],
        this.i18n.get('deploy_type')
      )
    ) {
      return 'cyberengine';
    } else {
      return 'a8000';
    }
  }

  openHelp() {
    window.open(
      `/console/assets/help/${this.getHelpPath()}/${
        this.i18n.language
      }/index.html`,
      '_blank'
    );
  }

  getJobDetail(job) {
    if (this.taskPopover) {
      this.taskPopover.hide();
    }

    const jobTable = new JobTableComponent(
      this.appUtilsService,
      this.i18n,
      this.messageService,
      this.drawModalService,
      this.dataMapService,
      this.copyActionService,
      this.copiesDetectReportService,
      this.slaApiService
    );
    jobTable.getDetail(job);
  }

  toggleGuide() {
    this.showGuide = true;
  }

  closeGuide() {
    this.showGuide = false;
    clearUserGuideCache();
  }

  protected readonly SYSTEM_TIME = SYSTEM_TIME;
}
