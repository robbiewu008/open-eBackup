import { Component, OnInit } from '@angular/core';
import {
  ApplicationType,
  CatalogName,
  CommonConsts,
  CookieService,
  DataMap,
  getAccessibleMenu,
  GlobalService,
  I18NService,
  RouterUrl,
  SupportLicense
} from 'app/shared';
import { AppUtilsService } from 'app/shared/services/app-utils.service';
import { ResourceCatalogsService } from 'app/shared/services/resource-catalogs.service';
import { includes, intersection, size } from 'lodash';
import { combineLatest } from 'rxjs';

@Component({
  selector: 'aui-explore',
  templateUrl: './explore.component.html',
  styleUrls: ['./explore.component.css']
})
export class ExploreComponent implements OnInit {
  menus = [];
  resCatalogs = [];
  collapsed = false;
  title = this.i18n.get('common_explore_label');
  isHcsUser = this.cookieService.get('userType') === CommonConsts.HCS_USER_TYPE;
  isHyperDetect =
    this.i18n.get('deploy_type') === DataMap.Deploy_Type.hyperdetect.value;

  constructor(
    public i18n: I18NService,
    public cookieService: CookieService,
    public globalService: GlobalService,
    private resourceCatalogsService: ResourceCatalogsService,
    public appUtilsService: AppUtilsService
  ) {}

  ngOnInit() {
    this.initTitle();
    combineLatest([
      this.globalService.getUserInfo(),
      this.resourceCatalogsService.getResourceCatalog()
    ]).subscribe(res => {
      this.resCatalogs = res[1] || [];
      this.initMenus();
    });
  }

  initTitle() {
    this.title =
      this.i18n.get('deploy_type') === DataMap.Deploy_Type.cyberengine.value
        ? this.i18n.get('protection_data_security_label')
        : this.i18n.get('common_explore_label');
  }

