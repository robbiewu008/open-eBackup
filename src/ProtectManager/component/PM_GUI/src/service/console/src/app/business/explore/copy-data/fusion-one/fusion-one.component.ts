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
import { Component } from '@angular/core';
import { DataMap, I18NService, ResourceType } from 'app/shared';

@Component({
  selector: 'aui-fusion-one',
  templateUrl: './fusion-one.component.html',
  styleUrls: ['./fusion-one.component.less']
})
export class FusionOneComponent {
  header = this.i18n.get('protection_fusionone_label');
  resourceType = ResourceType.FUSION_ONE;
  childResourceType = [DataMap.Resource_Type.fusionOne.value];

  constructor(private i18n: I18NService) {}
}
