import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { RegisterApsaraStackComponent } from './register-apsara-stack.component';

@NgModule({
  declarations: [RegisterApsaraStackComponent],
  imports: [CommonModule, BaseModule],
  exports: [RegisterApsaraStackComponent]
})
export class RegisterApsaraStackModule {}
