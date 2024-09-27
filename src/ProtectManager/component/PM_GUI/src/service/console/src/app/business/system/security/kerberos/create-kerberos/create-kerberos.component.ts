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
  AbstractControl,
  FormBuilder,
  FormControl,
  FormGroup,
  ValidationErrors,
  ValidatorFn
} from '@angular/forms';
import { MessageService, UploadFile } from '@iux/live';
import {
  BaseUtilService,
  I18NService,
  KerberosConfigMode,
  KerberosAPIService,
  CommonConsts
} from 'app/shared';
import { size, trim } from 'lodash';
import { Observable, Observer } from 'rxjs';

@Component({
  selector: 'aui-create-kerberos',
  templateUrl: './create-kerberos.component.html',
  styleUrls: ['./create-kerberos.component.less']
})
export class CreateKerberosComponent implements OnInit {
  data;
  selectedKrb5File;
  selectedKeytabFile;
  keytabFilters = [];
  configFilters = [];
  kerberosConfigMode = KerberosConfigMode;
  formGroup: FormGroup;

  nameErrorTip = {
    ...this.baseUtilService.nameErrorTip,
    invalidName: this.i18n.get('common_valid_archive_storage_name_label')
  };
  configModeErrorTip = {
    ...this.baseUtilService.requiredErrorTip
  };
  passwordErrorTip = {
    ...this.baseUtilService.pwdErrorTip,
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [2048])
  };
  principalNameErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [64])
  };

  constructor(
    private fb: FormBuilder,
    private i18n: I18NService,
    private messageService: MessageService,
    public kerberosApi: KerberosAPIService,
    public baseUtilService: BaseUtilService
  ) {}

  ngOnInit() {
    this.initForm();
    this.initFilters();
    this.updateData();
  }

  updateData() {
    if (!this.data) {
      return;
    }

    setTimeout(() => {
      this.formGroup.patchValue({ ...this.data, password: '' });
    }, 0);
  }

  initForm() {
    this.formGroup = this.fb.group(
      {
        name: new FormControl('', {
          validators: [
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.name(CommonConsts.REGEX.cloudStorageName)
          ]
        }),
        principalName: new FormControl('', {
          validators: [
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.maxLength(64)
          ]
        }),
        createModel: new FormControl(this.kerberosConfigMode.pwd, {
          validators: [this.baseUtilService.VALID.required()]
        }),
        keytabFile: new FormControl(''),
        password: new FormControl('', {
          validators: [
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.maxLength(2048)
          ]
        }),
        confirmpwd: new FormControl('', {
          validators: [
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.maxLength(2048)
          ]
        }),
        krb5File: new FormControl('', {
          validators: [this.baseUtilService.VALID.required()]
        })
      },
      {
        validators: this.validatorForm
      }
    );

    this.listenFormGroup();
  }

  validatorForm: ValidatorFn = (
    control: AbstractControl
  ): ValidationErrors | null => {
    const password = trim(control.get('password').value);
    const configPassword = trim(control.get('confirmpwd').value);
    if (control.get('createModel').value === KerberosConfigMode.pwd) {
      if (password && configPassword) {
        if (password !== configPassword) {
          control.get('password').setErrors({ diffPwd: true });
          control.get('confirmpwd').setErrors({ diffPwd: true });
        } else {
          control.get('password').setErrors(null);
          control.get('confirmpwd').setErrors(null);
        }
      }
    } else {
      control.get('password').setErrors(null);
      control.get('confirmpwd').setErrors(null);
    }

    return null;
  };

  listenFormGroup() {
    this.formGroup.get('createModel').valueChanges.subscribe(res => {
      if (res === KerberosConfigMode.pwd) {
        this.formGroup
          .get('password')
          .setValidators([
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.maxLength(2048)
          ]);
        this.formGroup
          .get('confirmpwd')
          .setValidators([
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.maxLength(2048)
          ]);
        this.formGroup.get('keytabFile').setValue('');
        this.formGroup.get('keytabFile').clearValidators();
      } else {
        this.formGroup
          .get('keytabFile')
          .setValidators([this.baseUtilService.VALID.required()]);
        this.formGroup.get('password').clearValidators();
        this.formGroup.get('confirmpwd').clearValidators();
      }

      this.formGroup.get('password').updateValueAndValidity();
      this.formGroup.get('confirmpwd').updateValueAndValidity();
      this.formGroup.get('keytabFile').updateValueAndValidity();
    });
  }

  initFilters() {
    this.keytabFilters = [
      {
        name: 'suffix',
        filterFn: (files: UploadFile[]) => {
          const supportSuffix = ['keytab'];
          const validFiles = files.filter(file => {
            const suffix = file.name.split('.').pop();
            return supportSuffix.includes(suffix);
          });
          if (validFiles.length !== files.length) {
            this.messageService.error(
              this.i18n.get('common_format_error_label', ['keytab']),
              { lvMessageKey: 'fileFormatError' }
            );
            return false;
          }
          if (files[0].size > 1024 * 1024 * 1) {
            this.messageService.error(
              this.i18n.get('common_max_size_file_label', ['1MB']),
              { lvMessageKey: 'fileMaxSizeError' }
            );
            return false;
          }

          if (size(files) > 1) {
            this.messageService.error(
              this.i18n.get('common_upload_files_num_label', [1]),
              {
                lvShowCloseButton: true,
                lvMessageKey: 'upload_files_num_key'
              }
            );
            return false;
          }
          this.selectedKeytabFile = files[0].originFile;
          this.formGroup.get('keytabFile').setValue(this.selectedKeytabFile);
          return validFiles;
        }
      }
    ];

    this.configFilters = [
      {
        name: 'suffix',
        filterFn: (files: UploadFile[]) => {
          const supportSuffix = ['conf'];
          const validFiles = files.filter(file => {
            const suffix = file.name.split('.').pop();
            return supportSuffix.includes(suffix);
          });
          if (validFiles.length !== files.length) {
            this.messageService.error(
              this.i18n.get('common_format_error_label', ['conf']),
              { lvMessageKey: 'fileFormatError' }
            );
            return false;
          }
          if (files[0].size > 1024 * 64) {
            this.messageService.error(
              this.i18n.get('common_max_size_file_label', ['64KB']),
              { lvMessageKey: 'fileMaxSizeError' }
            );
            return false;
          }

          if (size(files) > 1) {
            this.messageService.error(
              this.i18n.get('common_upload_files_num_label', [1]),
              {
                lvShowCloseButton: true,
                lvMessageKey: 'upload_files_num_key'
              }
            );
            return false;
          }
          this.selectedKrb5File = files[0].originFile;
          this.formGroup.get('krb5File').setValue(this.selectedKrb5File);
          return validFiles;
        }
      }
    ];
  }

  onOK(): Observable<any> {
    return this.data ? this.onModify() : this.onCreate();
  }

  onCreate(): Observable<any> {
    return new Observable<any>((observer: Observer<any>) => {
      if (this.formGroup.invalid) {
        return;
      }
      const params = this.formGroup.value;
      if (params.createModel === this.kerberosConfigMode.file) {
        delete params.password;
      }
      this.kerberosApi
        .createKerberosUsingPOST({
          ...params,
          krb5File: this.selectedKrb5File,
          keytabFile: this.selectedKeytabFile
        })
        .subscribe(
          res => {
            observer.next(res.kerberos_id);
            observer.complete();
          },
          error => {
            observer.error(error);
            observer.complete();
          }
        );
    });
  }

  onModify(): Observable<any> {
    return new Observable<any>((observer: Observer<any>) => {
      if (this.formGroup.invalid) {
        return;
      }
      const params = this.formGroup.value;
      if (params.createModel === this.kerberosConfigMode.file) {
        delete params.password;
      }
      this.kerberosApi
        .updateKerberosUsingPOST({
          ...params,
          kerberosId: this.data.kerberosId,
          krb5File: this.selectedKrb5File,
          keytabFile: this.selectedKeytabFile
        })
        .subscribe(
          res => {
            observer.next(res);
            observer.complete();
          },
          error => {
            observer.error(error);
            observer.complete();
          }
        );
    });
  }
}
