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
import { NgModule } from '@angular/core';
import { RouterModule, Routes } from '@angular/router';
import { RedirectGuard } from 'app/shared/guards/redirect.guard';
import { ExploreComponent } from './explore.component';

const routes: Routes = [
  {
    path: '',
    component: ExploreComponent,
    children: [
      { path: '', redirectTo: 'copy-data/database', pathMatch: 'full' },
      {
        path: 'copy-data/database',
        loadChildren: () =>
          import('./copy-data/copy-database/copy-database.module').then(
            mod => mod.CopyDatabaseModule
          ),
        canActivateChild: [RedirectGuard]
      },
      {
        path: 'copy-data/big-data',
        loadChildren: () =>
          import('./copy-data/copy-big-data/copy-big-data.module').then(
            mod => mod.CopyBigDataModule
          ),
        canActivateChild: [RedirectGuard]
      },
      {
        path: 'copy-data/virtualization',
        loadChildren: () =>
          import(
            './copy-data/copy-virtualization/copy-virtualization.module'
          ).then(mod => mod.CopyVirtualizationModule),
        canActivateChild: [RedirectGuard]
      },
      {
        path: 'copy-data/container',
        loadChildren: () =>
          import('./copy-data/copy-container/copy-container.module').then(
            mod => mod.CopyContainerModule
          ),
        canActivateChild: [RedirectGuard]
      },
      {
        path: 'copy-data/cloud',
        loadChildren: () =>
          import('./copy-data/copy-cloud/copy-cloud.module').then(
            mod => mod.CopyCloudModule
          ),
        canActivateChild: [RedirectGuard]
      },
      {
        path: 'copy-data/private-cloud',
        loadChildren: () =>
          import(
            './copy-data/copy-private-cloud/copy-private-cloud.module'
          ).then(mod => mod.CopyPrivateCloudModule),
        canActivateChild: [RedirectGuard]
      },
      {
        path: 'copy-data/file-service',
        loadChildren: () =>
          import('./copy-data/copy-file-service/copy-file-service.module').then(
            mod => mod.CopyFileServiceModule
          ),
        canActivateChild: [RedirectGuard]
      },
      {
        path: 'copy-data/application',
        loadChildren: () =>
          import('./copy-data/copy-application/copy-application.module').then(
            mod => mod.CopyApplicationModule
          ),
        canActivateChild: [RedirectGuard]
      },
      {
        path: 'copy-data/bare-metal',
        loadChildren: () =>
          import('./copy-data/copy-bare-metal/copy-bare-metal.module').then(
            mod => mod.CopyBareMetalModule
          ),
        canActivateChild: [RedirectGuard]
      },
      {
        path: 'copy-data/local-file-system',
        loadChildren: () =>
          import('./copy-data/local-file-system/local-file-system.module').then(
            mod => mod.LocalFileSystemModule
          ),
        canActivateChild: [RedirectGuard]
      },
      {
        path: 'live-mounts/fileset',
        loadChildren: () =>
          import('./live-mounts/fileset/fileset.module').then(
            mod => mod.FilesetModule
          ),
        canActivateChild: [RedirectGuard]
      },
      {
        path: 'live-mounts/volume',
        loadChildren: () =>
          import('./live-mounts/volume/volume.module').then(
            mod => mod.VolumeModule
          ),
        canActivateChild: [RedirectGuard]
      },
      {
        path: 'live-mounts/oracle',
        loadChildren: () =>
          import('./live-mounts/oracle/oracle.module').then(
            mod => mod.OracleModule
          ),
        canActivateChild: [RedirectGuard]
      },
      {
        path: 'live-mounts/mysql',
        loadChildren: () =>
          import('./live-mounts/mysql/mysql.module').then(
            mod => mod.MysqlModule
          ),
        canActivateChild: [RedirectGuard]
      },
      {
        path: 'live-mounts/tdsql',
        loadChildren: () =>
          import('./live-mounts/tdsql/tdsql.module').then(
            mod => mod.TdsqlModule
          ),
        canActivateChild: [RedirectGuard]
      },
      {
        path: 'live-mounts/vmware',
        loadChildren: () =>
          import('./live-mounts/vmware/vmware.module').then(
            mod => mod.VmwareModule
          ),
        canActivateChild: [RedirectGuard]
      },
      {
        path: 'live-mounts/cnware',
        loadChildren: () =>
          import('./live-mounts/cnware/cnware.module').then(
            mod => mod.CnwareModule
          ),
        canActivateChild: [RedirectGuard]
      },
      {
        path: 'live-mounts/dorado-file-system',
        loadChildren: () =>
          import(
            './live-mounts/dorado-file-system/dorado-file-system.module'
          ).then(mod => mod.DoradoFileSystemModule),
        canActivateChild: [RedirectGuard]
      },
      {
        path: 'live-mounts/nas-shared',
        loadChildren: () =>
          import('./live-mounts/nas-shared/nas-shared.module').then(
            mod => mod.NasSharedModule
          ),
        canActivateChild: [RedirectGuard]
      },
      {
        path: 'live-mounts/application',
        loadChildren: () =>
          import('./live-mounts/application/application.module').then(
            mod => mod.ApplicationModule
          ),
        canActivateChild: [RedirectGuard]
      },
      {
        path: 'recovery-drill',
        loadChildren: () =>
          import('./recovery-drill/recovery-drill.module').then(
            mod => mod.RecoveryDrillModule
          ),
        canActivateChild: [RedirectGuard]
      },
      {
        path: 'create-drill',
        loadChildren: () =>
          import('./recovery-drill/create-drill/create-drill.module').then(
            mod => mod.CreateDrillModule
          ),
        canActivateChild: [RedirectGuard]
      },
      {
        path: 'modify-drill/:uuid',
        loadChildren: () =>
          import('./recovery-drill/create-drill/create-drill.module').then(
            mod => mod.CreateDrillModule
          ),
        canActivateChild: [RedirectGuard]
      },
      {
        path: 'drill-detail/:uuid',
        loadChildren: () =>
          import('./recovery-drill/drill-detail/drill-detail.module').then(
            mod => mod.DrillDetailModule
          ),
        canActivateChild: [RedirectGuard]
      },
      {
        path: 'drill-execute-log/:uuid',
        loadChildren: () =>
          import(
            './recovery-drill/drill-detail/drill-execute-log/drill-execute-log.module'
          ).then(mod => mod.DrillExecuteLogModule),
        canActivateChild: [RedirectGuard]
      },
      {
        path: 'airgap',
        loadChildren: () =>
          import('./airgap/airgap.module').then(mod => mod.AirgapModule),
        canActivateChild: [RedirectGuard]
      },
      {
        path: 'data-desensitization/oracle',
        loadChildren: () =>
          import('./data-desensitization/oracle/oracle.module').then(
            mod => mod.OracleModule
          ),
        canActivateChild: [RedirectGuard]
      },
      {
        path: 'policy/mount-update-policy',
        loadChildren: () =>
          import(
            './policy/mount-update-policy/mount-update-policy.module'
          ).then(mod => mod.MountUpdatePolicyModule),
        canActivateChild: [RedirectGuard]
      },
      {
        path: 'policy/desensitization-policy',
        loadChildren: () =>
          import(
            './policy/desensitization-policy/desensitization-policy.module'
          ).then(mod => mod.DesensitizationPolicyModule),
        canActivateChild: [RedirectGuard]
      },
      {
        path: 'policy/anti-policy-setting',
        loadChildren: () =>
          import(
            './policy/anti-policy-setting/anti-policy-setting.module'
          ).then(mod => mod.AntiPolicySettingModule),
        canActivateChild: [RedirectGuard]
      },
      {
        path: 'anti-ransomware/overview',
        loadChildren: () =>
          import('./anti-ransomware/overview/overview.module').then(
            mod => mod.OverviewModule
          ),
        canActivateChild: [RedirectGuard]
      },
      {
        path: 'anti-ransomware/cloud-backup-overview',
        loadChildren: () =>
          import(
            './anti-ransomware/cloud-backup-overview/cloud-backup-overview.module'
          ).then(mod => mod.CloudBackupOverviewModule),
        canActivateChild: [RedirectGuard]
      },
      // 勒索检测设置
      {
        path: 'anti-ransomware/detection-setting',
        loadChildren: () =>
          import(
            './anti-ransomware/detection-setting/detection-setting.module'
          ).then(mod => mod.DetectionSettingModule),
        canActivateChild: [RedirectGuard]
      },
      // 文件拦截
      {
        path: 'anti-ransomware/blocking-rule-list',
        loadChildren: () =>
          import(
            './anti-ransomware/blocking-rule-list/blocking-rule-list.module'
          ).then(mod => mod.BlockingRuleListModule),
        canActivateChild: [RedirectGuard]
      },
      {
        path: 'anti-ransomware/real-time-detect',
        loadChildren: () =>
          import(
            './anti-ransomware/real-time-detect/real-time-detect.module'
          ).then(mod => mod.RealTimeDetectModule),
        canActivateChild: [RedirectGuard]
      },
      // 检测模型
      {
        path: 'anti-ransomware/detection-model-list',
        loadChildren: () =>
          import(
            './anti-ransomware/detection-model-list/detection-model-list.module'
          ).then(mod => mod.DetectionModelListModule),
        canActivateChild: [RedirectGuard]
      },
      {
        path: 'local-file-system',
        loadChildren: () =>
          import(
            './anti-ransomware/local-file-system/local-file-system.module'
          ).then(mod => mod.LocalFileSystemModule),
        canActivateChild: [RedirectGuard]
      },
      {
        path: 'storage-device',
        loadChildren: () =>
          import('./storage-device/storage-device.module').then(
            mod => mod.StorageDeviceModule
          ),
        canActivateChild: [RedirectGuard]
      },
      {
        path: 'ransomware-protection/file-interception',
        loadChildren: () =>
          import(
            './ransomware-protection/file-interception/file-interception.module'
          ).then(mod => mod.FileInterceptionModule),
        canActivateChild: [RedirectGuard]
      },
      {
        path: 'ransomware-protection/real-time-detection',
        loadChildren: () =>
          import(
            './ransomware-protection/real-time-detection/real-time-detection.module'
          ).then(mod => mod.RealTimeDetectionModule),
        canActivateChild: [RedirectGuard]
      },
      {
        path: 'ransomware-protection/data-backup',
        loadChildren: () =>
          import('./ransomware-protection/data-backup/data-backup.module').then(
            mod => mod.DataBackupModule
          ),
        canActivateChild: [RedirectGuard]
      },
      {
        path: 'ransomware-protection/detection-model',
        loadChildren: () =>
          import(
            './ransomware-protection/detection-model/detection-model.module'
          ).then(mod => mod.DetectionModelModule),
        canActivateChild: [RedirectGuard]
      },
      {
        path: 'detection-report',
        loadChildren: () =>
          import('./detection-report/detection-report.module').then(
            mod => mod.DetectionReportModule
          ),
        canActivateChild: [RedirectGuard]
      },
      {
        path: 'snapshot-data',
        loadChildren: () =>
          import('./snapshot-data/snapshot-data.module').then(
            mod => mod.SnapshotDataModule
          ),
        canActivateChild: [RedirectGuard]
      }
    ]
  }
];

@NgModule({
  imports: [RouterModule.forChild(routes)],
  exports: [RouterModule]
})
export class ExploreRoutingModule {}
