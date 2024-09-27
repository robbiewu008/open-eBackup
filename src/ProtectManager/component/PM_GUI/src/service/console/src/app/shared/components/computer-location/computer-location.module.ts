import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared/base.module';
import { ComputerLocationComponent } from './computer-location.component';

@NgModule({
  declarations: [ComputerLocationComponent],
  imports: [CommonModule, BaseModule],
  exports: [ComputerLocationComponent]
})
export class ComputerLocationModule {}
