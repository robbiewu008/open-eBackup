import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { SummaryDatabaseListComponent } from './summary-database-list.component';
import { BaseModule } from 'app/shared';
import { CustomModalOperateModule } from 'app/shared/components';
import { ProTableModule } from 'app/shared/components/pro-table';

@NgModule({
  declarations: [SummaryDatabaseListComponent],
  imports: [CommonModule, BaseModule, CustomModalOperateModule, ProTableModule]
})
export class SummaryDatabaseListModule {}
