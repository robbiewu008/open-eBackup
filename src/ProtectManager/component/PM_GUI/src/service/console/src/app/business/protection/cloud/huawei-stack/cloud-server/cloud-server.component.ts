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
import { Component, OnInit, ViewChild } from '@angular/core';
import { BaseTemplateComponent } from 'app/business/protection/virtualization/kubernetes/base-template/base-template.component';
import {
  CookieService,
  DataMap,
  DataMapService,
  I18NService,
  OpHcsServiceApiService,
  OperateItems
} from 'app/shared';
import { TableCols } from 'app/shared/components/pro-table';
import { get } from 'lodash';

@Component({
  selector: 'aui-hcs-cloud-server',
  templateUrl: './cloud-server.component.html',
  styleUrls: ['./cloud-server.component.less']
})
export class CloudServerComponent implements OnInit {
  subType = DataMap.Resource_Type.HCSCloudHost.value;
  columns: TableCols[] = [
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
      filter: {
        type: 'search',
        filterMode: 'contains'
      },
      cellRender: {
        type: 'text',
        config: {
          id: 'outerClosable',
          iconPos: 'flow-text',
          click: data => {}
        }
      }
    },
    {
      key: 'status',
      name: this.i18n.get('common_status_label'),
      filter: {
        type: 'select',
        isMultiple: true,
        showCheckAll: true,
        options: this.dataMapService
          .toArray('HCS_Host_LinkStatus')
          .filter(item => {
            return [
              DataMap.HCS_Host_LinkStatus.normal.value,
              DataMap.HCS_Host_LinkStatus.shutoff.value,
              DataMap.HCS_Host_LinkStatus.error.value
            ].includes(item.value);
          })
      },
      cellRender: {
        type: 'status',
        config: this.dataMapService.toArray('HCS_Host_LinkStatus')
      }
    },
    {
      key: 'vm_ip',
      name: this.i18n.get('common_ip_address_label'),
      filter: {
        type: 'search',
        filterMode: 'contains'
      }
    },
    {
      key: 'isWorkspace',
      name: this.i18n.get('protection_hcs_workspace_label'),
      filter: {
        type: 'select',
        isMultiple: true,
        showCheckAll: true,
        options: this.dataMapService.toArray('isWorkspace')
      },
      cellRender: {
        type: 'status',
        config: this.dataMapService.toArray('isWorkspace')
      }
    },
    {
      key: 'computerName',
      name: this.i18n.get('protection_hcs_workspace_name_label'),
      filter: {
        type: 'search',
        filterMode: 'contains'
      }
    },
    {
      key: 'path',
      name: this.i18n.get('common_location_label'),
      filter: {
        type: 'search',
        filterMode: 'contains'
      },
      hidden: true
    }
  ];
  extraConfig = {
    scan: {
      id: 'scan',
      type: 'primary',
      permission: OperateItems.ScanHCSProject,
      label: this.i18n.get('common_rescan_label'),
      onClick: () => this.scanResource()
    }
  };
  @ViewChild(BaseTemplateComponent, { static: false })
  BaseTemplateComponent: BaseTemplateComponent;

  constructor(
    private i18n: I18NService,
    private cookieService: CookieService,
    private dataMapService: DataMapService,
    private opHcsServiceApiService: OpHcsServiceApiService
  ) {}

  ngOnInit() {}

  scanResource() {
    this.opHcsServiceApiService
      .scanHcsActionResources({
        projectId:
          get(window, 'parent.hcsData.projectId', '') ||
          this.cookieService.get('projectId'),
        projectName: decodeURI(
          get(window, 'parent.hcsData.ProjectName', '') ||
            this.cookieService.get('projectName')
        )
      })
      .subscribe(() => {
        if (this.BaseTemplateComponent) {
          this.BaseTemplateComponent.dataTable?.fetchData();
        }
      });
  }
}
