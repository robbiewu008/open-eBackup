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
import { I18NService } from 'app/shared';
import { Subject } from 'rxjs';

@Component({
  selector: 'aui-feedback-warning',
  templateUrl: './feedback-warning.component.html',
  styleUrls: ['./feedback-warning.component.less']
})
export class FeedbackWarningComponent implements OnInit {
  status;
  isSecuritySnap = false;
  feedbackTplLabel = '';

  isFeedbackChecked$ = new Subject<boolean>();

  constructor(public i18n: I18NService) {}

  ngOnInit() {}

  warningConfirmChange(e) {
    this.isFeedbackChecked$.next(e);
  }
}
