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
import { I18NService } from 'app/shared';
import { AppUtilsService } from 'app/shared/services/app-utils.service';
import { filter, includes } from 'lodash';

@Component({
  selector: 'aui-application',
  templateUrl: './application.component.html',
  styleUrls: ['./application.component.less']
})
export class ApplicationComponent implements OnInit {
  subApp = [
    ...filter(this.appUtilsService.getApplicationConfig().database, app =>
      includes(['oralce', 'mysql', 'tdsql'], app.id)
    ),
    ...filter(this.appUtilsService.getApplicationConfig().virtualization, app =>
      includes(['vmware', 'cnware'], app.id)
    ),
    ...filter(this.appUtilsService.getApplicationConfig().fileService, app =>
      includes(
        this.appUtilsService.isDistributed
          ? ['nasshare', 'fileset', 'volume']
          : ['nasfilesystem', 'nasshare', 'fileset', 'volume'],
        app.id
      )
    )
  ];
  typeTitle = this.i18n.get('common_application_type_label');
  routerType = 'livemount';

  constructor(
    private i18n: I18NService,
    private appUtilsService: AppUtilsService
  ) {}

  ngOnInit(): void {}
}
