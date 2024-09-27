import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { PoReportDetailComponent } from './po-report-detail.component';
import { BaseModule } from 'app/shared';
import { ProTableModule } from 'app/shared/components/pro-table';
import { AssociatedSnapshotModule } from '../associated-snapshot/associated-snapshot.module';

@NgModule({
  declarations: [PoReportDetailComponent],
  imports: [CommonModule, BaseModule, ProTableModule, AssociatedSnapshotModule],
  exports: [PoReportDetailComponent]
})
export class PoReportDetailModule {}
