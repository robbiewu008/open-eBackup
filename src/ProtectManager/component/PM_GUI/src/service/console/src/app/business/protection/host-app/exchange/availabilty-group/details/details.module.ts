import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { ExchangeDetailsComponent } from './details.component';
import { BaseModule } from 'app/shared';
import { BaseInfoModule } from 'app/shared/components/base-info/base-info.module';
import { ProTableModule } from 'app/shared/components/pro-table';

@NgModule({
  declarations: [ExchangeDetailsComponent],
  imports: [CommonModule, BaseModule, BaseInfoModule, ProTableModule],
  exports: [ExchangeDetailsComponent]
})
export class ExchangeDetailsModule {}
