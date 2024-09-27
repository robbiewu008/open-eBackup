import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { DestroyLiveMountComponent } from './destroy-live-mount.component';
import { BaseModule } from 'app/shared';

@NgModule({
  declarations: [DestroyLiveMountComponent],
  imports: [CommonModule, BaseModule],
  exports: [DestroyLiveMountComponent]
})
export class DestroyLiveMountModule {}
