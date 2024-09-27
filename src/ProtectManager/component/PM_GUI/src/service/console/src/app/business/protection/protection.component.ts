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
import {
  ApplicationType,
  CatalogName,
  CommonConsts,
  CookieService,
  DataMap,
  getAccessibleMenu,
  GlobalService,
  RouterUrl,
  SupportLicense
} from 'app/shared';
import { I18NService } from 'app/shared/services/i18n.service';
import { ResourceCatalogsService } from 'app/shared/services/resource-catalogs.service';
import { filter, includes } from 'lodash';
import { combineLatest } from 'rxjs';

@Component({
  selector: 'aui-protection',
  templateUrl: './protection.component.html'
})
export class ProtectionComponent implements OnInit {
  menus = [];
  clientMenus = [];
  policyMenus = [];
  resCatalogs = [];
  collapsed = false;
  isHcsUser = false;
  activeId;

  isOceanProtect = !includes(
    [
      DataMap.Deploy_Type.cloudbackup.value,
      DataMap.Deploy_Type.cloudbackup2.value,
      DataMap.Deploy_Type.hyperdetect.value,
      DataMap.Deploy_Type.cyberengine.value
    ],
    this.i18n.get('deploy_type')
  );

  constructor(
    public i18n: I18NService,
    public cookieService: CookieService,
    public globalService: GlobalService,
    private resourceCatalogsService: ResourceCatalogsService
  ) {}

  ngOnInit() {
    combineLatest([
      this.globalService.getUserInfo(),
      this.resourceCatalogsService.getResourceCatalog()
    ]).subscribe(res => {
      this.resCatalogs = res[1] || [];
      this.initMenus();
      this.isHcsUser =
        this.cookieService.get('userType') === CommonConsts.HCS_USER_TYPE;
    });
  }

  initMenus() {
    const menus = [
      {
        id: 'summary',
        label: this.i18n.get('common_summary_label'),
        icon: 'aui-icon-summary',
        routerLink: RouterUrl.ProtectionSummary
      },
      {
        id: 'database',
        label: this.i18n.get('common_database_label'),
        hidden: !includes(this.resCatalogs, CatalogName.HostApps),
        icon: 'aui-icon-host-app',
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
        hidden: !includes(this.resCatalogs, CatalogName.BigData),
        icon: 'aui-icon-big-data',
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
        hidden: !includes(this.resCatalogs, CatalogName.Virtualization),
        icon: 'aui-icon-virtualization',
        routerLink: RouterUrl.ProtectionVirtualization,
        childrenLink: [
          RouterUrl.ProtectionVirtualizationVmware,
          RouterUrl.ProtectionVirtualizationCnware,
          RouterUrl.ProtectionVirtualizationFusionCompute,
          RouterUrl.ProtectionVirtualizationHyperV
        ]
      },
      {
        id: 'container',
        label: this.i18n.get('common_container_label'),
        hidden: !includes(this.resCatalogs, CatalogName.Virtualization),
        icon: 'aui-icon-container-app',
        routerLink: RouterUrl.ProtectionContainer,
        childrenLink: [
          RouterUrl.ProtectionVirtualizationKubernetes,
          RouterUrl.ProtectionVirtualizationKubernetesContainer
        ]
      },
      {
        id: 'cloud',
        label: this.i18n.get('common_huawei_clouds_label'),
        hidden: !includes(this.resCatalogs, CatalogName.Cloud),
        icon: 'aui-icon-cloud',
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
        hidden: !includes(this.resCatalogs, CatalogName.Application),
        icon: 'aui-icon-application',
        routerLink: [RouterUrl.ProtectionApplication],
        childrenLink: [
          RouterUrl.ProtectionActiveDirectory,
          RouterUrl.ProtectionHostAppExchange,
          RouterUrl.ProtectionHostAppSapHana
        ]
      },
      {
        id: 'file-service',
        label: this.i18n.get('common_file_systems_label'),
        hidden: !includes(this.resCatalogs, CatalogName.HostApps),
        icon: 'aui-icon-file-service-app',
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
      },
      {
        id: 'client',
        label: this.i18n.get('protection_clients_label'),
        hidden: !includes(this.resCatalogs, CatalogName.HostApps),
        icon: 'aui-icon-client',
        routerLink: RouterUrl.ProtectionHostAppHost,
        childrenLink: [
          RouterUrl.ProtectionHostAppHost,
          RouterUrl.ProtectionHostAppHostRegister
        ]
      },
      {
        id: 'cloudStorage',
        hidden:
          !includes(this.resCatalogs, CatalogName.Storage) ||
          !includes(
            [
              DataMap.Deploy_Type.cloudbackup.value,
              DataMap.Deploy_Type.cloudbackup2.value
            ],
            this.i18n.get('deploy_type')
          ),
        label: this.i18n.get('common_storage_label'),
        icon: 'aui-icon-storages',
        items: [
          {
            id: 'local-file-system',
            label: this.i18n.get('common_local_file_system_label'),
            hidden: !includes(
              this.resCatalogs,
              ApplicationType.LocalFileSystem
            ),
            routerLink: '/protection/storage/local-file-system'
          }
        ]
      },
      {
        id: 'hyperStorage',
        hidden:
          !includes(this.resCatalogs, CatalogName.Storage) ||
          !includes(
            [DataMap.Deploy_Type.hyperdetect.value],
            this.i18n.get('deploy_type')
          ),
        label: this.i18n.get('common_storage_label'),
        icon: 'aui-icon-storages',
        items: [
          {
            id: 'local-resource',
            label: this.i18n.get('protection_local_resource_label'),
            hidden: !(SupportLicense.isFile || SupportLicense.isSan),
            routerLink: '/protection/storage/local-resource'
          }
        ]
      },
      {
        id: 'protection-policy',
        label: this.i18n.get('protection_policy_label'),
        icon: 'aui-icon-sla',
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
    ];
    if (this.isOceanProtect) {
      this.menus = getAccessibleMenu(
        filter(
          menus,
          item => !includes(['client', 'protection-policy'], item.id)
        ),
        this.cookieService,
        this.i18n
      );
    } else {
      this.menus = getAccessibleMenu(menus, this.cookieService, this.i18n);
    }

    this.clientMenus = getAccessibleMenu(
      [
        {
          id: 'client',
          label: this.i18n.get('protection_clients_label'),
          hidden: !includes(this.resCatalogs, CatalogName.HostApps),
          icon: 'aui-icon-client',
          routerLink: RouterUrl.ProtectionHostAppHost,
          childrenLink: [
            RouterUrl.ProtectionHostAppHost,
            RouterUrl.ProtectionHostAppHostRegister
          ]
        }
      ],
      this.cookieService,
      this.i18n
    );

    this.policyMenus = [
      {
        id: 'sla',
        icon: 'aui-icon-sla',
        label: this.i18n.get('common_sla_label'),
        routerLink: RouterUrl.ProtectionSla
      },
      {
        id: 'limit-rate-policy',
        icon: 'aui-icon-limit-rate',
        label: this.i18n.get('common_limit_rate_policy_label'),
        routerLink: RouterUrl.ProtectionLimitRatePolicy
      }
    ];
  }

  activeMenuChange(id) {
    setTimeout(() => {
      this.activeId = id;
    });
  }
}
