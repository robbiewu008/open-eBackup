import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { CurrentSystemTimeComponent } from './current-system-time.component';
import { BaseModule } from 'app/shared/base.module';

@NgModule({
  declarations: [CurrentSystemTimeComponent],
  imports: [CommonModule, BaseModule],
  exports: [CurrentSystemTimeComponent]
})
export class CurrentSystemTimeModule {}
