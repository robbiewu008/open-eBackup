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
import { Router } from '@angular/router';
import { CAPACITY_UNIT, LicenseApiService, RouterUrl } from 'app/shared';
import { assign, each } from 'lodash';

@Component({
  selector: 'app-license-capacity',
  templateUrl: './license-capacity.component.html',
  styleUrls: ['./license-capacity.component.less']
})
export class LicenseCapacityComponent implements OnInit {
  unitconst = CAPACITY_UNIT;
  licenses = [];

  constructor(
    private router: Router,
    private licenseApiService: LicenseApiService
  ) {}

  ngOnInit() {
    this.loadData();
  }

  loadData(loading: boolean = true) {
    this.licenseApiService
      .queryLicenseUsingGET({ akLoading: loading, akDoException: false })
      .subscribe(res => {
        this.licenses = res || [];

        each(this.licenses, item => {
          assign(item, {
            capacity: (item.usedCapacity / item.totalCapacity) * 100
          });
        });
      });
  }

  gotoLicense() {
    this.router.navigateByUrl(RouterUrl.SystemLicense);
  }
}
