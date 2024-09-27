import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { LiveMountsListModule } from '../live-mounts-list/live-mounts-list.module';
import { BaseModule } from 'app/shared';
import { ExchangeComponent } from './exchange.component';

import { ExchangeRoutingModule } from './exchange-routing.module';

@NgModule({
  declarations: [ExchangeComponent],
  imports: [
    CommonModule,
    BaseModule,
    ExchangeRoutingModule,
    LiveMountsListModule
  ],
  exports: [ExchangeComponent]
})
export class ExchangeModule {}
