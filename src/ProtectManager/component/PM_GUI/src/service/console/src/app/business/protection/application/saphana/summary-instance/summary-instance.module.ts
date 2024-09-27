import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { SummaryInstanceComponent } from './summary-instance.component';
import { BaseModule } from 'app/shared';
import { CustomModalOperateModule } from 'app/shared/components';
import { ProTableModule } from 'app/shared/components/pro-table/pro-table.module';

@NgModule({
  declarations: [SummaryInstanceComponent],
  imports: [CommonModule, BaseModule, ProTableModule, CustomModalOperateModule],
  exports: [SummaryInstanceComponent]
})
export class SummaryInstanceModule {}
