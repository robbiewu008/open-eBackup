import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { CopyResourceListModule } from '../copy-resource-list/copy-resource-list.module';
import { HiveRoutingModule } from './hive-routing.module';
import { HiveComponent } from './hive.component';

@NgModule({
  declarations: [HiveComponent],
  imports: [CommonModule, HiveRoutingModule, BaseModule, CopyResourceListModule]
})
export class HiveModule {}
