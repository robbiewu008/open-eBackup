import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { ClusterBackupsetDetailComponent } from './cluster-backupset-detail.component';
import { BaseModule } from 'app/shared';
import { CustomModalOperateModule } from 'app/shared/components';
import { ProTableModule } from 'app/shared/components/pro-table';

@NgModule({
  declarations: [ClusterBackupsetDetailComponent],
  imports: [CommonModule, BaseModule, CustomModalOperateModule, ProTableModule]
})
export class ClusterBackupsetDetailModule {}
