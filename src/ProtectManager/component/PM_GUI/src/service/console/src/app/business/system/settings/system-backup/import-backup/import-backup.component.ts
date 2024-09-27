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
import { Subject, Observable, Observer } from 'rxjs';
import { MessageService, UploadFile } from '@iux/live';
import {
  BaseUtilService,
  DataMap,
  I18NService,
  SysbackupApiService,
  WarningMessageService
} from 'app/shared';
import { assign, get, isEmpty, size } from 'lodash';
import { FormBuilder, FormControl, FormGroup } from '@angular/forms';
@Component({
  selector: 'aui-import-backup',
  templateUrl: './import-backup.component.html',
  styleUrls: ['./import-backup.component.less']
})
export class ImportBackupComponent implements OnInit {
  selectedFile;
  filters = [];
  valid$ = new Subject<boolean>();
  formGroup: FormGroup;
  isCyberengine =
    this.i18n.get('deploy_type') === DataMap.Deploy_Type.cyberengine.value;
  pwdErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    invalidMinLength: this.i18n.get('common_valid_minlength_label', [8])
  };
  constructor(
    public fb: FormBuilder,
    public i18n: I18NService,
    public messageService: MessageService,
    private sysbackupApiService: SysbackupApiService,
    public baseUtilService: BaseUtilService,
    private warningMessageService: WarningMessageService
  ) {}

  ngOnInit() {
    this.initForm();
    this.initFilters();
  }

  initForm() {
    this.formGroup = this.fb.group({
      password: new FormControl('', {
        updateOn: 'change'
      })
    });
    // 只有备份软件需要password
    if (this.isCyberengine) {
      this.formGroup.get('password').clearValidators();
    } else {
      this.formGroup
        .get('password')
        .setValidators([
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.minLength(8)
        ]);
    }
    this.listenForm();
  }
  listenForm() {
    this.formGroup.get('password').statusChanges.subscribe(res => {
      this.valid$.next(res === 'VALID' && !isEmpty(this.selectedFile));
    });
  }
  initFilters() {
    this.filters = [
      {
        name: 'suffix',
        filterFn: (files: UploadFile[]) => {
          const supportSuffix = ['zip'];
          const validFiles = files.filter(file => {
            const suffix = file.name.split('.').pop();
            return supportSuffix.includes(suffix);
          });

          if (validFiles.length !== files.length) {
            this.messageService.error(
              this.i18n.get('common_format_error_label', ['zip']),
              {
                lvShowCloseButton: true,
                lvMessageKey: 'format_error_key'
              }
            );
            this.valid$.next(false);
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
            this.valid$.next(false);
            return false;
          }

          if (files[0].size > 1024 * 1024 * 1024 * 4) {
            this.messageService.error(
              this.i18n.get('common_max_size_file_label', ['4GB']),
              {
                lvShowCloseButton: true,
                lvMessageKey: 'max_size_file_key'
              }
            );
            this.valid$.next(false);
            return false;
          }
          return validFiles;
        }
      }
    ];
  }

  private _clear() {
    this.selectedFile = [];
    this.valid$.next(false);
  }

  crlFilesChange(files) {
    if (size(files) === 0) {
      this.selectedFile = [];
      this.valid$.next(false);
    } else {
      this.selectedFile = files[0];
      this.valid$.next(this.isCyberengine || this.formGroup.status === 'VALID');
    }
  }

  filesChange(e) {
    e?.action === 'remove' && this._clear();
  }

  uploadBackup(params, observer) {
    this.sysbackupApiService.uploadBackupUsingPOST(params).subscribe({
      next: () => {
        observer.next();
        observer.complete();
      },
      error: error => {
        if (
          get(error, 'error.errorCode') === '1677934081' &&
          get(error, 'error.errorMessage') ===
            `The file verify signature failed.`
        ) {
          this.warningMessageService.create({
            content: this.i18n.get(
              'protection_file_verify_signature_failed_label'
            ),
            okText: this.i18n.get('protection_continue_import_label'),
            onOK: () => {
              assign(params, {
                needSignVerify: false
              });
              this.uploadBackup(params, observer);
            },
            onCancel: () => {
              observer.error(null);
              observer.complete();
            }
          });
        } else {
          observer.error(error);
          observer.complete();
        }
      }
    });
  }

  onOk(): Observable<void> {
    return new Observable<void>((observer: Observer<void>) => {
      if (!this.isCyberengine) {
        this.warningMessageService.create({
          content: this.i18n.get('system_import_backup_warn_label'),
          onOK: () => {
            if (
              this.formGroup.status === 'INVALID' ||
              isEmpty(this.selectedFile)
            ) {
              return;
            }
            const params = {
              file: this.selectedFile.originFile,
              password: this.formGroup.get('password').value
            };
            this.uploadBackup(params, observer);
          },
          onCancel: () => {
            observer.error(null);
            observer.complete();
          },
          lvAfterClose: result => {
            if (result && result.trigger === 'close') {
              observer.error(null);
              observer.complete();
            }
          }
        });
      } else {
        const params = { file: this.selectedFile.originFile };
        this.uploadBackup(params, observer);
      }
    });
  }
}
