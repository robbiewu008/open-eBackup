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
import { Component, Input, OnInit } from '@angular/core';
import {
  CommonConsts,
  DataMap,
  extendSlaInfo,
  I18NService,
  ProtectedResourceApiService
} from 'app/shared';
import { SlaService } from 'app/shared/services/sla.service';
import { assign, each } from 'lodash';
import { map } from 'rxjs/operators';

@Component({
  selector: 'aui-associated-fileset',
  templateUrl: './associated-fileset.component.html',
  styleUrls: ['./associated-fileset.component.less']
})
export class AssociatedFilesetComponent implements OnInit {
  @Input() rowItem;
  @Input() lvScroll;
  sizeOptions = CommonConsts.PAGE_SIZE_OPTIONS;
  pageSize = CommonConsts.PAGE_SIZE;
  pageIndex = CommonConsts.PAGE_START;
  total = CommonConsts.PAGE_TOTAL;
  filesetData = [];

  columns = [
    {
      label: this.i18n.get('common_name_label'),
      key: 'name'
    },
    {
      label: this.i18n.get('protection_host_name_label'),
      key: 'environment_name'
    },
    {
      label: this.i18n.get('common_ip_address_label'),
      key: 'environment_endpoint'
    },
    {
      label: this.i18n.get('common_sla_label'),
      key: 'sla_name'
    },
    {
      label: this.i18n.get('protection_protected_status_label'),
      key: 'protection_status'
    }
  ];

  constructor(
    private i18n: I18NService,
    private slaService: SlaService,
    private protectedResourceApiService: ProtectedResourceApiService
  ) {}

  ngOnInit() {
    this.getFilesets();
  }

  pageChange(page) {
    this.pageIndex = page.pageIndex;
    this.pageSize = page.pageSize;
    this.getFilesets();
  }

  getFilesets() {
    const params = {
      pageNo: this.pageIndex,
      pageSize: this.pageSize,
      conditions: JSON.stringify({
        subType: [DataMap.Resource_Type.fileset.value],
        templateId: this.rowItem.uuid
      })
    };
    this.protectedResourceApiService
      .ListResources({
        ...params
      })
      .pipe(
        map(res => {
          each(res.records, item => {
            extendSlaInfo(item);
          });
          return res;
        })
      )
      .subscribe(res => {
        each(res.records, item => {
          assign(item, {
            environment_name: item.environment?.name,
            environment_endpoint: item.environment?.endpoint,
            protection_status: item['protectionStatus']
          });
        });
        this.filesetData = res.records;
        this.total = res.totalCount;
      });
  }

  getSlaDetail(data) {
    this.slaService.getDetail({
      uuid: data.sla_id,
      name: data.sla_name
    });
  }
}
