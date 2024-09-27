import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { IoSnapshotReportComponent } from './io-snapshot-report.component';
import { BaseModule } from 'app/shared';
import { ProTableModule } from 'app/shared/components/pro-table';

@NgModule({
  declarations: [IoSnapshotReportComponent],
  imports: [CommonModule, BaseModule, ProTableModule]
})
export class IoSnapshotReportModule {}
