import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { SnapshotDetailComponent } from './snapshot-detail.component';
import { BaseModule } from 'app/shared';

@NgModule({
  declarations: [SnapshotDetailComponent],
  imports: [CommonModule, BaseModule],
  exports: [SnapshotDetailComponent]
})
export class SnapshotDetailModule {}
