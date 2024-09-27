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
  OnInit,
  Input,
  Output,
  EventEmitter,
  OnDestroy
} from '@angular/core';
import { Subscription, Subject, timer } from 'rxjs';
import {
  DataMap,
  SystemApiService,
  CommonConsts,
  I18NService,
  Modify_Network_View_Type
} from 'app/shared';
import { Router } from '@angular/router';
import { switchMap, takeUntil } from 'rxjs/operators';

@Component({
  selector: 'aui-modify-network-process',
  templateUrl: './modify-network-process.component.html',
  styleUrls: ['./modify-network-process.component.less']
})
export class ModifyNetworkProcessComponent implements OnInit, OnDestroy {
  backupTimeSub$: Subscription;
  archiveTimeSub$: Subscription;
  backupDestroy$ = new Subject();
  archiveDestroy$ = new Subject();
  backupResult = {};
  archiveResult = {};
  initStatus = DataMap.System_Init_Status;
  type = Modify_Network_View_Type;

  @Output() onResetBackupChange = new EventEmitter<any>();
  @Output() onResetArchiveChange = new EventEmitter<any>();
  @Input() viewType;

  constructor(
    private router: Router,
    private i18n: I18NService,
    private systemApiService: SystemApiService
  ) {}

  ngOnDestroy() {
    if (this.viewType === this.type.BackupView) {
      this.backupDestroy$.next(true);
      this.backupDestroy$.complete();
    } else {
      this.archiveDestroy$.next(true);
      this.archiveDestroy$.complete();
    }
  }

  ngOnInit() {}

  getModifyBackupStatus() {}

  getModifyArchiveStatus() {}

  onResetBackup() {
    this.onResetBackupChange.emit();
  }

  onResetArchive() {
    this.onResetArchiveChange.emit();
  }
}
