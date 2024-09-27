import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { CopyResourceListModule } from '../copy-resource-list/copy-resource-list.module';
import { ObjectRoutingModule } from './object-routing.module';
import { ObjectComponent } from './object.component';

@NgModule({
  declarations: [ObjectComponent],
  imports: [
    CommonModule,
    BaseModule,
    ObjectRoutingModule,
    CopyResourceListModule
  ]
})
export class ObjectModule {}
