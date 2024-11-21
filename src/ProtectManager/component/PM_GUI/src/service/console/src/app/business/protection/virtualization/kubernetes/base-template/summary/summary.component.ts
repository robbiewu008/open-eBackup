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
  CommonConsts,
  DataMap,
  DataMapService,
  I18NService,
  MODAL_COMMON
} from 'app/shared';
import {
  TableCols,
  TableConfig,
  TableData
} from 'app/shared/components/pro-table';
import { DrawModalService } from 'app/shared/services/draw-modal.service';
import { assign, each, get, includes, isEmpty, map, size } from 'lodash';

@Component({
  selector: 'aui-statefulset-summary',
  templateUrl: './summary.component.html',
  styleUrls: ['./summary.component.less']
})
export class SummaryComponent implements OnInit {
  source: any;
  tableData: TableData;
  tableConfig: TableConfig;
  sourceType: string;
  dataMap = DataMap;

  @ViewChild('slaTpl', { static: true }) slaTpl: TemplateRef<any>;

  constructor(
    private i18n: I18NService,
    private dataMapService: DataMapService,
    private drawModalService: DrawModalService
  ) {}

  ngOnInit() {
    this.initConfig();
  }

  initConfig() {
    const cols: TableCols[] = [
      {
        key: 'name',
        name: this.i18n.get('common_name_label'),
        filter: {
          type: 'search'
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
        key: 'size',
        sort: true,
        name: this.i18n.get('common_capacity_label')
      }
    ];
    this.tableConfig = {
      table: {
        columns: cols,
        async: false,
        compareWith: 'name',
        colDisplayControl: false
      },
      pagination: {
        pageSize: CommonConsts.PAGE_SIZE_SMALL,
        winTablePagination: true,
        mode: 'simple',
        showPageSizeOptions: false,
        pageSizeOptions: CommonConsts.SIMPLE_PAGE_SIZE_OPTIONS
      }
    };
  }

  initDetailData(data: any) {
    this.sourceType = data.sub_type;
    this.source = data;
    if (
      this.sourceType === DataMap.Resource_Type.kubernetesDatasetCommon.value
    ) {
      return;
    }
    const protectedVolumes = !isEmpty(data.protectedObject)
      ? data.protectedObject.extParameters?.volume_names
      : [];
    const dataw = JSON.parse(get(data, ['extendInfo', 'sts']))?.pods[0].pvs;
    const volumes = map(dataw, item => {
      return {
        name: item.volumeName,
        sla:
          !isEmpty(protectedVolumes) &&
          includes(protectedVolumes, item.volumeName)
            ? DataMap.slaAssociateStatus.associate.value
            : DataMap.slaAssociateStatus.noAssociate.value,
        size: item.size?.replace('Gi', 'GB')
      };
    });
    this.tableData = {
      data: volumes,
      total: size(volumes)
    };
  }
}
