import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { ManualCopyComponent } from './manual-copy.component';
import { BaseModule } from 'app/shared';

@NgModule({
  declarations: [ManualCopyComponent],
  imports: [CommonModule, BaseModule]
})
export class ManualCopyModule {}
