import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { CopyResourceListModule } from '../copy-resource-list/copy-resource-list.module';
import { TdsqlRoutingModule } from './tdsql-routing.module';
import { TdsqlComponent } from './tdsql.component';

@NgModule({
  declarations: [TdsqlComponent],
  imports: [
    CommonModule,
    TdsqlRoutingModule,
    BaseModule,
    CopyResourceListModule
  ]
})
export class TdsqlModule {}
