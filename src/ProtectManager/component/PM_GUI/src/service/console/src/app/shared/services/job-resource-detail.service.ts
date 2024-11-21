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
import { CommonModule } from '@angular/common';
import { Injectable, NgModule } from '@angular/core';
import { SummaryComponent as FilesetSummaryComponent } from 'app/business/protection/host-app/fileset/summary/summary.component';
import { SummaryModule as FilesetSummaryModule } from 'app/business/protection/host-app/fileset/summary/summary.module';
import { CopyDataComponent as HostCopyDataComponent } from 'app/business/protection/host-app/host/copy-data/copy-data.component';
import { SummaryComponent as HostSummaryComponent } from 'app/business/protection/host-app/host/summary/summary.component';
import { CopyDataComponent as OracleCopyDataComponent } from 'app/business/protection/host-app/oracle/database-list/copy-data/copy-data.component';
import { CopyDataModule as OracleCopyDataModule } from 'app/business/protection/host-app/oracle/database-list/copy-data/copy-data.module';
import { SummaryComponent as OracleSummaryComponent } from 'app/business/protection/host-app/oracle/database-list/summary/summary.component';
import { SummaryModule as OracleSummaryModule } from 'app/business/protection/host-app/oracle/database-list/summary/summary.module';
import { CopyDataComponent as NasSharedCopyDataComponent } from 'app/business/protection/storage/nas-shared/copy-data/copy-data.component';
import { CopyDataModule as NasShareCopyDataModule } from 'app/business/protection/storage/nas-shared/copy-data/copy-data.module';
import { SummaryComponent as NasSharedSummaryComponent } from 'app/business/protection/storage/nas-shared/summary/summary.component';
import { SummaryModule as NasShareSummaryModule } from 'app/business/protection/storage/nas-shared/summary/summary.module';
import { CopyDataComponent as VmwareCopyDataComponent } from 'app/business/protection/virtualization/vmware/vm/copy-data/copy-data.component';
import { CopyDataModule as VmwareCopyDataModule } from 'app/business/protection/virtualization/vmware/vm/copy-data/copy-data.module';
import { SummaryComponent as VmwareSummaryComponent } from 'app/business/protection/virtualization/vmware/vm/summary/summary.component';
import { SummaryModule as VmwareSummaryModule } from 'app/business/protection/virtualization/vmware/vm/summary/summary.module';
import { DataMap, MODAL_COMMON } from 'app/shared/consts';
import { I18NService } from 'app/shared/services';
import { DrawModalService } from 'app/shared/services/draw-modal.service';
import { assign, cloneDeep } from 'lodash';
import { DetailModalComponent } from '../components/detail-modal/detail-modal.component';
import { DetailModalModule } from '../components/detail-modal/detail-modal.module';
import { CopyDataComponent as HdfsFilesetCopyDataComponent } from 'app/business/protection/big-data/hdfs/filesets/copy-data/copy-data.component';
import { CopyDataModule as HdfsFilesetCopyDataModule } from 'app/business/protection/big-data/hdfs/filesets/copy-data/copy-data.module';
import { SummaryComponent as BackupSetSummaryComponent } from 'app/business/protection/big-data/hbase/backup-set/summary/summary.component';
import { SummaryModule as BackupSetSummaryModule } from 'app/business/protection/big-data/hbase/backup-set/summary/summary.module';
import { CopyDataComponent as BackupSetCopyDataComponent } from 'app/business/protection/big-data/hbase/backup-set/copy-data/copy-data.component';
import { CopyDataModule as BackupSetCopyDataModule } from 'app/business/protection/big-data/hbase/backup-set/copy-data/copy-data.module';

