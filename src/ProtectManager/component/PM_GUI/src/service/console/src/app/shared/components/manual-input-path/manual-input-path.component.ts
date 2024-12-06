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
import {
  Component,
  EventEmitter,
  Input,
  OnInit,
  Output,
  ViewChild
} from '@angular/core';
import {
  AbstractControl,
  FormBuilder,
  FormControl,
  FormGroup,
  ValidatorFn
} from '@angular/forms';
import { CommonConsts } from 'app/shared/consts';
import { I18NService } from 'app/shared/services';
import { find, map, trim } from 'lodash';

@Component({
  selector: 'aui-manual-input-path',
  templateUrl: './manual-input-path.component.html',
  styleUrls: ['./manual-input-path.component.less']
})
export class ManualInputPathComponent implements OnInit {
  @Input() rowCopy;
  @Output() pathChange = new EventEmitter();

  formGroup: FormGroup;

  pathTag = [];
  @ViewChild('clearAllPathPopover', { static: false }) clearAllPathPopover;

  pathErrorTip = {
    invalidPath: this.i18n.get('common_incorrect_format_label')
  };

  constructor(private fb: FormBuilder, private i18n: I18NService) {}

  ngOnInit(): void {
    this.initForm();
  }

  initForm() {
    this.formGroup = this.fb.group({
      path: new FormControl('', {
        validators: [this.validPath()]
      })
    });
  }

  validPath(): ValidatorFn {
    return (control: AbstractControl): { [key: string]: any } | null => {
      if (!trim(control.value)) {
        return null;
      }
      if (
        !CommonConsts.REGEX.windowsPath.test(control.value) &&
        !CommonConsts.REGEX.linuxPath.test(control.value)
      ) {
        return { invalidPath: { value: control.value } };
      }
      return null;
    };
  }

  add() {
    const path = this.formGroup.value.path;
    if (find(this.pathTag, item => item.label === path) || !path) {
      return;
    }
    this.pathTag.push({
      label: path,
      removable: true
    });
    this.pathTag = [...this.pathTag];
    this.pathChange.emit(
      map(this.pathTag, item => this.windowsPathToLinuxPath(item.label))
    );
  }

  clearAll() {
    this.pathTag = [];
    this.clearAllPathPopover?.hide();
    this.pathChange.emit(
      map(this.pathTag, item => this.windowsPathToLinuxPath(item.label))
    );
  }

  removeTag() {
    this.pathChange.emit(
      map(this.pathTag, item => this.windowsPathToLinuxPath(item.label))
    );
  }

  cancel() {
    this.clearAllPathPopover?.hide();
  }

  // windows路径转为linux格式路径
  windowsPathToLinuxPath(path: string): string {
    let replacePath = path.replace(/([a-zA-Z]):/g, '/$1');
    replacePath = replacePath.replace(/\\/g, '/');
    return replacePath;
  }
}
