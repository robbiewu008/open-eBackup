import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { SetBlockSizeComponent } from './set-block-size.component';

@NgModule({
  declarations: [SetBlockSizeComponent],
  imports: [CommonModule, BaseModule],
  exports: [SetBlockSizeComponent]
})
export class SetBlockSizeModule {}
