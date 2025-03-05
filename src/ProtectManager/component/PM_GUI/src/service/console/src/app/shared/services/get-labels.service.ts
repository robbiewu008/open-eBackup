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
import { Observable, Subscriber } from 'rxjs';
import { FilterItem } from '@iux/live';
import { CommonConsts } from '../consts';
import { WarningMessageService } from './warning-message.service';
import { AppUtilsService } from './app-utils.service';
import { LabelApiService } from '../api/services/label-api.service';
import { Injectable } from '@angular/core';
@Injectable({
  providedIn: 'root'
})
export class GetLabelOptionsService {
  constructor(
    public warningMessageService: WarningMessageService,
    private appUtilsService: AppUtilsService,
    private labelApiService: LabelApiService
  ) {}
  labelOptionList = [];
  data = [];
  getLabelOptions(): Observable<FilterItem[]> {
    const extParams = {
      startPage: CommonConsts.PAGE_START_EXTRA,
      akLoading: false
    };
    return new Observable(observer => {
      this.fetchLabelOptions(extParams, observer);
    });
  }

  private fetchLabelOptions(
    extParams: { startPage: number; akLoading: boolean },
    observer: Subscriber<FilterItem[]>
  ) {
    this.appUtilsService.getResourceByRecursion(
      extParams,
      params => this.labelApiService.queryLabelUsingGET(params),
      res => {
        const arr = res?.map(item => {
          return {
            key: item.uuid,
            label: item.name,
            value: item.uuid
          };
        });
        this.labelOptionList = [...arr];
        observer.next(this.labelOptionList);
      }
    );
  }

  getAllLabelOptions() {
    const extParams = {
      startPage: CommonConsts.PAGE_START_EXTRA,
      akLoading: true
    };
    this.appUtilsService.getResourceByRecursion(
      extParams,
      params => this.labelApiService.queryLabelUsingGET(params),
      res => {
        this.data = res?.map(item => {
          return {
            id: item.uuid,
            label: item.name,
            value: item.uuid,
            isLeaf: true
          };
        });
        return this.data;
      }
    );
  }
}
