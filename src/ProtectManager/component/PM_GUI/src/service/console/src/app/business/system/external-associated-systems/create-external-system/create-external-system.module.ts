import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { CreateExternalSystemComponent } from './create-external-system.component';

@NgModule({
  declarations: [CreateExternalSystemComponent],
  imports: [CommonModule, BaseModule]
})
export class CreateExternalSystemModule {}
