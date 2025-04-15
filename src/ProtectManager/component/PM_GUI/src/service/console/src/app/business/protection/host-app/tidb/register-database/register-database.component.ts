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
import { FormBuilder, FormControl, FormGroup } from '@angular/forms';
import {
  BaseUtilService,
  CommonConsts,
  DataMap,
  I18NService,
  ProtectedEnvironmentApiService,
  ProtectedResourceApiService
} from 'app/shared';
import { cacheGuideResource } from 'app/shared/consts/guide-config';
import { each, filter, isNumber } from 'lodash';
import { Observable, Observer } from 'rxjs';

@Component({
  selector: 'aui-register-database',
  templateUrl: './register-database.component.html',
  styleUrls: ['./register-database.component.less']
})
export class RegisterDatabaseComponent implements OnInit {
  formGroup: FormGroup;
  databaseOptions = [];
  clusterOptions = [];
  pageIndex = CommonConsts.PAGE_START;
  pageSize = CommonConsts.PAGE_SIZE / 2;
  rowData;
  dataMap = DataMap;

  nameErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    ...this.baseUtilService.nameErrorTip,
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [64])
  };

  constructor(
    public baseUtilService: BaseUtilService,
    private fb: FormBuilder,
    private i18n: I18NService,
    private protectedResourceApiService: ProtectedResourceApiService,
    private protectedEnvironmentApiService: ProtectedEnvironmentApiService
  ) {}

  ngOnInit() {
    this.initForm();
    if (!this.rowData) {
      this.getData();
    }
  }

  getData(recordsTemp?, startPage?) {
    const params = {
      pageNo: startPage || this.pageIndex,
      pageSize: this.pageSize,
      akloading: false,
      conditions: JSON.stringify({
        subType: DataMap.Resource_Type.tidbCluster.value
      })
    };
    this.protectedResourceApiService.ListResources(params).subscribe(res => {
      if (!recordsTemp) {
        recordsTemp = [];
      }
      if (!isNumber(startPage)) {
        startPage = CommonConsts.PAGE_START;
      }
      startPage++;
      recordsTemp = [...recordsTemp, ...res.records];
      if (
        startPage === Math.ceil(res.totalCount / CommonConsts.PAGE_SIZE) ||
        res.totalCount === 0
      ) {
        const hostArray = [];
        each(res.records, item => {
          hostArray.push({
            ...item,
            key: item.uuid,
            value: item.uuid,
            label: `${item.name}`,
            isLeaf: true
          });
        });
        this.clusterOptions = hostArray;
        if (this.rowData) {
          this.getDatabase(this.formGroup.value.cluster);
        }
        return;
      }
      this.getData(recordsTemp, startPage);
    });
  }

  getDatabase(res?) {
    let tmp: any = filter(this.clusterOptions, item => {
      return item.uuid === res;
    });
    let conditions = {
      action_type: 'list_db',
      clusterName: tmp[0].extendInfo.clusterName,
      tiupPath: tmp[0].extendInfo.tiupPath,
      isCluster: false,
      agentIds: [tmp[0].extendInfo.tiupUuid]
    };
    this.protectedEnvironmentApiService
      .ListEnvironmentResource({
        envId: tmp[0].extendInfo.tiupUuid,
        pageSize: this.pageSize,
        pageNo: this.pageIndex,
        resourceSubType: this.dataMap.Resource_Type.tidbCluster.value,
        resourceType: this.dataMap.Resource_Type.tidbCluster.value,
        agentId: res,
        conditions: JSON.stringify(conditions)
      })
      .subscribe(res => {
        const dataArray = [];
        each(JSON.parse(res.records[0].extendInfo.database), item => {
          dataArray.push({
            key: item,
            value: item,
            label: `${item}`,
            isLeaf: true
          });
        });
        this.databaseOptions = dataArray;
      });
  }

  initForm() {
    this.formGroup = this.fb.group({
      name: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.name(),
          this.baseUtilService.VALID.maxLength(64)
        ]
      }),
      database: new FormControl([], {
        validators: [this.baseUtilService.VALID.required()]
      }),
      cluster: new FormControl([], {
        validators: [this.baseUtilService.VALID.required()]
      })
    });
    if (this.rowData) {
      this.formGroup.get('name').setValue(this.rowData.name);
      this.formGroup
        .get('database')
        .setValue(this.rowData.extendInfo.databaseName);
      this.formGroup.get('cluster').setValue(this.rowData?.parentUuid);
      this.getData();
    }
    this.formGroup.get('cluster').valueChanges.subscribe(res => {
      this.getDatabase(res);
    });
  }

  getParams() {
    let tmp: any = filter(this.clusterOptions, item => {
      return item.uuid === this.formGroup.value.cluster;
    });
    const params: any = {
      name: this.formGroup.value.name,
      type: 'Database',
      subType: this.dataMap.Resource_Type.tidbDatabase.value,
      extendInfo: {
        databaseName: this.formGroup.value.database,
        clusterName: tmp[0].extendInfo.clusterName,
        save_type: this.rowData ? 1 : 0,
        linkStatus: '1'
      },
      auth: tmp[0].auth,
      parentUuid: this.formGroup.value.cluster,
      parentName: tmp[0].name
    };
    return params;
  }

  onOK(): Observable<void> {
    return new Observable<void>((observer: Observer<void>) => {
      if (this.formGroup.invalid) {
        return;
      }
      const params = this.getParams();
      if (this.rowData) {
        this.protectedResourceApiService
          .UpdateResource({
            resourceId: this.rowData.uuid,
            UpdateResourceRequestBody: params
          })
          .subscribe({
            next: res => {
              observer.next();
              observer.complete();
            },
            error: err => {
              observer.error(err);
              observer.complete();
            }
          });
      } else {
        this.protectedResourceApiService
          .CreateResource({
            CreateResourceRequestBody: params
          })
          .subscribe({
            next: res => {
              cacheGuideResource(res);
              observer.next();
              observer.complete();
            },
            error: err => {
              observer.error(err);
              observer.complete();
            }
          });
      }
    });
  }
}
