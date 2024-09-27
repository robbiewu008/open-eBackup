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
  forwardRef,
  Input,
  OnInit,
  Output
} from '@angular/core';
import { ControlValueAccessor, NG_VALUE_ACCESSOR } from '@angular/forms';

const CUSTOM_NG_VALUE_ACCESSOR = {
  provide: NG_VALUE_ACCESSOR,
  useExisting: forwardRef(() => InuptWithEyeComponent),
  multi: true
};

@Component({
  selector: 'aui-inupt-with-eye',
  templateUrl: './inupt-with-eye.component.html',
  styleUrls: ['./inupt-with-eye.component.less'],
  providers: [CUSTOM_NG_VALUE_ACCESSOR]
})
export class InuptWithEyeComponent implements OnInit, ControlValueAccessor {
  @Input('lvDisabled') disabled = false;
  @Input('lvPasteAllowed') pasteAllowed = true;
  @Input('placeholder') placeholder = '';
  @Input('lv-tooltip') lvTooltip = '';
  @Input('lvTooltipTrigger') lvTooltipTrigger = '';
  @Input('lvTooltipPosition') lvTooltipPosition = '';
  @Input('lvTooltipTheme') lvTooltipTheme = '';
  @Input('isLanfree') isLanfree = false;

  @Output() focus = new EventEmitter();
  @Output() blur = new EventEmitter();

  value;
  coverPwd = true;
  cachePasteAllowed = true;

  onChange: (value: string) => void = () => null;
  onTouched: () => void = () => null;

  constructor() {}

  ngOnInit() {
    this.cachePasteAllowed = this.pasteAllowed;
  }

  writeValue(value: any) {
    this.value = value;
  }

  registerOnChange(fn: any) {
    this.onChange = fn;
  }

  registerOnTouched(fn: any) {
    this.onTouched = fn;
  }

  setDisabledState(isDisabled: boolean) {
    this.disabled = isDisabled;
  }

  updateValue(event: KeyboardEvent) {
    const cacheValue = (event.target as HTMLInputElement).value;
    this.value = cacheValue;
    this.onChange(this.value);
  }

  cover() {
    if (this.disabled) {
      return;
    }
    this.coverPwd = !this.coverPwd;
    if (!this.coverPwd) {
      this.pasteAllowed = true;
    } else {
      this.pasteAllowed = this.cachePasteAllowed;
    }
  }

  onFocus() {
    this.focus.emit();
  }

  onBlur() {
    this.onTouched();
    this.blur.emit();
  }

  copy() {
    return false;
  }
}
