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
import { TemplateRef } from '@angular/core';
import { Observable } from 'rxjs';

export interface ProButton {
  label: string | TemplateRef<any>;
  id?: string;
  icon?: string;
  type?: 'primary' | 'link' | 'default';
  size?: 'large' | 'default' | 'small' | 'auto';
  permission?: number | string;
  showLoading?: boolean;
  loadingText?: string;
  disabledTips?: string;
  popoverContent?: string;
  popoverShow?: boolean;
  divide?: boolean;
  items?: ProButton[];
  displayCheck?: (...args: any[]) => boolean;
  disableCheck?: (...args: any[]) => boolean;
  disabledTipsCheck?: (...args: any[]) => string;
  onClick?: (
    ...args: any[]
  ) =>
    | (boolean | void | {})
    | Promise<boolean | void | {}>
    | Observable<boolean | void | {}>;
}
