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
import { DataMap, DataMapService, I18NService } from 'app/shared';
import { TableCols } from 'app/shared/components/pro-table';

@Component({
  selector: 'aui-openGauss-instance',
  templateUrl: './instance.component.html',
  styleUrls: ['./instance.component.less']
})
export class InstanceComponent implements OnInit {
  resourceSubType = DataMap.Resource_Type.OpenGauss_instance.value;
  columns: TableCols[] = [];
  constructor(
    private i18n: I18NService,
    private dataMapService: DataMapService
  ) {}

  ngOnInit(): void {
    this.columns = [
      {
        key: 'uuid',
        name: this.i18n.get('protection_resource_id_label'),
        hidden: true,
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      },
      {
        key: 'name',
        name: this.i18n.get('common_name_label'),
        sort: true,
        cellRender: {
          type: 'text',
          config: {
            id: 'outerClosable',
            iconPos: 'flow-text'
          }
        }
      },
      {
        key: 'instanceStatus',
        name: this.i18n.get('common_status_label'),
        filter: {
          type: 'select',
          isMultiple: true,
          options: this.dataMapService.toArray('openGauss_InstanceStatus')
        },
        cellRender: {
          type: 'status',
          config: this.dataMapService.toArray('openGauss_InstanceStatus')
        }
      }
    ];
  }
}
