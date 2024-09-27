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
import { Component, forwardRef, Input } from '@angular/core';
import { ControlValueAccessor, NG_VALUE_ACCESSOR } from '@angular/forms';
import { DataMap } from 'app/shared/consts';
import { USER_GUIDE_CACHE_DATA } from 'app/shared/consts/guide-config';
import { I18NService } from 'app/shared/services';
import { includes } from 'lodash';

const CUSTOM_NG_VALUE_ACCESSOR = {
  provide: NG_VALUE_ACCESSOR,
  useExisting: forwardRef(() => AgentSelectComponent),
  multi: true
};

@Component({
  selector: 'aui-agent-select',
  templateUrl: './agent-select.component.html',
  styleUrls: ['./agent-select.component.less'],
  providers: [CUSTOM_NG_VALUE_ACCESSOR]
})
export class AgentSelectComponent implements ControlValueAccessor {
  @Input('lvOptions') hostOptions = [];
  @Input('lvValueKey') lvValueKey = 'uuid';
  @Input('lvMode') mode = 'single';
  @Input('lvDisabled') disabled = false;
  @Input('lvPlaceholder') placeholder = this.i18n.get('common_select_label');
  @Input('isAgent') isAgent = true;
  @Input('isSimple') isSimple = false; // 不显示外置和图标的，右侧显示在线离线的简单版本
  @Input('lvShowClear') lvShowClear = false;
  dataMap = DataMap;
  value;

  hostBuiltinLabel = this.i18n.get('protection_hcs_host_builtin_label');
  hostExternalLabel = this.i18n.get('protection_hcs_host_external_label');
  newResourceLabel = this.i18n.get('protection_guide_new_resource_label');

  onChange: (value: string) => void = () => null;
  onTouched: () => void = () => null;

  constructor(private i18n: I18NService) {}

  isOnline(item): boolean {
    return this.isAgent
      ? item.linkStatus === DataMap.resource_LinkStatus_Special.normal.value
      : item.environment?.linkStatus ===
          DataMap.resource_LinkStatus_Special.normal.value;
  }

  isHostBuiltIn(item): boolean {
    return this.isAgent
      ? item.extendInfo?.scenario === DataMap.proxyHostType.builtin.value
      : item.environment?.extendInfo?.scenario ===
          DataMap.proxyHostType.builtin.value;
  }

  showGuideNew(item): boolean {
    return (
      USER_GUIDE_CACHE_DATA.active &&
      includes(USER_GUIDE_CACHE_DATA.host, item.endpoint)
    );
  }

  writeValue(value): void {
    this.value = value;
  }

  registerOnChange(fn: any): void {
    this.onChange = fn;
  }

  registerOnTouched(fn: any): void {
    this.onTouched = fn;
  }

  setDisabledState?(isDisabled: boolean): void {
    this.disabled = isDisabled;
  }

  selectChange(value) {
    this.value = value;
    this.onChange(this.value);
  }
}
