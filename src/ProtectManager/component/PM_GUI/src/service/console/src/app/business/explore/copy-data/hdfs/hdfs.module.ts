import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { CopyResourceListModule } from '../copy-resource-list/copy-resource-list.module';
import { HdfsRoutingModule } from './hdfs-routing.module';
import { HdfsComponent } from './hdfs.component';

@NgModule({
  declarations: [HdfsComponent],
  imports: [CommonModule, HdfsRoutingModule, BaseModule, CopyResourceListModule]
})
export class HdfsModule {}
