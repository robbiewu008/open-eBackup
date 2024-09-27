import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { ResourceListComponent } from './resource-list.component';
import { BaseModule } from 'app/shared';
import { ProButtonModule } from 'app/shared/components/pro-button';

@NgModule({
  declarations: [ResourceListComponent],
  imports: [CommonModule, BaseModule, ProButtonModule],
  exports: [ResourceListComponent]
})
export class ResourceListModule {}
