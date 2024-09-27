import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { BaseModule } from 'app/shared';
import { AdvancedEmailComponent } from './advanced-email.component';

@NgModule({
  declarations: [AdvancedEmailComponent],
  imports: [CommonModule, BaseModule],
  exports: [AdvancedEmailComponent]
})
export class AdvancedEmailModule {}
