import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { BaseModule } from 'app/shared';
import { CustomModalOperateModule } from 'app/shared/components';
import { ProTableModule } from 'app/shared/components/pro-table/pro-table.module';
import { SummaryComponent } from './summary.component';
@NgModule({
  declarations: [SummaryComponent],
  imports: [CommonModule, BaseModule, ProTableModule, CustomModalOperateModule],
  exports: [SummaryComponent]
})
export class SummaryModule {}
