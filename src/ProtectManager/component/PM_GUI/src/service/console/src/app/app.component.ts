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
  Component,
  NgZone,
  OnDestroy,
  OnInit,
  TemplateRef,
  ViewChild,
  ViewContainerRef
} from '@angular/core';
import { Title } from '@angular/platform-browser';
import { ActivatedRoute, NavigationEnd, Router } from '@angular/router';
import { MessageboxService, MessageService, ModalService } from '@iux/live';
import {
  ALARM_NAVIGATE_STATUS,
  BaseUtilService,
  CommonConsts,
  CookieService,
  DataMap,
  DataMapService,
  EMIT_TASK,
  getAccessibleMenu,
  getAccessibleViewList,
  GlobalService,
  LANGUAGE,
  LogoutType,
  RoleType,
  RouterUrl,
  SUB_APP_REFRESH_FLAG,
  SupportLicense
} from 'app/shared';
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
  isEmpty,
  set,
  toString
} from 'lodash';
import { Observable, Observer } from 'rxjs';
import { finalize, map, switchMap } from 'rxjs/operators';
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
export class AppComponent implements OnInit, OnDestroy {
  isZh: boolean;
  isLogin = true;
  isErrorPage = true;
  isReportDetail = false;
  isMultiCluster = true;
  menus = [];
  optMenus = [];
  jobsData = [];
  taskData = [];
  alarmData = [];
  roleType = RoleType;
  currentCluster = {};
  runningTaskCount = 0;
  runningTotal = 0;
  criticalAlarmCount = 0;
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
  SESSION_START_TIME = new Date().getTime();
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

  colorDark = false;

  // 开源
  isOpenVersion = includes(
    [DataMap.Deploy_Type.openOem.value, DataMap.Deploy_Type.openServer.value],
    this.i18n.get('deploy_type')
  );

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

  @ViewChild('taskPopover', { static: false }) taskPopover;
  @ViewChild('alarmPopover', { static: false }) alarmPopover;
  @ViewChild('aboutHeaderTpl', { static: false })
  aboutHeaderTpl: TemplateRef<any>;
  @ViewChild('aboutContentTpl', { static: false })
  aboutContentTpl: TemplateRef<any>;
  @ViewChild('aboutFooterTpl', { static: false })
  aboutFooterTpl: TemplateRef<any>;

  constructor(
    public i18n: I18NService,
    public router: Router,
    private route: ActivatedRoute,
    private messageBox: MessageboxService,
    private messageService: MessageService,
    private appUtilsService: AppUtilsService,
    private viewContainerRef: ViewContainerRef,
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
    public adfsService: ADFSService
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
    // 开源处理
    if (this.isOpenVersion) {
      this.versionLabel = `${this.i18n.get(
        'common_version_label',
        [],
        true
      )}${this.i18n.get('common_open_version_label')}`;
    }
  }

  showGuidePop() {
    if (
      this.needGuideProduct &&
      localStorage.getItem(this.userName) !== 'true' &&
      !this.hasGuideTipShow
    ) {
      setTimeout(() => {
        this.guideTipShow = true;
        this.hasGuideTipShow = true;
      }, 500);
    }
  }

