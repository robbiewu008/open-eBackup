import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { BaseInfoModule } from 'app/shared/components/base-info/base-info.module';
import { ProButtonModule } from 'app/shared/components/pro-button';
import { ProStatusModule } from 'app/shared/components/pro-status';
import { ProTableModule } from 'app/shared/components/pro-table';
import { SummaryClusterComponent } from './summary-cluster.component';

@NgModule({
  declarations: [SummaryClusterComponent],
  imports: [
    CommonModule,
    BaseModule,
    ProTableModule,
    ProButtonModule,
    BaseInfoModule,
    ProStatusModule
  ],
  exports: [SummaryClusterComponent]
})
export class SummaryClusterModule {}
