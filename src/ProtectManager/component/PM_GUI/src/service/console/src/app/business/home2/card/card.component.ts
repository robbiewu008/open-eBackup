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
import { Component, OnInit, Input, Output, EventEmitter } from '@angular/core';
import { Router } from '@angular/router';
import { I18NService } from 'app/shared';
import { ClustersApiService } from 'app/shared';

@Component({
  selector: 'card',
  templateUrl: './card.component.html',
  styleUrls: ['./card.component.less']
})
export class CardComponent implements OnInit {
  @Input() cardInfo: any = {};
  @Output() selectChange = new EventEmitter();
  selectInfoArr: any = [];
  selectedArr = new Array(3).fill(-1); //最多三项
  constructor(
    public router: Router,
    public i18n: I18NService,
    private clustersApiService: ClustersApiService
  ) {}

  change(value: string, key: string, name: string): void {
    this.selectChange.emit({ value, key, name });
  }
  ngOnInit(): void {}

  navigate(navigateInfo, navigateParams?) {
    let params = {};
    for (let key in navigateParams) {
      params[key] = this.cardInfo[key].toString();
    }
    this.router.navigate(navigateInfo, {
      queryParams: params
    });
  }
}
