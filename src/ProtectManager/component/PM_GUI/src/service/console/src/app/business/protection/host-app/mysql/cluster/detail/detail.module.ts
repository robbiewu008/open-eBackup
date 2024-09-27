import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { BasicInfoModule } from './basic-info/basic-info.module';
import { InstanceDatabaseListModule } from './basic-info/instance-database-list/instance-database-list.module';
import { DetailComponent } from './detail.component';

@NgModule({
  declarations: [DetailComponent],
  imports: [
    CommonModule,
    BaseModule,
    BasicInfoModule,
    InstanceDatabaseListModule
  ]
})
export class DetailModule {}
