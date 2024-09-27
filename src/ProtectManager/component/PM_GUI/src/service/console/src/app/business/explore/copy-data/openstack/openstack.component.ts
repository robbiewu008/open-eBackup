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
import { DataMap, I18NService, ResourceType } from 'app/shared';

@Component({
  selector: 'aui-openstack',
  templateUrl: './openstack.component.html',
  styleUrls: ['./openstack.component.less']
})
export class OpenstackComponent implements OnInit {
  header = this.i18n.get('common_open_stack_label');
  resourceType = ResourceType.OpenStack;
  childResourceType = [DataMap.Resource_Type.openStackCloudServer.value];

  constructor(private i18n: I18NService) {}

  ngOnInit(): void {}
}
