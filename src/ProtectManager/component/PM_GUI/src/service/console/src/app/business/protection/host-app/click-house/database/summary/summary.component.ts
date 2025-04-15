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
import {
  DataMap,
  GlobalService,
  I18NService,
  ProtectedResourceApiService
} from 'app/shared';
import { assign, toNumber } from 'lodash';
import { Observable, Observer } from 'rxjs';

@Component({
  selector: 'aui-database-summary',
  templateUrl: './summary.component.html',
  styleUrls: ['./summary.component.css']
})
export class SummaryComponent implements OnInit {
  source;
  dbInfo;
  type;
  dataMap = DataMap;

  constructor(
    private i18n: I18NService,
    public globalService: GlobalService,
    private protectedResourceApiService: ProtectedResourceApiService
  ) {}

  ngOnInit() {}

  initDetailData(data) {
    this.source = assign(data, {
      link_status: toNumber(data.linkStatus)
    });
    this.type = DataMap.Resource_Type.ClickHouseDatabase.value;
    this.getDetail(data.environment_uuid || data?.environment?.uuid).subscribe(
      item => {
        this.source.link_status = (item as any).linkStatus;
        this.globalService.emitStore({
          action: 'autoReshResource',
          state: this.source
        });
      }
    );
  }

  getDetail(uuid) {
    return new Observable<object>((observer: Observer<object>) => {
      this.protectedResourceApiService
        .ShowResource({
          resourceId: uuid
        })
        .subscribe(
          item => {
            observer.next(item);
            observer.complete();
          },
          error => {
            observer.error(error);
            observer.complete();
          }
        );
    });
  }

  getData() {}
}
