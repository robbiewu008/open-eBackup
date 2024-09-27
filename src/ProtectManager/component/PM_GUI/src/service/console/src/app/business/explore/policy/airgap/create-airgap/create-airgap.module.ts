import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { CreateAirgapComponent } from './create-airgap.component';
import { BaseModule } from 'app/shared';

@NgModule({
  declarations: [CreateAirgapComponent],
  imports: [CommonModule, BaseModule]
})
export class CreateAirgapModule {}
