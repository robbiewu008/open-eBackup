import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { AddPathComponent } from './add-path.component';
import { BaseModule } from 'app/shared';

@NgModule({
  declarations: [AddPathComponent],
  imports: [CommonModule, BaseModule],
  exports: [AddPathComponent]
})
export class AddPathModule {}
