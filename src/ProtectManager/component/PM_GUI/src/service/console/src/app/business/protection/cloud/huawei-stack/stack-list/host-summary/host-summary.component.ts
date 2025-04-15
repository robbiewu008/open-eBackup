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
  AppService,
  CommonConsts,
  DataMap,
  DataMapService,
  I18NService,
  ProtectedResourceApiService,
  ResourceDetailType
} from 'app/shared';
import { TableCols, TableConfig } from 'app/shared/components/pro-table';
import { AppUtilsService } from 'app/shared/services/app-utils.service';
import {
  assign,
  each,
  filter,
  first,
  includes,
  isEmpty,
  isNumber,
  size
} from 'lodash';

@Component({
  selector: 'aui-hcs-host-summary',
  templateUrl: './host-summary.component.html',
  styleUrls: ['./host-summary.component.less']
})
export class HCSHostSummaryComponent implements OnInit {
  tableConfig: TableConfig;
  tableData = {
    data: [],
    total: 0
  };
  source;
  isAPS = false;
  isHCSCloud = false;

  @ViewChild('sizeTpl', { static: true }) sizeTpl: TemplateRef<any>;
  constructor(
    private i18n: I18NService,
    private appService: AppService,
    private dataMapService: DataMapService,
    private appUtilsService: AppUtilsService,
    private protectedResourceApiService: ProtectedResourceApiService
  ) {}

  ngOnInit() {
    this.isHCSCloud =
      this.source.subType === DataMap.Resource_Type.HCSCloudHost.value;
    this.isAPS =
      this.source.subType === DataMap.Resource_Type.APSCloudServer.value;
    this.initConfig();
    this.showTableData();
    if (this.isAPS) {
      this.getResourceDetail();
    }
  }

  initConfig() {
    const cols: TableCols[] = [
      {
        key: 'name',
        name: this.i18n.get('common_name_label'),
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      },
      {
        key: 'storageType',
        name: this.i18n.get('protection_hcs_storage_type_label')
      },
      {
        key: 'mode',
        name: this.i18n.get('common_type_label'),
        filter: {
          type: 'select',
          isMultiple: true,
          showCheckAll: true,
          options: this.dataMapService.toArray('Disk_Mode')
        },
        cellRender: {
          type: 'status',
          config: this.dataMapService.toArray('Disk_Mode')
        }
      },
      {
        key: 'attr',
        name: this.i18n.get('protection_incremental_mode_label'),
        filter: {
          type: 'select',
          isMultiple: true,
          showCheckAll: true,
          options: this.dataMapService.toArray('Disk_Status')
        },
        cellRender: {
          type: 'status',
          config: this.dataMapService.toArray('Disk_Status')
        }
      },
      {
        key: 'kinds',
        name: this.i18n.get('protection_kind_label'),
        hidden: !this.isAPS,
        filter: {
          type: 'select',
          isMultiple: true,
          showCheckAll: true,
          options: this.dataMapService.toArray('aliDiskType')
        },
        cellRender: {
          type: 'status',
          config: this.dataMapService.toArray('aliDiskType')
        }
      },
      {
        key: 'sla',
        name: this.i18n.get('protection_associate_sla_label'),
        filter: {
          type: 'select',
          isMultiple: true,
          showCheckAll: true,
          options: this.dataMapService.toArray('slaAssociateStatus')
        },
        cellRender: {
          type: 'status',
          config: this.dataMapService.toArray('slaAssociateStatus')
        }
      },
      {
        key: 'encrypted',
        name: this.i18n.get('protection_hcs_encryption_label'),
        hidden: !this.isHCSCloud,
        cellRender: {
          type: 'status',
          config: this.dataMapService.toArray('passwordType')
        }
      },
      {
        key: 'size',
        name: this.i18n.get('common_capacity_label'),
        sort: true,
        cellRender: this.sizeTpl,
        thAlign: 'right'
      }
    ];
    let colsTmp = [];
    if (this.isAPS) {
      colsTmp = filter(cols, item => {
        return !includes(['storageType', 'attr'], item.key);
      });
    }

    this.tableConfig = {
      table: {
        async: false,
        size: 'small',
        columns: this.isAPS ? colsTmp : cols,
        compareWith: 'uuid',
        autoPolling: CommonConsts.TIME_INTERVAL_RESOURCE,
        colDisplayControl: false,
        trackByFn: (index, item) => {
          return item.uuid;
        }
      },
      pagination: {
        mode: 'simple',
        showPageSizeOptions: false,
        winTablePagination: true
      }
    };
  }

  getResourceDetail() {
    this.appUtilsService
      .getResourcesDetails(
        this.source,
        ResourceDetailType.apsDisk,
        {},
        { regionId: this.source.extendInfo.regionId }
      )
      .subscribe(recordsTemp => {
        const ext_parameters = this.source?.protectedObject?.extParameters;
        each(recordsTemp, item => {
          assign(item, {
            size: item.extendInfo?.size,
            mode: item.extendInfo?.type === 'data' ? 'false' : 'true',
            kinds: item.extendInfo?.category,
            sla:
              ext_parameters &&
              (ext_parameters?.all_disk === 'True' ||
                isEmpty(ext_parameters?.disk_info) ||
                (!!ext_parameters?.disk_info &&
                  ext_parameters.disk_info.includes(item.uuid))),
            name: `${item.name}(${item.uuid})`
          });
        });
        this.tableData = {
          data: recordsTemp,
          total: size(recordsTemp)
        };
      });
  }

  initDetailData(data: any) {
    this.source = data;
    const item = JSON.parse(this.source.extendInfo?.host || '{}');
    assign(this.source, {
      link_status: item.status
    });
  }

  showTableData() {
    let selectAll = false;
    let selectDisk = [];
    if (!isEmpty(this.source?.protectedObject)) {
      const diskInfo = this.source.protectedObject?.extParameters?.disk_info;
      if (isEmpty(diskInfo)) {
        selectAll = true;
      } else {
        selectDisk = diskInfo;
      }
    }
    const showData = JSON.parse(this.source.extendInfo?.host || '{}');
    if (showData?.diskInfo?.length) {
      each(showData.diskInfo, item => {
        assign(item, {
          sla: selectAll ? true : selectDisk.includes(item.id),
          encrypted: item.systemEncrypted === '1'
        });
      });
      this.tableData = {
        data: showData.diskInfo,
        total: size(showData.diskInfo)
      };
    }
  }
}
