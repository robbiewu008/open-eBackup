import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { BaseModule } from 'app/shared';
import { AirgapDetailComponent } from './airgap-detail.component';
import { ProTableModule } from 'app/shared/components/pro-table';

@NgModule({
  declarations: [AirgapDetailComponent],
  imports: [CommonModule, BaseModule, ProTableModule]
})
export class AirgapDetailModule {}
