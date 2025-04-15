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
  FormBuilder,
  FormControl,
  FormGroup,
  Validators
} from '@angular/forms';
import { UploadFile, MessageService, OptionItem } from '@iux/live';
import {
  BaseUtilService,
  CommonConsts,
  ComponentRestApiService,
  DataMapService,
  I18NService,
  DataMap,
  CookieService,
  WarningMessageService
} from 'app/shared';
import { AppUtilsService } from 'app/shared/services/app-utils.service';
import { find, includes, set } from 'lodash';
import { Subject } from 'rxjs';

@Component({
  selector: 'aui-add-components',
  templateUrl: './add-components.component.html',
  styles: [
    `
      .lv-fileupload {
        width: 100%;
      }
    `
  ]
})
export class AddComponentsComponent implements OnInit {
  formGroup: FormGroup;
  validCer$ = new Subject<boolean>();
  selectCaCertificateFile;
  selectCertificateFile;
  componentType = this.dataMapService.getConfig('Component_Type');
  certificateFileLabel = this.i18n.get('system_certificate_file_label');
  requiredLabel = this.i18n.get('common_required_label');
  nameValidLabel = this.i18n.get('system_component_name_valid_label');
  nameErrorTip = {
    invalidName: this.nameValidLabel,
    required: this.requiredLabel
  };
  filters = [];
  typeOptions = this.dataMapService
    .toArray('Component_Type')
    .filter((v: OptionItem) => {
      v.isLeaf = true;
      return !this.cookieService.isCloudBackup
        ? this.i18n.get('deploy_type') === DataMap.Deploy_Type.cyberengine.value
          ? includes(
              [
                DataMap.Component_Type.email.value,
                DataMap.Component_Type.externalStorage.value,
                DataMap.Component_Type.syslog.value,
                DataMap.Component_Type.ldap.value
              ],
              v.value
            )
          : !includes(
              [
                DataMap.Component_Type.internal.value,
                DataMap.Component_Type.vmware.value,
                DataMap.Component_Type.communicationComponent.value,
                DataMap.Component_Type.redisComponent.value,
                DataMap.Component_Type.other.value,
                DataMap.Component_Type.protectAgent.value,
                DataMap.Component_Type.ha.value,
                DataMap.Component_Type.adfs.value
              ],
              v.value
            )
        : v.value === DataMap.Component_Type.s3.value;
    });
  constructor(
    public i18n: I18NService,
    public fb: FormBuilder,
    public certApiService: ComponentRestApiService,
    public dataMapService: DataMapService,
    public baseUtilService: BaseUtilService,
    public message: MessageService,
    public cookieService: CookieService,
    private warningMessageService: WarningMessageService,
    private appUtilsService: AppUtilsService
  ) {}

  initForm() {
    this.formGroup = this.fb.group({
      name: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.name(CommonConsts.REGEX.componentName)
        ]
      }),
      type: this.cookieService.isCloudBackup
        ? new FormControl([DataMap.Component_Type.s3.value])
        : new FormControl([], Validators.required)
    });
    this.filters = [
      {
        name: 'suffix',
        filterFn: (files: UploadFile[]) => {
          const supportSuffix = ['pem'];
          const validFiles = files.filter(file => {
            const suffix = file.name.split('.').pop();
            return supportSuffix.includes(suffix);
          });

          if (validFiles.length !== files.length) {
            this.message.error(
              this.i18n.get('common_format_error_label', ['pem']),
              {
                lvMessageKey: 'formatErrorKey',
                lvShowCloseButton: true
              }
            );
            this.validCer$.next(false);
            return validFiles;
          }
          if (files[0].size > 1024 * 1024) {
            this.message.error(
              this.i18n.get('common_max_size_file_label', ['1MB']),
              {
                lvMessageKey: 'maxSizeFileErrorKey',
                lvShowCloseButton: true
              }
            );
            this.validCer$.next(false);
            return validFiles;
          }
          this.selectCaCertificateFile = files[0].originFile;
          this.validCer$.next(true);
          return validFiles;
        }
      }
    ];
  }

  registerComponent(cb?: () => void) {
    this.certApiService
      .getCertDetailUsingPOST({
        cert: this.selectCaCertificateFile,
        akOperationTips: false
      })
      .subscribe(res => {
        if (!res.safety) {
          this.warningMessageService.create({
            content: this.i18n.get('system_cert_safety_tips_label', [
              this.selectCaCertificateFile['name']
            ]),
            onOK: () => this.register(cb)
          });
          return;
        }

        this.register(cb);
      });
  }

  register(cb) {
    this.certApiService
      .registerComponentUsingPOST1({
        name: this.formGroup.value.name,
        type: this.formGroup.value.type.join(),
        ca: this.selectCaCertificateFile,
        sync: true
      })
      .subscribe(res => cb());
  }

  ngOnInit() {
    if (this.appUtilsService.isDecouple || this.appUtilsService.isDistributed) {
      const a8000Type = find(this.typeOptions, {
        value: DataMap.Component_Type.a8000.value
      });
      set(
        a8000Type,
        'label',
        this.i18n.get('common_e_series_cluster_type_label')
      );
    }
    this.initForm();
  }
}
