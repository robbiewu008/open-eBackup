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
  ChangeDetectionStrategy,
  ChangeDetectorRef,
  Component,
  EventEmitter,
  Input,
  OnInit,
  Output
} from '@angular/core';
import {
  CommonConsts,
  GROUP_COMMON,
  I18NService,
  LANGUAGE,
  QosService,
  ResourceSetApiService,
  ResourceSetType
} from 'app/shared';
import { assign, each, isEmpty, map, set, trim } from 'lodash';

@Component({
  selector: 'aui-qos-template',
  templateUrl: './qos-template.component.html',
  styleUrls: ['./qos-template.component.less'],
  changeDetection: ChangeDetectionStrategy.OnPush
})
export class QosTemplateComponent implements OnInit {
  @Input() allSelectionMap;
  @Input() data;
  @Input() isDetail;
  @Output() allSelectChange = new EventEmitter<any>();

  isAllSelect = false; // 用来标记是否全选
  pageIndex = CommonConsts.PAGE_START;
  pageSize = CommonConsts.PAGE_SIZE;
  total = CommonConsts.PAGE_TOTAL;
  sizeOptions = CommonConsts.PAGE_SIZE_OPTIONS;
  policyName;
  qosData = [];
  qosSelection = [];
  language = LANGUAGE;
  buttonLabel = this.i18n.get('system_resourceset_all_select_label');

  groupCommon = GROUP_COMMON;

  constructor(
    public i18n: I18NService,
    private cdr: ChangeDetectorRef,
    private qosServiceApi: QosService,
    private resourceSetService: ResourceSetApiService
  ) {}

  ngOnInit() {
    this.getQos();
    if (!!this.allSelectionMap[ResourceSetType.QOS]?.isAllSelected) {
      // 重新进入时回显选中的数据
      this.isAllSelect = true;
    }
  }

  getQos() {
    this.qosSelection = [];
    const params = { pageNo: this.pageIndex, pageSize: this.pageSize };
    const defaultCondition = {};
    if (this.isDetail) {
      assign(defaultCondition, {
        resource_set_id: this.data[0].uuid
      });
    }
    if (this.policyName) {
      assign(defaultCondition, {
        name: trim(this.policyName)
      });
    }
    if (!isEmpty(defaultCondition)) {
      assign(params, {
        conditions: JSON.stringify(defaultCondition)
      });
    }

    this.qosServiceApi.queryResourcesV1QosGet(params).subscribe(
      res => {
        this.qosData = res.items;
        this.total = res.total;
        if (!isEmpty(this.allSelectionMap[ResourceSetType.QOS]?.data)) {
          // 重新进入时回显选中的数据
          this.qosSelection = this.allSelectionMap[ResourceSetType.QOS].data;
        }

        if (!!this.allSelectionMap[ResourceSetType.QOS]?.isAllSelected) {
          this.allSelect(false);
        }

        if (
          !!this.data &&
          !this.isDetail &&
          !(ResourceSetType.QOS in this.allSelectionMap)
        ) {
          this.getSelectedData();
        }
        this.cdr.detectChanges();
      },
      err => {
        this.qosData = [];
        this.total = 0;
        this.cdr.detectChanges();
      }
    );
  }

  getSelectedData() {
    const params: any = {
      resourceSetId: this.data[0].uuid,
      scopeModule: ResourceSetType.QOS,
      type: ResourceSetType.QOS
    };

    this.resourceSetService.queryResourceObjectIdList(params).subscribe(res => {
      set(this.allSelectionMap, ResourceSetType.QOS, {
        data: map(res, item => {
          return { uuid: item };
        })
      });
      this.qosSelection = this.allSelectionMap[ResourceSetType.QOS].data;
    });
  }

  allSelect(turnPage?) {
    const isAllSelected = !!turnPage ? !this.isAllSelect : this.isAllSelect;
    set(this.allSelectionMap, ResourceSetType.QOS, { isAllSelected });
    this.qosSelection = isAllSelected ? [...this.qosData] : [];
    each(this.qosData, item => {
      item.disabled = isAllSelected;
    });
    this.isAllSelect = isAllSelected;
    this.buttonLabel = this.i18n.get(
      isAllSelected
        ? 'system_resourceset_cancel_all_select_label'
        : 'system_resourceset_all_select_label'
    );
    this.allSelectChange.emit();
    this.cdr.detectChanges();
  }

  selectionChange(e) {
    set(this.allSelectionMap, `QOS`, {
      data: this.qosSelection
    });
    this.allSelectChange.emit();
  }

  searchByName(name) {
    this.policyName = name;
    this.getQos();
  }

  pageChange(page) {
    this.pageSize = page.pageSize;
    this.pageIndex = page.pageIndex;
    this.getQos();
  }
}