const DETAIL_CONFIG = {
  [DataMap.Resource_Type.ABBackupClient.value]: {
    lvWidth: 1150,
    tabs: [
      {
        title: 'common_summary_label',
        activeId: 'summary',
        component: HostSummaryComponent
      },
      {
        title: 'common_copy_data_label',
        activeId: 'copydata',
        component: HostCopyDataComponent
      }
    ]
  },
  [DataMap.Resource_Type.oracle.value]: {
    lvWidth: 1150,
    tabs: [
      {
        title: 'common_summary_label',
        activeId: 'summary',
        component: OracleSummaryComponent
      },
      {
        title: 'common_copy_data_label',
        activeId: 'copydata',
        component: OracleCopyDataComponent
      }
    ]
  },
  [DataMap.Resource_Type.virtualMachine.value]: {
    lvWidth: 1150,
    tabs: [
      {
        title: 'common_summary_label',
        activeId: 'summary',
        component: VmwareSummaryComponent
      },
      {
        title: 'common_copy_data_label',
        activeId: 'copydata',
        component: VmwareCopyDataComponent
      }
    ]
  },
  [DataMap.Resource_Type.NASFileSystem.value]: {
    lvWidth: 1150,
    tabs: [
      {
        title: 'common_summary_label',
        activeId: 'summary',
        component: NasSharedSummaryComponent
      },
      {
        title: 'common_copy_data_label',
        activeId: 'copydata',
        component: NasSharedCopyDataComponent
      }
    ]
  },
  [DataMap.Resource_Type.LocalFileSystem.value]: {
    lvWidth: 1150,
    tabs: [
      {
        title: 'common_summary_label',
        activeId: 'summary',
        component: NasSharedSummaryComponent
      },
      {
        title: 'common_copy_data_label',
        activeId: 'copydata',
        component: NasSharedCopyDataComponent
      }
    ]
  },
  [DataMap.Resource_Type.NASShare.value]: {
    lvWidth: 1150,
    tabs: [
      {
        title: 'common_summary_label',
        activeId: 'summary',
        component: NasSharedSummaryComponent
      },
      {
        title: 'common_copy_data_label',
        activeId: 'copydata',
        component: NasSharedCopyDataComponent
      }
    ]
  },
  [DataMap.Resource_Type.HDFSFileset.value]: {
    lvWidth: 1150,
    tabs: [
      {
        title: 'common_summary_label',
        activeId: 'summary',
        component: FilesetSummaryComponent
      },
      {
        title: 'common_copy_data_label',
        activeId: 'copydata',
        component: HdfsFilesetCopyDataComponent
      }
    ]
  },
  [DataMap.Resource_Type.HBaseBackupSet.value]: {
    lvWidth: 1150,
    tabs: [
      {
        title: 'common_summary_label',
        activeId: 'summary',
        component: BackupSetSummaryComponent
      },
      {
        title: 'common_copy_data_label',
        activeId: 'copydata',
        component: BackupSetCopyDataComponent
      }
    ]
  }
};

@Injectable({ providedIn: 'root' })
export class JobResourceDetailService {
  constructor(
    private drawModalService: DrawModalService,
    private i18n: I18NService
  ) {}

  openDetailModal(item, option: { [key: string]: any } = {}) {
    const config = cloneDeep(DETAIL_CONFIG[item.sourceSubType]);
    const params = assign({}, MODAL_COMMON.generateDrawerOptions(), {
      lvModalKey: 'detail-modal',
      lvModality: false,
      lvWidth: config.lvWidth || MODAL_COMMON.xLargeWidth,
      lvHeader: option.lvHeader,
      lvContent: DetailModalComponent,
      lvFooter: [
        {
          label: this.i18n.get('common_close_label'),
          onClick: modal => modal.close()
        }
      ],
      lvComponentParams: {
        resourceConfigs: config.tabs,
        detailData: assign({}, option.data, {
          resourceType: item.sourceSubType
        }),
        activeId: option.activeId || config.tabs[0].activeId
      }
    });

    if (config) {
      this.drawModalService.openDetailModal(params);
    }
  }
}

const modules = [
  FilesetSummaryModule,
  OracleSummaryModule,
  OracleCopyDataModule,
  VmwareCopyDataModule,
  VmwareSummaryModule,
  NasShareSummaryModule,
  NasShareCopyDataModule,
  HdfsFilesetCopyDataModule,
  BackupSetSummaryModule,
  BackupSetCopyDataModule
];

@NgModule({
  imports: [CommonModule, DetailModalModule, ...modules],
  providers: [JobResourceDetailService]
})
export class JobResourceDetailModule {}
