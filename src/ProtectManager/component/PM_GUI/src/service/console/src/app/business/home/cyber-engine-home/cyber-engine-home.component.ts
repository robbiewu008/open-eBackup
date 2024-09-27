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
import { Component, OnInit, OnDestroy, ViewChild } from '@angular/core';
import { interval, Subscription } from 'rxjs';
import { BigScreenComponent } from './big-screen/big-screen.component';
import { ExceptionsFileComponent } from './exceptions-file/exceptions-file.component';
import { AlarmsCountComponent } from './alarms-count/alarms-count.component';
import { FileInterceptionComponent } from './file-interception/file-interception.component';
import { TaskCountComponent } from './task-count/task-count.component';
import { AirGapComponent } from './air-gap/air-gap.component';
import { LicenseCapacityComponent } from './license-capacity/license-capacity.component';

const ONE_SECOND = 1000 as const;

@Component({
  selector: 'aui-cyber-engine-home',
  templateUrl: './cyber-engine-home.component.html',
  styleUrls: ['./cyber-engine-home.component.less']
})
export class CyberEngineHomeComponent implements OnInit, OnDestroy {
  timer: Subscription;

  @ViewChild('airGap', { static: false })
  airGap: AirGapComponent;
  @ViewChild('alarmsCount', { static: false })
  alarmsCount: AlarmsCountComponent;
  @ViewChild('bigScreen', { static: false }) bigScreen: BigScreenComponent;
  @ViewChild('exceptionsFile', { static: false })
  exceptionsFile: ExceptionsFileComponent;
  @ViewChild('fileInterception', { static: false })
  fileInterception: FileInterceptionComponent;
  @ViewChild('licenseCapacity', { static: false })
  licenseCapacity: LicenseCapacityComponent;
  @ViewChild('taskCount', { static: false }) taskCount: TaskCountComponent;

  constructor() {}

  ngOnInit() {
    this.timer = interval(ONE_SECOND * 30).subscribe(() => this.refresh());
  }

  ngOnDestroy() {
    this.timer.unsubscribe();
  }

  refresh() {
    this.airGap.loadData(false);
    this.alarmsCount.loadData(false);
    this.bigScreen.loadNodeInfo(false);
    this.exceptionsFile.loadTabelData(false);
    this.fileInterception.loadData(false);
    this.licenseCapacity.loadData(false);
    this.taskCount.loadData(false);
  }
}
