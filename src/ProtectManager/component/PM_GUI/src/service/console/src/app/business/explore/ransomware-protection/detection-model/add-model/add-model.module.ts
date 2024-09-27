import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { AddModelComponent } from './add-model.component';
import { BaseModule } from 'app/shared';

@NgModule({
  declarations: [AddModelComponent],
  imports: [CommonModule, BaseModule],
  exports: [AddModelComponent]
})
export class AddModelModule {}
