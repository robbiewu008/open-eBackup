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
import { Component, Input, OnInit } from '@angular/core';
import { FormGroup } from '@angular/forms';
import { CommonConsts, CookieService, MultiCluster } from 'app/shared';

@Component({
  selector: 'aui-multi-duduplication-tip',
  templateUrl: './multi-duduplication-tip.component.html',
  styleUrls: ['./multi-duduplication-tip.component.less']
})
export class MultiDuduplicationTipComponent implements OnInit {
  multiCluster = MultiCluster;
  @Input() formGroup: FormGroup;
  isHcsUser = this.cookieService.get('userType') === CommonConsts.HCS_USER_TYPE;
  constructor(private cookieService: CookieService) {}

  ngOnInit(): void {}
}
