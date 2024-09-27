import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { CopyResourceListModule } from '../copy-resource-list/copy-resource-list.module';
import { ClickHouseRoutingModule } from './click-house-routing.module';
import { ClickHouseComponent } from './click-house.component';

@NgModule({
  declarations: [ClickHouseComponent],
  imports: [
    CommonModule,
    ClickHouseRoutingModule,
    BaseModule,
    CopyResourceListModule
  ]
})
export class ClickHouseModule {}
