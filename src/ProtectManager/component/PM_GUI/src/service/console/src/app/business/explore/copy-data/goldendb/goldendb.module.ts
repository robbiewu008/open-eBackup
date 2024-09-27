import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { CopyResourceListModule } from '../copy-resource-list/copy-resource-list.module';
import { GoldendbRoutingModule } from './goldendb-routing.module';
import { GoldendbComponent } from './goldendb.component';

@NgModule({
  declarations: [GoldendbComponent],
  imports: [
    CommonModule,
    GoldendbRoutingModule,
    BaseModule,
    CopyResourceListModule
  ]
})
export class GoldendbModule {}
