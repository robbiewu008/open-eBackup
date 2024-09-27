import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { SummaryInstanceListComponent } from './summary-instance-list.component';
import { BaseModule } from 'app/shared';
import { CustomModalOperateModule } from 'app/shared/components';
import { ProTableModule } from 'app/shared/components/pro-table';

@NgModule({
  declarations: [SummaryInstanceListComponent],
  imports: [CommonModule, BaseModule, CustomModalOperateModule, ProTableModule]
})
export class SummaryInstanceListModule {}
