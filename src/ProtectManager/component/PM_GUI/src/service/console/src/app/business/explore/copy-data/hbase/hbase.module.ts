import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { CopyResourceListModule } from '../copy-resource-list/copy-resource-list.module';
import { HbaseRoutingModule } from './hbase-routing.module';
import { HbaseComponent } from './hbase.component';

@NgModule({
  declarations: [HbaseComponent],
  imports: [
    CommonModule,
    HbaseRoutingModule,
    BaseModule,
    CopyResourceListModule
  ]
})
export class HbaseModule {}
