import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { InuptWithEyeComponent } from './inupt-with-eye.component';
import { IconModule, InputModule, TooltipModule } from '@iux/live';

@NgModule({
  declarations: [InuptWithEyeComponent],
  imports: [CommonModule, InputModule, IconModule, TooltipModule],
  exports: [InuptWithEyeComponent]
})
export class InuptWithEyeModule {}
