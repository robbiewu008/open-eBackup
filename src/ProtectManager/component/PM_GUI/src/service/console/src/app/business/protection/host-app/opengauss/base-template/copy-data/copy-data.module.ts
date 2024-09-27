import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { CopyDataComponent } from './copy-data.component';
import { BaseModule } from 'app/shared';
import { CopyDataModule as CommonCopyData } from 'app/shared/components/copy-data/copy-data.module';
import { InstanceRestoreModule } from './instance-restore/instance-restore.module';
import { DatabaseRestoreModule } from './database-restore/database-restore.module';

@NgModule({
  declarations: [CopyDataComponent],
  imports: [
    CommonModule,
    BaseModule,
    CommonCopyData,
    InstanceRestoreModule,
    DatabaseRestoreModule
  ]
})
export class CopyDataModule {}
