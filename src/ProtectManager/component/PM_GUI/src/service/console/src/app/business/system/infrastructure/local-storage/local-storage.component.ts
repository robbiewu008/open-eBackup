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
import { DataMap, CookieService } from 'app/shared';
import { each } from 'lodash';
import { StorageAuthComponent } from './storage-auth/storage-auth.component';
import { StorageSummaryComponent } from './storage-summary/storage-summary.component';

@Component({
  selector: 'aui-local-storage',
  templateUrl: './local-storage.component.html',
  styleUrls: ['./local-storage.component.less']
})
export class LocalStorageComponent implements OnInit {
  @ViewChild(StorageSummaryComponent, { static: false })
  storageSummaryComponent: StorageSummaryComponent;

  @ViewChild(StorageAuthComponent, { static: false })
  StorageAuthComponent: StorageAuthComponent;

  constructor(public cookieService: CookieService) {}

  ngOnInit() {}

  onStatusChange(res) {
    each(res, item => {
      if (
        item.authType === 'serviceAuth' &&
        item.status === DataMap.Storage_Status.normal.value
      ) {
        this.storageSummaryComponent.getData();
      }

      if (item.authType === 'managerAuth') {
        this.storageSummaryComponent.ableJump =
          item.status === DataMap.Storage_Status.normal.value ? true : false;
      }
    });
  }

  openDeviceChange() {
    this.StorageAuthComponent.ngOnInit();
  }

  onChange() {
    this.StorageAuthComponent.ngOnInit();
    this.storageSummaryComponent.ngOnInit();
  }
}
