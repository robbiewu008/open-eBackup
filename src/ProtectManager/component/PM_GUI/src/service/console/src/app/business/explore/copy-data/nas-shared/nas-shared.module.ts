import { BaseModule } from 'app/shared';
import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';

import { NasSharedRoutingModule } from './nas-shared-routing.module';
import { NasSharedComponent } from './nas-shared.component';
import { CopyResourceListModule } from '../copy-resource-list/copy-resource-list.module';

@NgModule({
  declarations: [NasSharedComponent],
  imports: [
    CommonModule,
    NasSharedRoutingModule,
    BaseModule,
    CopyResourceListModule
  ]
})
export class NasSharedModule {}
