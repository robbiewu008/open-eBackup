import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';

import { ErrorPageRoutingModule } from './error-page-routing.module';
import { ErrorPageComponent } from './error-page.component';
import { BaseModule } from 'app/shared';

@NgModule({
  declarations: [ErrorPageComponent],
  imports: [CommonModule, BaseModule, ErrorPageRoutingModule]
})
export class ErrorPageModule {}
