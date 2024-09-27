import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { ExchangeCopyDataComponent } from './exchange-copy-data.component';
import { BaseModule } from 'app/shared';
import { AdvancedExchangeModule } from '../advanced-exchange/advanced-exchange.module';

@NgModule({
  declarations: [ExchangeCopyDataComponent],
  imports: [CommonModule, BaseModule, AdvancedExchangeModule],
  exports: [ExchangeCopyDataComponent]
})
export class ExchangeCopyDataModule {}
