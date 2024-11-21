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
import { MessageService, ModalRef, UploadFile } from '@iux/live';
import { DataMap, I18NService, LicenseApiService } from 'app/shared';
import { BehaviorSubject, Observable, Observer } from 'rxjs';

@Component({
  selector: 'aui-import-license',
  templateUrl: './import-license.component.html',
  styleUrls: ['./import-license.component.less']
})
export class ImportLicenseComponent implements OnInit {
  cyberFilters = [];
  selectedCyberFile;
  selectedOpFile;
  isCyberEngine =
    this.i18n.get('deploy_type') === DataMap.Deploy_Type.cyberengine.value;

  constructor(
    private modal: ModalRef,
    private i18n: I18NService,
    private message: MessageService,
    private licenseApiService: LicenseApiService
  ) {}

  ngOnInit() {
    this.initFilters();
  }

  disableOkBtn() {
    this.modal.getInstance().lvOkDisabled = !this.selectedCyberFile;
  }

  initFilters() {
    this.cyberFilters = [
      {
        name: 'suffix',
        filterFn: (files: UploadFile[]) => {
          const supportSuffix = ['dat'];
          const validFiles = files.filter(file => {
            const suffix = file.name.split('.').pop();
            return supportSuffix.includes(suffix);
          });

          if (validFiles.length !== files.length) {
            this.message.error(
              this.i18n.get('common_format_error_label', ['dat']),
              {
                lvMessageKey: 'formatErrorKey1',
                lvShowCloseButton: true
              }
            );
            return '';
          }
          if (files[0].size > 2 * 1024 * 1024) {
            this.message.error(
              this.i18n.get('common_max_size_file_label', ['2MB']),
              {
                lvMessageKey: 'maxSizeFileErrorKey1',
                lvShowCloseButton: true
              }
            );
            return '';
          }
          return validFiles;
        }
      }
    ];
  }

  cyberFilesChange(e) {
    if (e.activeFiles && e.activeFiles.length) {
      this.selectedCyberFile = e.activeFiles[0].originFile;
    } else {
      this.selectedCyberFile = '';
    }
    this.disableOkBtn();
  }

  opFilesChange(e) {
    if (e.activeFiles && e.activeFiles.length) {
      this.selectedOpFile = e.activeFiles[0].originFile;
    } else {
      this.selectedOpFile = '';
    }
    this.disableOkBtn();
  }

  onOK(): Observable<any> {
    return new Observable<any>((observer: Observer<any>) => {
      this.licenseApiService
        .importLicenseUsingPOST({
          basicLicenseFile: this.selectedCyberFile
        })
        .subscribe(
          res => {
            observer.next({});
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
