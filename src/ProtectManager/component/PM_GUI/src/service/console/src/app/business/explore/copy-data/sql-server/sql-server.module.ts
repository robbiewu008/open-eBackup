import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { CopyResourceListModule } from '../copy-resource-list/copy-resource-list.module';
import { SQLServerRoutingModule } from './sql-server-routing.module';
import { SQLServerComponent } from './sql-server.component';

@NgModule({
  declarations: [SQLServerComponent],
  imports: [
    CommonModule,
    SQLServerRoutingModule,
    BaseModule,
    CopyResourceListModule
  ]
})
export class SQLServerModule {}
