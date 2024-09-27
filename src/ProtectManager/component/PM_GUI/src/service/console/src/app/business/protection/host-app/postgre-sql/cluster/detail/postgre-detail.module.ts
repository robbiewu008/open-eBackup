import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { PostgreInstanceDatabaseListModule } from './basic-info/instance-database-list/postgre-instance-database-list.module';
import { PostgreBasicInfoModule } from './basic-info/postgre-basic-info.module';
import { PostgreDetailComponent } from './postgre-detail.component';

@NgModule({
  declarations: [PostgreDetailComponent],
  imports: [
    CommonModule,
    BaseModule,
    PostgreBasicInfoModule,
    PostgreInstanceDatabaseListModule
  ]
})
export class PostgreDetailModule {}
