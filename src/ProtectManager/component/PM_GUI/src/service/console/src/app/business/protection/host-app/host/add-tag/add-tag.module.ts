import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { AddTagComponent } from './add-tag.component';
import { BaseModule } from 'app/shared';

@NgModule({
  imports: [CommonModule, BaseModule],
  exports: [AddTagComponent],
  declarations: [AddTagComponent]
})
export class AddTagModule {}