  hasKnowGuide() {
    localStorage.setItem(this.userName, 'true');
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
                    RouterUrl.ProtectionVirtualizationFusionOne
                  ]
                },
                {
                  id: 'container',
                  label: this.i18n.get('common_container_label'),
                  routerLink: RouterUrl.ProtectionContainer,
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
                  childrenLink: [
                    RouterUrl.ProtectionActiveDirectory,
                    RouterUrl.ProtectionHostAppExchange,
                    RouterUrl.ProtectionHostAppSapHana
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
                    RouterUrl.ExploreCopyDataFusionOne
                  ]
                },
                {
                  id: 'copy-container',
                  label: this.i18n.get('common_container_label'),
                  routerLink: '/explore/copy-data/container',
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
                  childrenLink: [
                    RouterUrl.ExploreCopyDataActiveDirectory,
                    RouterUrl.ExploreCopyDataDatabaseExchange,
                    RouterUrl.ExploreCopyDataSapHana
                  ]
                },
                {
                  id: 'copy-file-service',
                  label: this.i18n.get('common_file_systems_label'),
                  routerLink: '/explore/copy-data/file-service',
                  childrenLink: [
                    RouterUrl.ExploreCopyDataFileSystem,
                    RouterUrl.ExploreCopyDataNasShared,
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
              id: 'ransomware-protection',
              label: this.i18n.get('explore_ransomware_protection_label'),
              routerLink: RouterUrl.ExploreAntiRansomwareProtectionDataBackup,
              type: 'group',
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
                  routerLink:
                    RouterUrl.ExploreAntiRansomwareProtectionDataBackup
                },
                {
                  id: 'detection-model',
                  label: this.i18n.get('explore_detection_models_new_label'),
                  routerLink: RouterUrl.ExploreAntiRansomwareProtectionModel
                }
              ]
            },
            {
              id: 'data-desensitization',
              label: this.i18n.get('common_data_desensitization_label'),
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
              items: [
                {
                  id: 'airgap',
                  label: this.i18n.get('Air Gap'),
                  hidden: this.isCyberEngine,
                  routerLink: RouterUrl.ExplorePolicyAirgap
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
          label: this.i18n.get('explore_desensitization_report_label'),
          routerLink: RouterUrl.ExploreDetectionReport
        },
        {
          id: 'jobs',
          icon: 'aui-menu-job',
          label: this.i18n.get('common_jobs_label'),
          routerLink: '/insight/jobs'
        },
        {
          id: 'alarms',
          icon: 'aui-menu-alarm',
          label: this.i18n.get('common_alarms_events_label'),
          routerLink: '/insight/alarms'
        },
        {
          id: 'reports',
          icon: 'aui-menu-report',
          label: this.i18n.get('common_report_label'),
          routerLink: '/insight/reports',
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
          hidden:
            this.i18n.get('deploy_type') ===
            DataMap.Deploy_Type.hyperdetect.value
        },
        {
          id: 'storage-device',
          icon: 'aui-menu-storage-device',
          hidden: !this.isCyberEngine,
          label: this.i18n.get('protection_storage_device_label'),
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
                  routerLink: '/system/settings/tag-management',
                  childrenLink: RouterUrl.SystemTagManagement
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
                    [DataMap.Deploy_Type.cyberengine.value],
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
                        DataMap.Deploy_Type.x9000.value
                      ],
                      this.i18n.get('deploy_type')
                    )
                  ),
                  routerLink: '/system/settings/config-network'
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

    this.optMenus = [
      {
        id: 'modifyPwd',
        label: this.i18n.get('common_update_password_label'),
        hidden: includes(
          [
            DataMap.loginUserType.saml.value,
            DataMap.loginUserType.ldap.value,
            DataMap.loginUserType.ldapGroup.value,
            DataMap.loginUserType.hcs.value,
            DataMap.loginUserType.adfs.value
          ],
          this.userType
        ),
        onClick: () => this.modifyPwd()
      },
      {
        id: 'logout',
        label: this.i18n.get('common_logout_label'),
        onClick: () => this.logout()
      }
    ];
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
        this.colorDark = !includes(
          [RouterUrl.Home, RouterUrl.Login],
          this.router.url
        );
        // 首页加载字体是浅色，通过动态添加类实现
        if (includes([RouterUrl.Home], this.router.url)) {
          document.body.classList?.add('light-loading');
        } else {
          document.body.classList?.remove('light-loading');
        }
        this.drawModalService.destroyAllModals();
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
        .subscribe(
          res => {
            this.userName = res.userName;
            this.userType = res.userType;
            this.isHcsUser = res.userType === CommonConsts.HCS_USER_TYPE;
            this.isDmeUser = res.userType === CommonConsts.DME_USER_TYPE;
            observer.next(res);
            observer.complete();
          },
          () => {
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
        );
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
    const startTime = this.SESSION_START_TIME;
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
      this.SESSION_START_TIME = new Date().getTime();
    });
    this.SESSION_START_TIME = new Date().getTime();
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
      .subscribe(
        res => {
          this.calculatTimeout(res.sessionTime);
          this.seesionTimeOut = res.sessionTime;
          this.sessionTimeout = setTimeout(() => {
            clearTimeout(this.sessionTimeout);
            this.getSessionTime();
          }, this.SESSION_TIMER);
        },
        () => {
          this.calculatTimeout(this.seesionTimeOut);
          this.sessionTimeout = setTimeout(() => {
            clearTimeout(this.sessionTimeout);
            this.getSessionTime();
          }, this.SESSION_TIMER);
        }
      );
  }

  toLogin() {
    clearTimeout(this.jobTimeout);
    clearTimeout(this.recentjobTimeout);
    clearTimeout(this.sessionTimeout);
    clearTimeout(this.criticalAlarmTimeout);
    this.cookieService.removeAll(this.i18n.languageKey);
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
          this.adfsService.adfsLoginForward({}).subscribe(
            (resData: any) => {
              window.open(
                resData?.logoutForwardUrl,
                '_blank',
                'scrollbars=yes,resizable=yes,statebar=no,width=400,height=400,left=200, top=100'
              );
              this.rediectLoginPage();
            },
            () => {
              this.rediectLoginPage();
            }
          );
        } else {
          this.rediectLoginPage();
        }
      });
  }

  rediectLoginPage() {
    clearTimeout(this.jobTimeout);
    clearTimeout(this.sessionTimeout);
    this.cookieService.removeAll(this.i18n.languageKey);
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
        lvOkDisabled: true,
        lvWidth: 500,
        lvHeight: 330,
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
      return 'hyperdetect';
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
}
