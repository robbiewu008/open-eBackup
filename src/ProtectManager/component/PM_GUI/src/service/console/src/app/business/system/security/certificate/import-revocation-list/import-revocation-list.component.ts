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
import { FormBuilder, FormGroup } from '@angular/forms';
import { I18NService, WarningMessageService } from 'app/shared';
import { Subject } from 'rxjs';
import { ComponentRestApiService } from 'app/shared/api/services';
import { UploadFile, MessageService } from '@iux/live';

@Component({
  selector: 'aui-import-revocation-list',
  templateUrl: './import-revocation-list.component.html',
  styles: [
    `
      .lv-fileupload {
        width: 100%;
      }
    `
  ]
})
export class ImportRevocationListComponent implements OnInit {
  formGroup: FormGroup;
  valid$ = new Subject<boolean>();
  currentComponent;
  selectFile;
  fileLabel = this.i18n.get('system_revocation_list_label');
  filters = [];

  constructor(
    public i18n: I18NService,
    public fb: FormBuilder,
    public certApiService: ComponentRestApiService,
    public message: MessageService,
    public warningMessageService: WarningMessageService
  ) {}

  initForm() {
    this.formGroup = this.fb.group({});
    this.filters = [
      {
        name: 'suffix',
        filterFn: (files: UploadFile[]) => {
          const supportSuffix = ['crl'];
          let validFiles = files.filter(file => {
            let suffix = file.name.split('.').pop();
            return supportSuffix.includes(suffix);
          });

          if (validFiles.length !== files.length) {
            this.message.error(
              this.i18n.get('common_format_error_label', ['crl']),
              {
                lvMessageKey: 'formatErrorKey',
                lvShowCloseButton: true
              }
            );
            this.valid$.next(false);
            return validFiles;
          }
          if (files[0].size > 5 * 1024) {
            this.message.error(
              this.i18n.get('common_max_size_file_label', ['5KB']),
              {
                lvMessageKey: 'maxSizeFileErrorKey',
                lvShowCloseButton: true
              }
            );
            this.valid$.next(false);
            return validFiles;
          }
          this.selectFile = files[0].originFile;
          this.valid$.next(true);
          return validFiles;
        }
      }
    ];
  }

  importRevocation(cb?: () => void) {
    this.warningMessageService.create({
      content: this.i18n.get('system_import_revocation_warn_label', [
        this.currentComponent.name
      ]),
      onOK: () => {
        this.certApiService
          .importCertificateCtlListUsingPOST({
            componentId: this.currentComponent.componentId,
            crl: this.selectFile,
            sync: true
          })
          .subscribe(res => cb());
      }
    });
  }

  ngOnInit() {
    this.initForm();
  }
}
