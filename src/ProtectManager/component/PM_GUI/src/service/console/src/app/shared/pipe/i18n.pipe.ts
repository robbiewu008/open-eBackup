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
import { Pipe, PipeTransform } from '@angular/core';
import { I18NService } from '../services/i18n.service';

@Pipe({ name: 'i18n', pure: false })
export class I18NPipe implements PipeTransform {
  constructor(private I18N: I18NService) {}

  transform(value: any, args: any[] = null, colon: boolean = false): any {
    return this.I18N.get(value, args, colon);
  }
}
