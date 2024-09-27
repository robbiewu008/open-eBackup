import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { CommonModule } from '@angular/common';
import { ListItemComponent } from './list-item.component';

@NgModule({
  imports: [CommonModule, BaseModule],
  declarations: [ListItemComponent],
  exports: [ListItemComponent]
})
export class ListItemModule {}
