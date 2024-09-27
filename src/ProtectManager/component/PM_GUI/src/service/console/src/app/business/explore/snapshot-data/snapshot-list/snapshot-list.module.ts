import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { BaseModule } from 'app/shared';
import { SnapshotListComponent } from './snapshot-list.component';
import { SnapshotRestoreModule } from './snapshot-restore/snapshot-restore.module';
import { CustomTableSearchModule } from 'app/shared/components/custom-table-search/custom-table-search.module';

@NgModule({
  declarations: [SnapshotListComponent],
  imports: [
    CommonModule,
    BaseModule,
    SnapshotRestoreModule,
    CustomTableSearchModule
  ],
  exports: [SnapshotListComponent]
})
export class SnapshotListModule {}
