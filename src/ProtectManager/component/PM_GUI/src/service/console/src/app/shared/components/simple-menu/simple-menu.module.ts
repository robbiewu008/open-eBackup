import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { SimpleMenuComponent } from './simple-menu.component';
import { IconModule, OverlaysModule, MenuModule } from '@iux/live';

@NgModule({
  declarations: [SimpleMenuComponent],
  imports: [CommonModule, IconModule, OverlaysModule, MenuModule],
  exports: [SimpleMenuComponent]
})
export class SimpleMenuModule {}
