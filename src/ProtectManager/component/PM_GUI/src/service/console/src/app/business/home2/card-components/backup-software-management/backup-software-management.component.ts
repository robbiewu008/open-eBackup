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
import { Component, Input } from '@angular/core';
import {
  CapacityCalculateLabel,
  DataMap,
  ExternalSystemService,
  I18NService
} from 'app/shared';
import { each, toNumber, toUpper } from 'lodash';
import { finalize } from 'rxjs/operators';
import ShowExternalSystemInfoParams = ExternalSystemService.ShowExternalSystemInfoParams;

@Component({
  selector: 'aui-backup-software-management',
  templateUrl: './backup-software-management.component.html',
  styleUrls: ['./backup-software-management.component.less'],
  providers: [CapacityCalculateLabel]
})
export class BackupSoftwareManagementComponent {
  @Input() cardInfo: any = {};
  dataMap = DataMap;
  externalSystemList = [];
  constructor(
    private i18n: I18NService,
    private externalSystemService: ExternalSystemService
  ) {}

  ngOnInit(): void {
    this.cardInfo.loading = true;
    this.getExternalSystemList();
  }

  jumpToDPA(data) {
    // DPA跳转用ip+端口跳转
    const url = `https://${encodeURI(data.endpoint)}`;
    window.open(url, '_blank');
  }

  getExternalSystemList() {
    const params: ShowExternalSystemInfoParams = { akLoading: false };
    this.externalSystemService
      .getAllStatus(params)
      .pipe(
        finalize(() => {
          this.cardInfo.loading = false;
        })
      )
      .subscribe((res: any) => {
        each(res, item => {
          item.usedSize = toNumber(item.usedSize);
          item.totalSize = toNumber(item.totalSize);
          item.tagLabel = [
            {
              label: item.status
            },
            {
              label: toUpper(item.type)
            }
          ];
          item.usedPercentage =
            Math.round((item.usedSize / (item.totalSize || 1)) * 100 * 100) /
            100;
        });
        this.externalSystemList = res.slice(0, 2);
        this.cardInfo.title = res?.length
          ? `${this.i18n.get('common_external_associated_systems_label')}(${
              res.length
            })`
          : `${this.i18n.get('common_external_associated_systems_label')}`;
      });
  }
}
