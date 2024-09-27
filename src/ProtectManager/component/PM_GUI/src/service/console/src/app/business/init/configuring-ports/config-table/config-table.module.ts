import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { RouteTableModule } from '../route-table/route-table.module';
import { ConfigTableComponent } from './config-table.component';

@NgModule({
  declarations: [ConfigTableComponent],
  imports: [CommonModule, BaseModule, RouteTableModule],
  exports: [ConfigTableComponent]
})
export class ConfigTableModule {}
