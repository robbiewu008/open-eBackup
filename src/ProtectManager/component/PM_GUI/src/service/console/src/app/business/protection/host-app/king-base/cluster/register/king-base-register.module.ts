import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { BaseModule } from 'app/shared';
import { KingBaseRegisterComponent } from './king-base-register.component';

@NgModule({
  declarations: [KingBaseRegisterComponent],
  imports: [CommonModule, BaseModule]
})
export class KingBaseRegisterModule {}
