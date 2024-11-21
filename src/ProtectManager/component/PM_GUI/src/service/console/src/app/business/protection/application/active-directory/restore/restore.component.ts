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
import { FormBuilder, FormControl, FormGroup } from '@angular/forms';
import { ModalRef } from '@iux/live';
import { DataMap, RestoreV2LocationType, RestoreV2Type } from 'app/shared';
import { RestoreApiV2Service } from 'app/shared/api/services';
import { isString } from 'lodash';
import { Observable, Observer } from 'rxjs';

@Component({
  selector: 'aui-restore',
  templateUrl: './restore.component.html',
  styleUrls: ['./restore.component.less']
})
export class RestoreComponent implements OnInit {
  resourceData;
  instanceOptions = [];
  formGroup: FormGroup;
  restoreLocationType = RestoreV2LocationType;
  @Input() rowCopy;
  @Input() childResType;
  @Input() restoreType;
  isDrill;

  constructor(
    private fb: FormBuilder,
    private modal: ModalRef,
    private restoreV2Service: RestoreApiV2Service
  ) {}

  ngOnInit() {
    this.resourceData = isString(this.rowCopy.resource_properties)
      ? JSON.parse(this.rowCopy.resource_properties)
      : {};
    this.initForm();
  }

  initForm() {
    this.formGroup = this.fb.group({
      restoreLocation: new FormControl(RestoreV2LocationType.ORIGIN),
      originLocation: new FormControl({
        value: this.resourceData?.name,
        disabled: true
      })
    });
    this.modal.getInstance().lvOkDisabled =
      this.rowCopy?.resource_status === DataMap.Resource_Status.notExist.value;
  }

  getParams() {
    const params = {
      copyId: this.rowCopy.uuid,
      targetEnv:
        this.resourceData?.environment_uuid || this.resourceData?.root_uuid,
      restoreType: RestoreV2Type.CommonRestore,
      targetLocation: this.formGroup.value.restoreLocation,
      targetObject: this.resourceData?.uuid
    };
    return params;
  }

  restore(): Observable<void> {
    return new Observable<void>((observer: Observer<void>) => {
      const params = this.getParams();
      this.restoreV2Service
        .CreateRestoreTask({ CreateRestoreTaskRequestBody: params as any })
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
