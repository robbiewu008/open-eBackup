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
import { AfterViewInit, Component } from '@angular/core';
import { Router } from '@angular/router';
import { DetectReportsService, RouterUrl } from 'app/shared';
import { AppUtilsService } from 'app/shared/services/app-utils.service';
import { size } from 'lodash';

@Component({
  selector: 'aui-exceptions-file',
  templateUrl: './exceptions-file.component.html',
  styleUrls: ['./exceptions-file.component.less']
})
export class ExceptionsFileComponent implements AfterViewInit {
  tableData = [];

  total = 0;

  constructor(
    private router: Router,
    private appUtilsService: AppUtilsService,
    private detectReportsService: DetectReportsService
  ) {}

  ngAfterViewInit() {
    this.loadTabelData();
  }

  loadTabelData(loading: boolean = true) {
    this.detectReportsService
      .getExceptionFileSystems({
        akLoading: loading
      })
      .subscribe(res => {
        this.tableData = res.exceptionFileSystemList;
        this.total = size(res.exceptionFileSystemList);
      });
  }

  gotoSnapshot() {
    this.router.navigateByUrl(RouterUrl.ExploreSnapShotData);
  }

  gotoDetectionDetail(item) {
    this.appUtilsService.setCacheValue('resourceName', item.resourceName);
    this.router.navigateByUrl(RouterUrl.ExploreSnapShotData);
  }
}
