import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { RegisterGroupComponent } from './register-group.component';
import { BaseModule } from 'app/shared';

@NgModule({
  declarations: [RegisterGroupComponent],
  imports: [CommonModule, BaseModule],
  exports: [RegisterGroupComponent]
})
export class RegisterGroupModule {}
