import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { SummaryServiceComponent } from './summary-service.component';
import { BaseModule } from 'app/shared';
import { CustomModalOperateModule } from 'app/shared/components';
import { ProTableModule } from 'app/shared/components/pro-table/pro-table.module';

@NgModule({
  declarations: [SummaryServiceComponent],
  imports: [CommonModule, BaseModule, ProTableModule, CustomModalOperateModule],
  exports: [SummaryServiceComponent]
})
export class SummaryServiceModule {}
