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
  Input,
  OnInit,
  OnChanges,
  SimpleChanges
} from '@angular/core';
import { DataMap } from 'app/shared/consts';
import { DataMapService, I18NService } from 'app/shared/services';

@Component({
  selector: 'aui-file-indexed',
  template: `
    <i
      [lv-tooltip]="content"
      [lv-icon]="icon"
      [ngClass]="{
        'lv-m-rotate': indexed === dataMap.CopyData_fileIndex.indexing.value
      }"
    ></i>
  `,
  styles: [
    `
      i {
        width: 20px;
        height: 20px;
      }
    `
  ]
})
export class FileIndexedComponent implements OnInit, OnChanges {
  @Input() indexed;

  content;
  icon;
  dataMap = DataMap;

  constructor(
    private dataMapService: DataMapService,
    private i18n: I18NService
  ) {}

  ngOnChanges(changes: SimpleChanges) {
    if (changes.indexed) {
      this.init();
    }
  }

  ngOnInit() {
    this.init();
  }

  init() {
    const config = this.dataMapService.getValueConfig(
      'CopyData_fileIndex',
      this.indexed
    );
    this.icon = config ? config.icon : 'aui-file-unIndexed';
    this.content = this.i18n.get(config ? config.label : '--');
  }
}
