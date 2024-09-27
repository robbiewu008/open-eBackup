import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { ProTableModule } from 'app/shared/components/pro-table';
import { RouteTableComponent } from './route-table.component';

@NgModule({
  declarations: [RouteTableComponent],
  imports: [CommonModule, BaseModule, ProTableModule],
  exports: [RouteTableComponent]
})
export class RouteTableModule {}
