import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { CopyResourceListModule } from '../copy-resource-list/copy-resource-list.module';
import { MysqlRoutingModule } from './mysql-routing.module';
import { MysqlComponent } from './mysql.component';

@NgModule({
  declarations: [MysqlComponent],
  imports: [
    CommonModule,
    MysqlRoutingModule,
    BaseModule,
    CopyResourceListModule
  ]
})
export class MysqlModule {}
