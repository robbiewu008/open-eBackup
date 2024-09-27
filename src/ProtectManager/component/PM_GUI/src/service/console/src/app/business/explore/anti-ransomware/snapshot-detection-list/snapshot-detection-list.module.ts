import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { SnapshotDetectionListComponent } from './snapshot-detection-list.component';
import { BaseModule } from 'app/shared';
import { ProButtonModule } from 'app/shared/components/pro-button';
import { ProTableModule } from 'app/shared/components/pro-table';

@NgModule({
  declarations: [SnapshotDetectionListComponent],
  imports: [CommonModule, BaseModule, ProTableModule, ProButtonModule],
  exports: [SnapshotDetectionListComponent]
})
export class SnapshotDetectionListModule {}
