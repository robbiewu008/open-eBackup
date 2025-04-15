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
import { Subject } from 'rxjs';
import { I18NService } from '@iux/live';
import { find } from 'lodash';
import { DataMap } from 'app/shared';

@Component({
  selector: 'aui-destroy-live-mount',
  templateUrl: './destroy-live-mount.component.html',
  styleUrls: ['./destroy-live-mount.component.less']
})
export class DestroyLiveMountComponent implements OnInit {
  items;
  status;
  reserveCopy = true;
  forceDelete = false;
  forceDeleteShow = true;
  isChecked$ = new Subject<boolean>();

  constructor(private i18n: I18NService) {}

  ngOnInit() {
    this.forceDeleteShow =
      this.items &&
      !find(this.items, {
        status: DataMap.LiveMount_Status.available.value
      });
  }

  warningConfirmChange(e) {
    this.isChecked$.next(e);
  }
}
