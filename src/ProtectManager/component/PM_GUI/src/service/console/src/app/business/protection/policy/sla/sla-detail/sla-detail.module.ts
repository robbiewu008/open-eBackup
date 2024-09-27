import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { SlaInfoModule } from 'app/shared/components';
import { SlaDetailComponent } from './sla-detail.component';

@NgModule({
  declarations: [SlaDetailComponent],
  imports: [CommonModule, BaseModule, SlaInfoModule],
  exports: [SlaDetailComponent]
})
export class SlaDetailModule {}
