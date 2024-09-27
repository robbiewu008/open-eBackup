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
import { Component, Input } from '@angular/core';
import { FormBuilder, FormControl, FormGroup } from '@angular/forms';
import { BaseUtilService, I18NService, LabelApiService } from 'app/shared';
import { Observable, Observer } from 'rxjs';

@Component({
  selector: 'aui-create-tag',
  templateUrl: './create-tag.component.html',
  styleUrls: ['./create-tag.component.less']
})
export class CreateTagComponent {
  formGroup: FormGroup;
  @Input() data;

  constructor(
    private i18n: I18NService,
    public fb: FormBuilder,
    private labelApiService: LabelApiService,
    public baseUtilService: BaseUtilService
  ) {}

  ngOnInit(): void {
    this.initForm();
  }

  initForm() {
    this.formGroup = this.fb.group({
      tagName: new FormControl(this.data ? this.data.name : '', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.name(),
          this.baseUtilService.VALID.maxLength(64)
        ]
      })
    });
  }

  onOk(): Observable<void> {
    return new Observable<void>((observer: Observer<void>) => {
      if (this.data) {
        this.labelApiService
          .modifyLabelUsingPUT({
            CreateOrUpdateLabelRequest: {
              uuid: this.data.uuid,
              name: this.formGroup.get('tagName').value
            }
          })
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
      } else {
        this.labelApiService
          .createLabelUsingPOST({
            CreateOrUpdateLabelRequest: {
              name: this.formGroup.get('tagName').value
            }
          })
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
      }
    });
  }
}