  initMenus() {
    const menus = [
      {
        id: 'storage-device',
        label: this.i18n.get('protection_storage_devices_label'),
        icon: 'aui-icon-storages',
        hidden:
          this.i18n.get('deploy_type') !==
          DataMap.Deploy_Type.cyberengine.value,
        routerLink: '/explore/storage-device'
      },
      {
        id: 'ransomware-protection',
        label: this.i18n.get('explore_ransomware_protection_label'),
        icon: 'aui-icon-anti-ransomware',
        items: [
          {
            id: 'file-interception',
            label: this.i18n.get('explore_file_block_label'),
            routerLink: '/explore/ransomware-protection/file-interception'
          },
          {
            id: 'real-time-detection',
            label: this.i18n.get('explore_real_time_detection_new_label'),
            routerLink: '/explore/ransomware-protection/real-time-detection'
          },
          {
            id: 'data-backup',
            label: this.i18n.get('explore_intelligent_detection_label'),
            routerLink: '/explore/ransomware-protection/data-backup'
          },
          {
            id: 'detection-model',
            label: this.i18n.get('explore_detection_models_new_label'),
            routerLink: '/explore/ransomware-protection/detection-model'
          }
        ]
      },
      {
        id: 'copy-data',
        label: this.i18n.get('common_copy_data_label'),
        icon: 'aui-icon-copy-data',
        hidden: includes(
          [
            DataMap.Deploy_Type.hyperdetect.value,
            DataMap.Deploy_Type.cloudbackup.value,
            DataMap.Deploy_Type.cloudbackup2.value
          ],
          this.i18n.get('deploy_type')
        ),
        items: [
          {
            id: 'databse',
            hidden: !includes(this.resCatalogs, CatalogName.HostApps),
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
            id: 'big-data',
            hidden: !includes(this.resCatalogs, CatalogName.BigData),
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
            id: 'virtualization',
            hidden: !includes(this.resCatalogs, CatalogName.Virtualization),
            label: this.i18n.get('common_virtualization_label'),
            routerLink: '/explore/copy-data/virtualization',
            childrenLink: [
              RouterUrl.ExploreCopyDataVMware,
              RouterUrl.ExploreCopyDataCNware,
              RouterUrl.ExploreCopyDataFusionCompute,
              RouterUrl.ExploreCopyDataHyperv
            ]
          },
          {
            id: 'container',
            hidden: !includes(this.resCatalogs, CatalogName.Virtualization),
            label: this.i18n.get('common_container_label'),
            routerLink: '/explore/copy-data/container',
            childrenLink: [
              RouterUrl.ExploreCopyDataKubernetes,
              RouterUrl.ExploreCopyDataKubernetesContainer
            ]
          },
          {
            id: 'cloud',
            hidden: !includes(this.resCatalogs, CatalogName.Virtualization),
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
            id: 'application',
            hidden: !includes(this.resCatalogs, CatalogName.Application),
            label: this.i18n.get('common_application_label'),
            routerLink: '/explore/copy-data/application',
            childrenLink: [
              RouterUrl.ExploreCopyDataActiveDirectory,
              RouterUrl.ExploreCopyDataDatabaseExchange,
              RouterUrl.ExploreCopyDataSapHana
            ]
          },
          {
            id: 'file-service',
            hidden: !includes(this.resCatalogs, CatalogName.Storage),
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
        id: 'copy-data',
        label: this.i18n.get('common_copy_data_label'),
        icon: 'aui-icon-copy-data',
        hidden: !includes(
          [
            DataMap.Deploy_Type.cloudbackup.value,
            DataMap.Deploy_Type.cloudbackup2.value
          ],
          this.i18n.get('deploy_type')
        ),
        items: [
          {
            id: 'copy-data-local-file-syste',
            hidden: !includes(
              this.resCatalogs,
              ApplicationType.LocalFileSystem
            ),
            label: this.i18n.get('common_local_file_system_label'),
            routerLink: '/explore/copy-data/local-file-system'
          }
        ]
      },
      {
        id: 'live-mounts',
        label: this.i18n.get('common_live_mount_label'),
        hidden: !size(
          intersection(this.resCatalogs, [
            ApplicationType.Vmware,
            ApplicationType.Oracle
          ])
        ),
        icon: 'aui-icon-live-mounts',
        items: [
          {
            id: 'live-mounts-mysql',
            hidden: !includes(this.resCatalogs, ApplicationType.MySQL),
            label: this.i18n.get('protection_mysql_label'),
            routerLink: '/explore/live-mounts/mysql'
          },
          {
            id: 'live-mounts-oracle',
            hidden: !includes(this.resCatalogs, ApplicationType.Oracle),
            label: this.i18n.get('common_oracle_label'),
            routerLink: '/explore/live-mounts/oracle'
          },
          {
            id: 'live-mounts-tdsql',
            hidden: !includes(this.resCatalogs, ApplicationType.TDSQL),
            label: this.i18n.get('TDSQL'),
            routerLink: '/explore/live-mounts/tdsql'
          },
          {
            id: 'live-mounts-cnware',
            hidden: !includes(this.resCatalogs, ApplicationType.CNware),
            label: this.i18n.get('common_cnware_label'),
            routerLink: '/explore/live-mounts/cnware'
          },
          {
            id: 'live-mounts-vmware',
            hidden: !includes(this.resCatalogs, ApplicationType.Vmware),
            label: this.i18n.get('common_vmware_label'),
            routerLink: '/explore/live-mounts/vmware'
          },
          {
            id: 'live-mounts-nas-shared',
            hidden:
              this.appUtilsService.isDistributed ||
              !includes(this.resCatalogs, ApplicationType.NASShare),
            label: this.i18n.get('common_nas_shares_label'),
            routerLink: '/explore/live-mounts/nas-shared'
          },
          {
            id: 'live-mounts-dorado-file-system',
            hidden:
              this.appUtilsService.isDistributed ||
              this.appUtilsService.isDecouple ||
              !includes(this.resCatalogs, ApplicationType.NASFileSystem),
            label: this.i18n.get('common_nas_file_systems_label'),
            routerLink: '/explore/live-mounts/dorado-file-system'
          },
          {
            id: 'live-mounts-volume',
            hidden: !includes(this.resCatalogs, ApplicationType.Volume),
            label: this.i18n.get('protection_volume_label'),
            routerLink: '/explore/live-mounts/volume'
          },
          {
            id: 'live-mounts-fileset',
            hidden:
              this.appUtilsService.isDistributed ||
              !includes(this.resCatalogs, ApplicationType.Fileset),
            label: this.i18n.get('common_fileset_label'),
            routerLink: '/explore/live-mounts/fileset'
          }
        ]
      },
      {
        id: 'recovery-drill',
        label: this.i18n.get('explore_recovery_drill_label'),
        icon: 'aui-icon-recovery-drill',
        routerLink: RouterUrl.ExploreRecoveryDrill,
        childrenLink: [
          RouterUrl.ExploreRecoveryDrill,
          RouterUrl.ExploreCreateDrill,
          RouterUrl.ExploreModifyDrill,
          RouterUrl.ExploreDrillDetail,
          RouterUrl.ExploreDrillExecuteLog
        ]
      },
      {
        id: 'data-desensitization',
        label: this.i18n.get('common_data_desensitization_label'),
        hidden:
          this.i18n.get('deploy_type') === DataMap.Deploy_Type.x3000.value,
        icon: 'aui-icon-data-desensitization',
        items: [
          {
            id: 'data-desensitization-oracle',
            hidden: !includes(this.resCatalogs, ApplicationType.Oracle),
            label: this.i18n.get('common_oracle_label'),
            routerLink: '/explore/data-desensitization/oracle'
          }
        ]
      },
      {
        id: 'anti-ransomware',
        label: this.i18n.get('common_anti_ransomware_label'),
        icon: 'aui-icon-anti-ransomware',
        items: [
          {
            id: 'anti-ransomware-overview',
            label: this.i18n.get('explore_detection_overview_labe'),
            routerLink: '/explore/anti-ransomware/overview'
          },
          {
            id: 'anti-ransomware-cloud-backup-overview',
            hidden:
              this.isHyperDetect &&
              !(SupportLicense.isFile || SupportLicense.isSan),
            label: this.i18n.get('common_summary_label'),
            routerLink: '/explore/anti-ransomware/cloud-backup-overview'
          },
          {
            id: 'anti-ransomware-detection-setting',
            hidden:
              this.isHyperDetect &&
              !(SupportLicense.isFile || SupportLicense.isSan),
            label: this.i18n.get('explore_ransomware_dectetion_setting_label'),
            routerLink: '/explore/anti-ransomware/detection-setting'
          },
          {
            id: 'anti-ransomware-blocking-rule-list',
            hidden: this.isHyperDetect && !SupportLicense.isFile,
            label: this.i18n.get('explore_file_blocking_label'),
            routerLink: '/explore/anti-ransomware/blocking-rule-list'
          },
          {
            id: 'anti-ransomware-real-time-detect',
            hidden: this.isHyperDetect && !SupportLicense.isFile,
            label: this.i18n.get('explore_real_time_detection_label'),
            routerLink: '/explore/anti-ransomware/real-time-detect'
          },
          {
            id: 'anti-ransomware-detection-model-list',
            hidden:
              this.isHyperDetect &&
              !(SupportLicense.isFile || SupportLicense.isSan),
            label: this.i18n.get('explore_detection_model_label'),
            routerLink: '/explore/anti-ransomware/detection-model-list'
          }
        ],
        hidden: [
          DataMap.Deploy_Type.cloudbackup.value,
          DataMap.Deploy_Type.cloudbackup2.value,
          DataMap.Deploy_Type.x3000.value
        ].includes(this.i18n.get('deploy_type'))
      },
      {
        id: 'anti-detection-result',
        label: this.i18n.get('explore_ransomware_detection_result_label'),
        routerLink: '/explore/local-file-system',
        icon: 'aui-icon-anti-detection-result',
        hidden: [
          DataMap.Deploy_Type.cloudbackup.value,
          DataMap.Deploy_Type.cloudbackup2.value,
          DataMap.Deploy_Type.x3000.value
        ].includes(this.i18n.get('deploy_type'))
      },
      {
        id: 'airgap',
        hidden: this.appUtilsService.isDistributed,
        label: this.i18n.get('Air Gap'),
        icon: 'aui-airgap-menu',
        routerLink: '/explore/airgap'
      },
      {
        id: 'snapshot-data',
        label: this.i18n.get('common_hyperdetect_copy_data_label'),
        icon: 'aui-icon-copy-data',
        routerLink: '/explore/snapshot-data'
      },
      {
        id: 'detection-report',
        label: this.i18n.get('explore_desensitization_reports_label'),
        icon: 'aui-icon-jobs',
        routerLink: '/explore/detection-report'
      },
      {
        id: 'explore-policy',
        label: this.i18n.get('explore_policy_label'),
        icon: 'aui-icon-policy',
        items: [
          {
            id: 'policy-mount-update-policy',
            label: this.i18n.get('common_mount_update_policy_label'),
            routerLink: '/explore/policy/mount-update-policy'
          },
          {
            id: 'policy-desensitization-policy',
            label: this.i18n.get('common_desensitization_policy_label'),
            hidden:
              this.i18n.get('deploy_type') === DataMap.Deploy_Type.x3000.value,
            routerLink: '/explore/policy/desensitization-policy'
          },
          {
            id: 'policy-anti_policy',
            label:
              this.i18n.get('deploy_type') === DataMap.Deploy_Type.x3000.value
                ? this.i18n.get('common_worm_policy_label')
                : this.i18n.get('common_anti_policy_label'),
            routerLink: '/explore/policy/anti-policy'
          }
        ]
      }
    ];
    this.menus = getAccessibleMenu(menus, this.cookieService, this.i18n);
  }
}
