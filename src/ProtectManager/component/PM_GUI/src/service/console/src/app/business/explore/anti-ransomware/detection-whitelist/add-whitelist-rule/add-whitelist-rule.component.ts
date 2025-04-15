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
import { ChangeDetectorRef, Component, OnInit } from '@angular/core';
import {
  AbstractControl,
  FormBuilder,
  FormControl,
  FormGroup,
  ValidatorFn
} from '@angular/forms';
import {
  BaseUtilService,
  CommonConsts,
  DataMap,
  DataMapService,
  I18NService,
  WhiteListManagementService
} from 'app/shared';
import {
  assign,
  find,
  includes,
  isEmpty,
  isUndefined,
  size,
  trim,
  uniq
} from 'lodash';
import { Observable, Observer } from 'rxjs';

@Component({
  selector: 'aui-add-whitelist-rule',
  templateUrl: './add-whitelist-rule.component.html',
  styleUrls: ['./add-whitelist-rule.component.less']
})
export class AddWhitelistRuleComponent implements OnInit {
  dataMap = DataMap;
  formGroup: FormGroup;
  typeOptions = this.dataMapService
    .toArray('Detection_Whitelist_Type')
    .filter(item => {
      return (item.isLeaf = true);
    });

  extensionTipTpl = this.i18n.get('explore_whitelist_file_extension_tip_label');
  extensionErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [127]),
    invalidExtension: this.i18n.get(
      'explore_valid_whitelist_file_extension_label'
    )
  };
  pathErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    pathError: this.i18n.get('common_path_error_label'),
    samePathError: this.i18n.get('protection_same_path_error_label'),
    unsupportPathError: this.i18n.get(
      'protection_unsupport_fileset_linux_path_label'
    )
  };

  constructor(
    private fb: FormBuilder,
    private i18n: I18NService,
    private baseUtilService: BaseUtilService,
    private dataMapService: DataMapService,
    private whiteListManagementService: WhiteListManagementService
  ) {}

  ngOnInit(): void {
    this.initForm();
  }

  initForm() {
    this.formGroup = this.fb.group({
      type: new FormControl(DataMap.Detection_Whitelist_Type.dir.value),
      dirName: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.validPath(),
          this.validLinuxPath()
        ]
      }),
      fileName: new FormControl('')
    });
    this.watch();
  }
  watch() {
    this.formGroup.get('type').valueChanges.subscribe(res => {
      if (res === this.dataMap.Detection_Whitelist_Type.dir.value) {
        this.formGroup
          .get('dirName')
          .setValidators([
            this.baseUtilService.VALID.required(),
            this.validPath(),
            this.validLinuxPath()
          ]);
        this.formGroup.get('fileName').clearValidators();
      } else {
        this.formGroup
          .get('fileName')
          .setValidators([
            this.baseUtilService.VALID.required(),
            this.validExtension()
          ]);
        this.formGroup.get('dirName').clearValidators();
      }
      this.formGroup.get('dirName').updateValueAndValidity();
      this.formGroup.get('fileName').updateValueAndValidity();
    });
  }
  validPath(): ValidatorFn {
    return (control: AbstractControl): { [key: string]: any } | null => {
      if (
        !trim(control.value) ||
        size(trim(control.value)) !== size(control.value)
      ) {
        return { pathError: { value: control.value } };
      }
      const paths = control.value;

      if (
        !CommonConsts.REGEX.templatLinuxPath.test(paths) ||
        paths.length > 2048
      ) {
        return { pathError: { value: control.value } };
      }
      return null;
    };
  }

  validExtension(): ValidatorFn {
    return (control: AbstractControl): { [key: string]: any } | null => {
      if (isUndefined(this.formGroup)) {
        return null;
      }
      if (!control.value) {
        return { required: { value: control.value } };
      }

      const values = control.value.split(',');
      const regAll = /^[a-zA-Z0-9~!@%#$&\"\'\(\)\*\+\-\.\/\:\;\<\=\>\?\[\\\]\^\_\`\{\|\}\u0020]+$/;
      for (let i = 0; i < size(values); i++) {
        const value = values[i];
        if (!regAll.test(value)) {
          return { invalidExtension: { value: control.value } };
        }
      }
      return null;
    };
  }

  validLinuxPath(): ValidatorFn {
    return (control: AbstractControl): { [key: string]: any } | null => {
      const paths = control.value.split(',')?.filter(item => {
        return !isEmpty(item);
      });

      for (let path of paths) {
        if (includes(['proc', 'dev', 'run'], path.split('/')[1])) {
          return { unsupportPathError: { value: control.value } };
        }
      }

      return null;
    };
  }

  getPamars() {
    const params = { type: this.formGroup.value.type };
    if (
      this.formGroup.value.type ===
      this.dataMap.Detection_Whitelist_Type.dir.value
    ) {
      assign(params, {
        content: this.formGroup.value.dirName
      });
    } else {
      assign(params, {
        content: this.formGroup.value.fileName
      });
    }
    return params;
  }

  onOK(): Observable<void> {
    return new Observable<void>((observer: Observer<void>) => {
      const params = this.getPamars();
      this.whiteListManagementService
        .createWhiteListUsingPOST({
          createWhiteListRequest: params as any
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
    });
  }
}
