import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { PostgreInstanceDatabaseListComponent } from './postgre-instance-database-list.component';
import { BaseModule } from 'app/shared';
import { ProStatusModule } from 'app/shared/components/pro-status';
import { ProTableModule } from 'app/shared/components/pro-table';

@NgModule({
  declarations: [PostgreInstanceDatabaseListComponent],
  imports: [CommonModule, BaseModule, ProTableModule, ProStatusModule],
  exports: [PostgreInstanceDatabaseListComponent]
})
export class PostgreInstanceDatabaseListModule {}
