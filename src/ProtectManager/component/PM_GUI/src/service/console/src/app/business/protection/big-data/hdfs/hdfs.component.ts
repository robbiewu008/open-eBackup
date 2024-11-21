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
import { Component, OnInit, ViewChild } from '@angular/core';
import { ClustersComponent } from './clusters/clusters.component';
import { FilesetsComponent } from './filesets/filesets.component';

@Component({
  selector: 'aui-hdfs',
  templateUrl: './hdfs.component.html',
  styleUrls: ['./hdfs.component.less']
})
export class HdfsComponent implements OnInit {
  activeIndex;

  @ViewChild(FilesetsComponent, { static: false })
  filesetsComponent: FilesetsComponent;

  @ViewChild(ClustersComponent, { static: false })
  clustersComponent: ClustersComponent;

  constructor() {}

  ngOnInit() {}

  onChange() {
    if (this.activeIndex === 0) {
      this.clustersComponent.ngOnInit();
    } else {
      this.filesetsComponent.ngOnInit();
    }
  }
}
