import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { BaseModule } from 'app/shared/base.module';
import { ProtectFilterComponent } from './protect-filter.component';

@NgModule({
  declarations: [ProtectFilterComponent],
  imports: [CommonModule, BaseModule],
  exports: [ProtectFilterComponent]
})
export class ProtectFilterModule {}
