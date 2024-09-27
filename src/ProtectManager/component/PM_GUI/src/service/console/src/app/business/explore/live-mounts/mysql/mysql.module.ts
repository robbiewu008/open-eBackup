import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { MysqlRoutingModule } from './mysql-routing.module';
import { MysqlComponent } from './mysql.component';
import { LiveMountsListModule } from '../live-mounts-list/live-mounts-list.module';

@NgModule({
  declarations: [MysqlComponent],
  imports: [CommonModule, MysqlRoutingModule, BaseModule, LiveMountsListModule]
})
export class MysqlModule {}
