import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { ClusterDetailComponent } from './cluster-detail.component';
import { BaseModule } from 'app/shared';
import { BackupSetModule } from '../../backup-set/backup-set.module';
import { CustomModalOperateModule } from 'app/shared/components';
import { ProTableModule } from 'app/shared/components/pro-table/pro-table.module';

@NgModule({
  declarations: [ClusterDetailComponent],
  imports: [
    CommonModule,
    BaseModule,
    BackupSetModule,
    ProTableModule,
    CustomModalOperateModule
  ]
})
export class ClusterDetailModule {}
