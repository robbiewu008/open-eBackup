import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { AssociatedSnapshotComponent } from './associated-snapshot.component';
import { BaseModule } from 'app/shared';
import { ProTableModule } from 'app/shared/components/pro-table';

@NgModule({
  declarations: [AssociatedSnapshotComponent],
  imports: [CommonModule, BaseModule, ProTableModule],
  exports: [AssociatedSnapshotComponent]
})
export class AssociatedSnapshotModule {}
