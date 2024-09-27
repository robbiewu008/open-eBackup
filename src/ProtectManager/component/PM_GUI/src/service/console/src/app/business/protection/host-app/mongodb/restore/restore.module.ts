import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { RestoreComponent } from './restore.component';
import { BaseModule } from 'app/shared';

@NgModule({
  declarations: [RestoreComponent],
  imports: [CommonModule, BaseModule],
  exports: [RestoreComponent]
})
export class RestoreModule {}
