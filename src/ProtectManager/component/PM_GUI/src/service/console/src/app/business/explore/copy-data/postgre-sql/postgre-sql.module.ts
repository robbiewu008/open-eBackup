import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { CopyResourceListModule } from '../copy-resource-list/copy-resource-list.module';
import { PostgreSQLRoutingModule } from './postgre-sql-routing.module';
import { PostgreSQLComponent } from './postgre-sql.component';

@NgModule({
  declarations: [PostgreSQLComponent],
  imports: [
    CommonModule,
    PostgreSQLRoutingModule,
    BaseModule,
    CopyResourceListModule
  ]
})
export class PostgreSQLModule {}
