import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { CopyResourceListModule } from '../copy-resource-list/copy-resource-list.module';
import { GaussdbTRoutingModule } from './gaussdb-t-routing.module';
import { GaussdbTComponent } from './gaussdb-t.component';

@NgModule({
  declarations: [GaussdbTComponent],
  imports: [
    CommonModule,
    GaussdbTRoutingModule,
    BaseModule,
    CopyResourceListModule
  ]
})
export class GaussdbTModule {}
