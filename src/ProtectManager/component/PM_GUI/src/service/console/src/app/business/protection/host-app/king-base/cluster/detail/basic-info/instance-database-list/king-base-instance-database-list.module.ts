import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { KingBaseInstanceDatabaseListComponent } from './king-base-instance-database-list.component';
import { BaseModule } from 'app/shared';
import { ProStatusModule } from 'app/shared/components/pro-status';
import { ProTableModule } from 'app/shared/components/pro-table';

@NgModule({
  declarations: [KingBaseInstanceDatabaseListComponent],
  imports: [CommonModule, BaseModule, ProTableModule, ProStatusModule],
  exports: [KingBaseInstanceDatabaseListComponent]
})
export class PostgreInstanceDatabaseListModule {}
