import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { BaseModule } from 'app/shared';
import { SummaryClusterComponent } from './summary-cluster.component';
import { BaseInfoModule } from 'app/shared/components/base-info/base-info.module';
import { ProTableModule } from 'app/shared/components/pro-table';
import { ProStatusModule } from 'app/shared/components/pro-status';

@NgModule({
  declarations: [SummaryClusterComponent],
  imports: [
    CommonModule,
    BaseModule,
    BaseInfoModule,
    ProTableModule,
    ProStatusModule
  ]
})
export class SummaryClusterModule {}
