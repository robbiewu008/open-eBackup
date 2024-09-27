import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { SnapshotRestoreComponent } from './snapshot-restore.component';
import { BaseModule } from 'app/shared';
import { ProTableModule } from 'app/shared/components/pro-table';

@NgModule({
  declarations: [SnapshotRestoreComponent],
  imports: [CommonModule, BaseModule, ProTableModule],
  exports: [SnapshotRestoreComponent]
})
export class SnapshotRestoreModule {}
