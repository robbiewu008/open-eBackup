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
import { Component, OnInit, TemplateRef, ViewChild } from '@angular/core';
import {
  ApiStorageBackupPluginService,
  CAPACITY_UNIT,
  DataMap,
  DataMapService,
  I18NService,
  CookieService
} from 'app/shared';
import { TableCols, TableConfig } from 'app/shared/components/pro-table';
import { assign, each, map, omit, size } from 'lodash';

@Component({
  selector: 'aui-nas-shared-summary',
  templateUrl: './summary.component.html',
  styleUrls: ['./summary.component.less']
})
export class SummaryComponent implements OnInit {
  source;
  sourceType;
  dataMap = DataMap;
  unitconst = CAPACITY_UNIT;

  fileSystemConfig: TableConfig;
  nfsSharedConfig: TableConfig;
  cifsSharedConfig: TableConfig;
  fileSystemData = {};
  nfsSharedData = {};
  cifsSharedData = {};
  isCyberEngine =
    this.i18n.get('deploy_type') === DataMap.Deploy_Type.cyberengine.value;

  @ViewChild('capacityTpl', { static: true }) capacityTpl: TemplateRef<any>;
  @ViewChild('usedCapacityTpl', { static: true }) usedCapacityTpl: TemplateRef<
    any
  >;

  constructor(
    private i18n: I18NService,
    public cookieService: CookieService,
    private dataMapService: DataMapService,
    private apiStorageBackupPluginService: ApiStorageBackupPluginService
  ) {}

  initDetailData(data: any) {
    this.source = data;
    this.sourceType = data.resourceType;
  }

  ngOnInit() {
    if (
      [
        DataMap.Resource_Type.NASFileSystem.value,
        DataMap.Resource_Type.LocalFileSystem.value
      ].includes(this.sourceType) &&
      !this.isCyberEngine
    ) {
      this.initTable();
      this.getFileSystemInfo();
    }
  }

  initTable() {
    const authCol: TableCols = {
      key: 'authorityLevel',
      name: this.i18n.get('explore_authority_level_label'),
      filter: {
        type: 'select',
        isMultiple: true,
        showCheckAll: true,
        options: this.dataMapService.toArray('Filesystem_Authority_Level')
      },
      cellRender: {
        type: 'status',
        config: this.dataMapService.toArray('Filesystem_Authority_Level')
      }
    };
    const fileSysCols: TableCols[] = [
      {
        key: 'equipmentName',
        name: this.i18n.get('protection_storage_device_label')
      },
      {
        key: 'tenant',
        name: this.i18n.get('common_tenant_label'),
        cellRender: {
          type: 'status',
          config: this.dataMapService.toArray('Nas_Tenant_Type')
        }
      },
      {
        key: 'capacity',
        name: this.i18n.get('common_total_capacity_label'),
        cellRender: this.capacityTpl
      },
      {
        key: 'usedCapacity',
        name: this.i18n.get('common_used_capcity_label'),
        cellRender: this.usedCapacityTpl
      }
    ];
    const nfsCols: TableCols[] = [
      {
        key: 'sharePath',
        name: this.i18n.get('protection_share_path_info_label')
      },
      {
        key: 'client',
        name: this.i18n.get('protection_share_client_label'),
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      },
      {
        key: 'clientType',
        name: this.i18n.get('common_type_label'),
        filter: {
          type: 'select',
          isMultiple: true,
          showCheckAll: true,
          options: this.dataMapService.toArray('Nfs_Share_Client_Type')
        },
        cellRender: {
          type: 'status',
          config: this.dataMapService.toArray('Nfs_Share_Client_Type')
        }
      },
      authCol
    ];
    const cifsCols: TableCols[] = [
      {
        key: 'shareName',
        name: this.i18n.get('explore_share_name_label'),
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      },
      {
        key: 'sharePath',
        name: this.i18n.get('protection_share_path_info_label')
      },
      {
        key: 'user',
        name: this.i18n.get('protection_user_group_label')
      },
      {
        key: 'domainType',
        name: this.i18n.get('common_type_label'),
        filter: {
          type: 'select',
          isMultiple: true,
          showCheckAll: true,
          options: this.dataMapService.toArray('Cifs_Domain_Client_Type')
        },
        cellRender: {
          type: 'status',
          config: this.dataMapService.toArray('Cifs_Domain_Client_Type')
        }
      },
      authCol
    ];
    this.fileSystemConfig = {
      table: {
        columns: fileSysCols,
        showLoading: false,
        colDisplayControl: false,
        size: 'small',
        async: false
      },
      pagination: {
        mode: 'simple',
        showPageSizeOptions: false,
        winTablePagination: true
      }
    };
    this.nfsSharedConfig = {
      table: {
        columns: nfsCols,
        showLoading: false,
        colDisplayControl: false,
        size: 'small',
        async: false
      },
      pagination: {
        mode: 'simple',
        showPageSizeOptions: false,
        winTablePagination: true
      }
    };
    this.cifsSharedConfig = {
      table: {
        columns: cifsCols,
        showLoading: false,
        colDisplayControl: false,
        size: 'small',
        async: false
      },
      pagination: {
        mode: 'simple',
        showPageSizeOptions: false,
        winTablePagination: true
      }
    };
  }

  getFileSystemInfo() {
    this.apiStorageBackupPluginService
      .ShowStorageFileSystemInfo({
        fileSystemId: this.source.uuid,
        akDoException: !(
          this.source.resource_status === DataMap.Resource_Status.notExist.value
        )
      })
      .subscribe(res => {
        this.fileSystemData = {
          data: [omit(res, ['nfsShares', 'cifsShares'])],
          total: 1
        };
        const nfsInfo = [];
        const cifsInfo = [];
        each(res.nfsShares, item => {
          each(item.nfsClients, v => {
            nfsInfo.push(assign({}, item, v));
          });
        });
        each(res.cifsShares, item => {
          each(item.cifsClients, v => {
            cifsInfo.push(assign({}, item, v));
          });
        });
        this.nfsSharedData = {
          data: nfsInfo,
          total: size(nfsInfo)
        };
        this.cifsSharedData = {
          data: cifsInfo,
          total: size(cifsInfo)
        };
      });
  }
}
