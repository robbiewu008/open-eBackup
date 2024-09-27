import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { AdvancedExchangeComponent } from './advanced-exchange.component';
import { BaseModule } from 'app/shared';

@NgModule({
  declarations: [AdvancedExchangeComponent],
  imports: [CommonModule, BaseModule],
  exports: [AdvancedExchangeComponent]
})
export class AdvancedExchangeModule {}
