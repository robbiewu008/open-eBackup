import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { RegisterGaussdbTComponent } from './register-gaussdb-t.component';
import { BaseModule } from 'app/shared';

@NgModule({
  declarations: [RegisterGaussdbTComponent],
  imports: [CommonModule, BaseModule]
})
export class RegisterGaussdbTModule {}
