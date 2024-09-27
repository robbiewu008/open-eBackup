import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared/base.module';
import { SpecialBaseInfoComponent } from './special-base-info.component';

@NgModule({
  declarations: [SpecialBaseInfoComponent],
  imports: [CommonModule, BaseModule],
  exports: [SpecialBaseInfoComponent]
})
export class SpecialBaseInfoModule {}
