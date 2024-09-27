import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import {
  CreateContainerModalComponent,
  CreateProtectModalComponent
} from './create-protect-modal.component';

@NgModule({
  declarations: [CreateProtectModalComponent, CreateContainerModalComponent],
  imports: [CommonModule],
  exports: [CreateProtectModalComponent, CreateContainerModalComponent]
})
export class CreateProtectModalModule {}
