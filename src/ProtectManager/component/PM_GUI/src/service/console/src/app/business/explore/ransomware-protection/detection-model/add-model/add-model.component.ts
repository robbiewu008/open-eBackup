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
import {
  I18NService,
  ModelManagementService,
  WarningMessageService
} from 'app/shared';
import { Observable, Observer } from 'rxjs';

@Component({
  selector: 'aui-add-model',
  templateUrl: './add-model.component.html',
  styleUrls: ['./add-model.component.less']
})
export class AddModelComponent implements OnInit {
  selectFile;
  filters = [];

  constructor(
    private modal: ModalRef,
    private i18n: I18NService,
    private message: MessageService,
    private warningMessageService: WarningMessageService,
    private modelManagementService: ModelManagementService
  ) {}

  ngOnInit(): void {
    this.initFilters();
  }

  initFilters() {
    this.filters = [
      {
        name: 'suffix',
        filterFn: (files: UploadFile[]) => {
          const supportSuffix = ['zip', 'tgz'];
          let validFiles = files.filter(file => {
            let suffix = file.name.split('.').pop();
            return supportSuffix.includes(suffix);
          });

          if (validFiles.length !== files.length) {
            this.message.error(
              this.i18n.get('common_format_error_label', ['zip/tgz']),
              {
                lvMessageKey: 'formatErrorKey',
                lvShowCloseButton: true
              }
            );
            this.modal.getInstance().lvOkDisabled = true;
            return;
          }
          if (files[0].size > 1024 * 1024 * 10) {
            this.message.error(
              this.i18n.get('common_max_size_file_label', ['10MB']),
              {
                lvMessageKey: 'maxSizeFileErrorKey',
                lvShowCloseButton: true
              }
            );
            this.modal.getInstance().lvOkDisabled = true;
            return;
          }
          this.modal.getInstance().lvOkDisabled = false;
          return validFiles;
        }
      }
    ];
  }

  uploadChange(e) {
    if (e.activeFiles && e.activeFiles.length) {
      this.selectFile = e.activeFiles[0].originFile;
      this.modal.getInstance().lvOkDisabled = false;
    } else {
      this.modal.getInstance().lvOkDisabled = true;
    }
  }

  onOK(): Observable<void> {
    return new Observable<void>((observer: Observer<void>) => {
      this.warningMessageService.create({
        content: this.i18n.get('explore_upload_cyber_anti_model_label'),
        onOK: () => {
          this.modelManagementService
            .addModelInfoUsingPOST({
              modelFile: this.selectFile
            })
            .subscribe(
              () => {
                observer.next();
                observer.complete();
              },
              err => {
                observer.error(err);
                observer.complete();
              }
            );
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
    });
  }
}
