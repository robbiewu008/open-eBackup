import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared/base.module';
import { BaseInfoComponent } from './base-info.component';

@NgModule({
  declarations: [BaseInfoComponent],
  imports: [CommonModule, BaseModule],
  exports: [BaseInfoComponent]
})
export class BaseInfoModule {}
