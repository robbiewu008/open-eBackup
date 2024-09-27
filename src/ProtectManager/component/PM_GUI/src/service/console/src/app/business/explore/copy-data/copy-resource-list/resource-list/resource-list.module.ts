import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { ResourceListComponent } from './resource-list.component';
import { BaseModule } from 'app/shared';

@NgModule({
  declarations: [ResourceListComponent],
  imports: [CommonModule, BaseModule],
  exports: [ResourceListComponent]
})
export class ResourceListModule {}
