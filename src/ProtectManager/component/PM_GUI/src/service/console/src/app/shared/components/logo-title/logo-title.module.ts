import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { LogoTitleComponent } from './logo-title.component';

@NgModule({
  imports: [CommonModule],
  declarations: [LogoTitleComponent],
  exports: [LogoTitleComponent]
})
export class LogoTitleModule {}
