import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { CopyDataComponent } from './copy-data.component';
import { BaseModule } from 'app/shared';
import { CopyDataModule as CommonCopyData } from 'app/shared/components/copy-data/copy-data.module';
import { DamengRestoreModule } from './dameng-restore/dameng-restore.module';
import { DamengClusterRestoreModule } from './dameng-cluster-restore/dameng-cluster-restore.module';

@NgModule({
  declarations: [CopyDataComponent],
  imports: [
    CommonModule,
    BaseModule,
    CommonCopyData,
    DamengRestoreModule,
    DamengClusterRestoreModule
  ]
})
export class CopyDataModule {}
