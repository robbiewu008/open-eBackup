import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { SummaryClusterComponent } from './summary-cluster.component';
import { BaseModule } from 'app/shared';
import { CustomModalOperateModule } from 'app/shared/components';
import { ProTableModule } from 'app/shared/components/pro-table/pro-table.module';

@NgModule({
  declarations: [SummaryClusterComponent],
  imports: [CommonModule, BaseModule, ProTableModule, CustomModalOperateModule]
})
export class SummaryClusterModule {}
