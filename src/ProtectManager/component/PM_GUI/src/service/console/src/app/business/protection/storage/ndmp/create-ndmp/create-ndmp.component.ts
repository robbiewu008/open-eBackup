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
  ProtectedResourceApiService
} from 'app/shared';
import { AppUtilsService } from 'app/shared/services/app-utils.service';
import { assign, find, map } from 'lodash';
import { Observable, Observer } from 'rxjs';

@Component({
  selector: 'aui-create-ndmp',
  templateUrl: './create-ndmp.component.html',
  styleUrls: ['./create-ndmp.component.less']
})
export class CreateNdmpComponent implements OnInit {
  rowData;
  formGroup: FormGroup;
  equipmentOptions = [];
  fileSystemOptions = [];

  MAX_PATH_LENGTH = 4096;

  nameErrorTip: any = {
    ...this.baseUtilService.nameErrorTip
  };
  directoryErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    invalidName: this.i18n.get('common_path_error_label'),
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [
      this.MAX_PATH_LENGTH
    ])
  };

  constructor(
    private fb: FormBuilder,
    private i18n: I18NService,
    private baseUtilService: BaseUtilService,
    private appUtilsService: AppUtilsService,
    private protectedResourceApiService: ProtectedResourceApiService
  ) {}

  ngOnInit(): void {
    this.initForm();
    this.getEquipmentOptions();
    this.patchForm();
  }

  initForm() {
    this.formGroup = this.fb.group({
      name: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.name()
        ]
      }),
      equipment: new FormControl('', {
        validators: [this.baseUtilService.VALID.required()]
      }),
      fileSystem: new FormControl('', {
        validators: [this.baseUtilService.VALID.required()]
      }),
      directory: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.name(CommonConsts.REGEX.linuxPath),
          this.baseUtilService.VALID.maxLength(this.MAX_PATH_LENGTH)
        ]
      })
    });

    this.formGroup.get('equipment').valueChanges.subscribe(res => {
      if (!res) {
        return;
      }
      this.getFileSystemOptions(res);
    });
  }

  patchForm() {
    if (!this.rowData) {
      return;
    }
    this.formGroup.patchValue({
      equipment: this.rowData.rootUuid,
      fileSystem: this.rowData.parentUuid,
      directory: this.rowData.path
    });
  }

  getEquipmentOptions() {
    const extParams = {
      conditions: JSON.stringify({
        subType: [DataMap.Device_Storage_Type.ndmp.value]
      })
    };
    this.appUtilsService.getResourceByRecursion(
      extParams,
      params => this.protectedResourceApiService.ListResources(params),
      resource => {
        this.equipmentOptions = map(resource, item => {
          assign(item, {
            label: item.name,
            isLeaf: true
          });
          return item;
        });
      }
    );
  }

  getFileSystemOptions(envId) {
    const extParams = {
      conditions: JSON.stringify({
        subType: [DataMap.Resource_Type.ndmp.value],
        parentUuid: envId
      })
    };
    this.appUtilsService.getResourceByRecursion(
      extParams,
      params => this.protectedResourceApiService.ListResources(params),
      resource => {
        this.fileSystemOptions = map(resource, item => {
          assign(item, {
            label: item.name,
            isLeaf: true
          });
          return item;
        });
      }
    );
  }

  onOK(): Observable<void> {
    return new Observable<void>((observer: Observer<void>) => {
      const fileSystem = find(this.fileSystemOptions, {
        uuid: this.formGroup.value.fileSystem
      });
      const CreateResourceRequestBody = {
        name: this.formGroup.value.name,
        parentUuid: this.formGroup.value.fileSystem,
        parentName: fileSystem?.name,
        type: 'NDMP',
        subType: DataMap.Resource_Type.ndmp.value,
        extendInfo: {
          isFs: '0',
          parentName: fileSystem?.name,
          tenantName: fileSystem?.extendInfo?.tenantName,
          fullName: `${fileSystem?.extendInfo?.fullName || ''}${
            this.formGroup.value.directory
          }`,
          directory: this.formGroup.value.directory
        }
      };
      if (this.rowData) {
        this.protectedResourceApiService
          .UpdateResource({
            resourceId: this.rowData.uuid,
            UpdateResourceRequestBody: CreateResourceRequestBody
          })
          .subscribe({
            next: () => {
              observer.next();
              observer.complete();
            },
            error: error => {
              observer.error(error);
              observer.complete();
            }
          });
      } else {
        this.protectedResourceApiService
          .CreateResource({
            CreateResourceRequestBody
          })
          .subscribe({
            next: () => {
              observer.next();
              observer.complete();
            },
            error: error => {
              observer.error(error);
              observer.complete();
            }
          });
      }
    });
  }
}
