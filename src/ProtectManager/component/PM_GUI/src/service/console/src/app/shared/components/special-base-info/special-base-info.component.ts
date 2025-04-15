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
import { DatePipe } from '@angular/common';
import { Component, Input, OnDestroy, OnInit } from '@angular/core';
import {
  HostService,
  ProjectedObjectApiService
} from 'app/shared/api/services';
import { CopiesService } from 'app/shared/api/services/copies.service';
import { DataMap } from 'app/shared/consts';
import {
  DataMapService,
  GlobalService,
  I18NService
} from 'app/shared/services';
import { assign, find, includes, remove } from 'lodash';
import { Subscription } from 'rxjs';

@Component({
  selector: 'aui-special-base-info',
  templateUrl: './special-base-info.component.html',
  styleUrls: ['./special-base-info.component.less'],
  providers: [DatePipe]
})
export class SpecialBaseInfoComponent implements OnInit, OnDestroy {
  resourceType = DataMap.Resource_Type;
  @Input() source;
  @Input() sourceType;
  formItems: any = [
    [
      {
        key: 'name',
        label: this.i18n.get('common_name_label'),
        content: ''
      },
      {
        key: 'type',
        label: this.i18n.get('common_type_label'),
        content: ''
      },
      {
        key: 'link_status',
        label: this.i18n.get('common_status_label'),
        content: ''
      }
    ],
    [
      {
        key: 'protect_activation',
        label: this.i18n.get('protection_protected_status_label'),
        content: ''
      },
      {
        key: 'sla_policy',
        label: this.i18n.get('common_sla_label'),
        content: ''
      },
      {
        key: 'sla_compliance',
        label: this.i18n.get('common_sla_compliance_label'),
        content: ''
      }
    ]
  ];
  subscription$: Subscription;

  constructor(
    public datePipe: DatePipe,
    public i18n: I18NService,
    public hostServiceAPI: HostService,
    public copiesApiService: CopiesService,
    public dataMapService: DataMapService,
    public projectedObjectApiService: ProjectedObjectApiService,
    private globalService: GlobalService
  ) {}

  ngOnDestroy() {
    this.subscription$.unsubscribe();
  }

  getResourceInfo() {
    switch (this.sourceType) {
      case DataMap.Resource_Type.ApsaraStack.value:
      case DataMap.Resource_Type.cNwareCluster.value:
      case DataMap.Resource_Type.nutanixCluster.value:
      case DataMap.Resource_Type.FusionComputeCluster.value:
      case DataMap.Resource_Type.FusionComputeCNA.value:
      case DataMap.Resource_Type.Project.value:
      case DataMap.Resource_Type.stackProject.value: {
        if (find(this.formItems[0], { key: 'link_status' })) {
          remove(this.formItems[0] as any, { key: 'link_status' });
        }
        this.formItems[0][0].content = this.source.name;
        this.formItems[0][1].content =
          this.source?.environment?.name || this.source?.parentName;
        this.formItems[0][1].label = this.i18n.get('common_vm_label');
        break;
      }
      case DataMap.Resource_Type.cNwareHost.value:
      case DataMap.Resource_Type.nutanixHost.value:
        this.formItems[0][0].content = this.source.name;
        this.formItems[0][1].content =
          this.source?.environment?.name || this.source?.parentName;
        this.formItems[0][1].label = this.i18n.get('common_vm_label');
        if (this.sourceType === DataMap.Resource_Type.nutanixHost.value) {
          this.formItems[0][2].content = this.source?.extendInfo?.status;
        } else {
          this.formItems[0][2].content = this.source?.status;
        }
        break;
      case DataMap.Resource_Type.hyperVHost.value:
        this.formItems[0][0].content = this.source.name;
        this.formItems[0][1].content = this.source?.path;
        this.formItems[0][1].label = this.i18n.get('common_path_label');
        this.formItems[0][2].content =
          this.source?.linkStatus ??
          includes(
            [
              DataMap.hypervHostStatus.Up.value,
              DataMap.hypervHostStatus.Ok.value
            ],
            this.source.extendInfo.status
          )
            ? '1'
            : '0';
        break;
      case DataMap.Resource_Type.nutanixVm.value:
        this.formItems[0][2].content = this.source?.extendInfo?.status;
        break;
    }
    this.formItems[1][0].content = this.source.protection_status;
    this.formItems[1][1].content = this.source.sla_name;
    this.formItems[1][2].content = this.source.sla_compliance;
  }

  getAutoRefresh() {
    this.subscription$ = this.globalService
      .getState('autoReshResource')
      .subscribe(res => {
        assign(this.source, res);
        this.getResourceInfo();
      });
  }

  ngOnInit() {
    this.getResourceInfo();
    this.getAutoRefresh();
  }
}
