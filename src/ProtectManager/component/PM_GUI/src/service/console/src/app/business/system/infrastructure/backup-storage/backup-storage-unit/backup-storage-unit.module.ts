import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { ProButtonModule } from 'app/shared/components/pro-button';
import { ProTableModule } from 'app/shared/components/pro-table';
import { BackupStorageUnitDetailModule } from './backup-storage-unit-detail/backup-storage-unit-detail.module';
import { BackupStorageUnitComponent } from './backup-storage-unit.component';

@NgModule({
  declarations: [BackupStorageUnitComponent],
  imports: [
    CommonModule,
    BaseModule,
    ProTableModule,
    ProButtonModule,
    BackupStorageUnitDetailModule
  ],
  exports: [BackupStorageUnitComponent]
})
export class BackupStorageUnitModule {}
