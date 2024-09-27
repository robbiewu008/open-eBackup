import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { CopyResourceListModule } from '../copy-resource-list/copy-resource-list.module';
import { GaussdbForOpengaussRoutingModule } from './gaussdb-for-opengauss-routing.module';
import { GaussdbForOpengaussComponent } from './gaussdb-for-opengauss.component';

@NgModule({
  declarations: [GaussdbForOpengaussComponent],
  imports: [
    CommonModule,
    GaussdbForOpengaussRoutingModule,
    BaseModule,
    CopyResourceListModule
  ]
})
export class GaussdbForOpengaussModule {}
