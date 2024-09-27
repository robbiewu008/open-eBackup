import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { TdsqlRoutingModule } from './tdsql-routing.module';
import { TdsqlComponent } from './tdsql.component';
import { LiveMountsListModule } from '../live-mounts-list/live-mounts-list.module';

@NgModule({
  declarations: [TdsqlComponent],
  imports: [CommonModule, TdsqlRoutingModule, BaseModule, LiveMountsListModule],
  exports: [TdsqlComponent]
})
export class TdsqlModule {}
