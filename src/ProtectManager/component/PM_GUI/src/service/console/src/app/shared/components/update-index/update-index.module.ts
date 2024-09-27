import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { UpdateIndexComponent } from './update-index.component';
import { BaseModule } from 'app/shared/base.module';

@NgModule({
  declarations: [UpdateIndexComponent],
  imports: [CommonModule, BaseModule],
  exports: [UpdateIndexComponent]
})
export class UpdateIndexModule {}
