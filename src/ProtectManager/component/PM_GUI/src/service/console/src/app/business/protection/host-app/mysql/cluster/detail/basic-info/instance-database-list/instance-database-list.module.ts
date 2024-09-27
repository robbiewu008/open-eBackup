import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { InstanceDatabaseListComponent } from './instance-database-list.component';
import { BaseModule } from 'app/shared';
import { ProStatusModule } from 'app/shared/components/pro-status';
import { ProTableModule } from 'app/shared/components/pro-table';

@NgModule({
  declarations: [InstanceDatabaseListComponent],
  imports: [CommonModule, BaseModule, ProTableModule, ProStatusModule],
  exports: [InstanceDatabaseListComponent]
})
export class InstanceDatabaseListModule {}
