import { LiveMountsListModule } from './../live-mounts-list/live-mounts-list.module';
import { BaseModule } from 'app/shared';
import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';

import { NasSharedRoutingModule } from './nas-shared-routing.module';
import { NasSharedComponent } from './nas-shared.component';

@NgModule({
  declarations: [NasSharedComponent],
  imports: [
    CommonModule,
    NasSharedRoutingModule,
    BaseModule,
    LiveMountsListModule
  ]
})
export class NasSharedModule {}
