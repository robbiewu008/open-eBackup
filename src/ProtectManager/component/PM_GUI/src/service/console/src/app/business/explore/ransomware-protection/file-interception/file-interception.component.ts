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
import {
  CommonConsts,
  DataMap,
  FsFileExtensionFilterManagementService
} from 'app/shared';
import { assign } from 'lodash';

@Component({
  selector: 'aui-file-interception',
  templateUrl: './file-interception.component.html',
  styleUrls: ['./file-interception.component.less']
})
export class FileInterceptionComponent implements OnInit {
  activeIndex = 'fileSystem';
  totalFileSystem = 0;
  protectedFileSystem = 0;
  totalRule = 0;

  constructor(
    private fsFileExtensionFilterManagementService: FsFileExtensionFilterManagementService
  ) {}

  ngOnInit() {
    this.getFilesystem();
    this.getFilesystem(true);
    this.getFilterRule();
  }

  refreshFileSystem() {
    this.getFilesystem(false, false);
    this.getFilesystem(true, false);
  }

  refreshRule() {
    this.getFilterRule(false);
  }

  getFilesystem(protectedFlag = false, mask = true) {
    const params = {
      pageNum: CommonConsts.PAGE_START_EXTRA,
      pageSize: CommonConsts.PAGE_SIZE,
      akLoading: mask
    };
    if (protectedFlag) {
      assign(params, {
        configStatus: [DataMap.File_Extension_Status.enable.value]
      });
    }
    this.fsFileExtensionFilterManagementService
      .getFsFileBlockConfigUsingGET(params)
      .subscribe(res => {
        if (protectedFlag) {
          this.protectedFileSystem = res.totalCount;
        } else {
          this.totalFileSystem = res.totalCount;
        }
      });
  }

  getFilterRule(mask = true) {
    const params: any = {
      pageNum: CommonConsts.PAGE_START_EXTRA,
      pageSize: CommonConsts.PAGE_SIZE
    };
    this.fsFileExtensionFilterManagementService
      .getFsExtensionFilterUsingGET({ ...params, akLoading: mask })
      .subscribe(res => {
        this.totalRule = res.totalCount;
      });
  }
}
