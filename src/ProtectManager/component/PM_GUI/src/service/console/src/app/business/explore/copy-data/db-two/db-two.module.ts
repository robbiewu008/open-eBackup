import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { CopyResourceListModule } from '../copy-resource-list/copy-resource-list.module';
import { DbTwoRoutingModule } from './db-two-routing.module';
import { DbTwoComponent } from './db-two.component';

@NgModule({
  declarations: [DbTwoComponent],
  imports: [
    CommonModule,
    DbTwoRoutingModule,
    BaseModule,
    CopyResourceListModule
  ]
})
export class DbTwoModule {}
