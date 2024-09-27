import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { ClusterBackupsetDetailComponent } from './cluster-backupset-detail.component';
import { BaseModule } from 'app/shared';
import { BackupSetModule } from '../../backup-set/backup-set.module';
import { CustomModalOperateModule } from 'app/shared/components';

@NgModule({
  declarations: [ClusterBackupsetDetailComponent],
  imports: [CommonModule, BaseModule, BackupSetModule, CustomModalOperateModule]
})
export class ClusterBackupsetDetailModule {}
