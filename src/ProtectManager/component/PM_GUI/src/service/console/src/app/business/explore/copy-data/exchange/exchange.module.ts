import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { ExchangeComponent } from './exchange.component';
import { ExchangeRoutingModule } from './exchange-routing.module';
import { BaseModule } from 'app/shared';
import { CopyResourceListModule } from '../copy-resource-list/copy-resource-list.module';

@NgModule({
  declarations: [ExchangeComponent],
  imports: [
    CommonModule,
    ExchangeRoutingModule,
    BaseModule,
    CopyResourceListModule
  ]
})
export class ExchangeModule {}
