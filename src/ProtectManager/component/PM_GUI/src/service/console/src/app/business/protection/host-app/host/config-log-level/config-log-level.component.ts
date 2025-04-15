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
import { Form, FormBuilder, FormControl, FormGroup } from '@angular/forms';
import {
  BaseUtilService,
  ClientManagerApiService,
  DataMap,
  DataMapService,
  I18NService
} from 'app/shared';
import { Observable, Observer } from 'rxjs';

@Component({
  selector: 'aui-config-log-level',
  templateUrl: './config-log-level.component.html',
  styleUrls: ['./config-log-level.component.less']
})
export class ConfigLogLevelComponent implements OnInit {
  data;
  currentLevel = DataMap.Log_Level.info.value;
  formGroup: FormGroup;
  logLevelOptions = this.dataMapService.toArray('Log_Level').filter(item => {
    return (item.isLeaf = true);
  });

  constructor(
    private fb: FormBuilder,
    private i18n: I18NService,
    private dataMapService: DataMapService,
    public baseUtilService: BaseUtilService,
    private clientManagerApiService: ClientManagerApiService
  ) {}

  ngOnInit() {
    if (this.data?.extendInfo?.logLeve) {
      this.currentLevel = this.data?.extendInfo?.logLeve;
    }
    this.initForm();
  }

  initForm() {
    this.formGroup = this.fb.group({
      log_level: new FormControl(this.currentLevel)
    });
  }

  onOK(): Observable<void> {
    return new Observable<void>((observer: Observer<void>) => {
      if (this.formGroup.invalid) {
        return;
      }

      const params = {
        logLevel: this.formGroup.value.log_level
      };

      this.clientManagerApiService
        .updateAgentLogConfigurationPUT({
          configParam: params,
          agentId: this.data.uuid
        })
        .subscribe(
          res => {
            observer.next();
            observer.complete();
          },
          err => {
            observer.error(err);
            observer.complete();
          }
        );
    });
  }
}
