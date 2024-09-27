import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { CopyResourceListModule } from '../copy-resource-list/copy-resource-list.module';
import { GaussDBDWSRoutingModule } from './gaussdb-dws-routing.module';
import { GaussDBDWSComponent } from './gaussdb-dws.component';

@NgModule({
  declarations: [GaussDBDWSComponent],
  imports: [
    CommonModule,
    GaussDBDWSRoutingModule,
    BaseModule,
    CopyResourceListModule
  ]
})
export class GaussDBDWSModule {}
