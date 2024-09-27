import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { BaseModule } from 'app/shared';
import { CreateClusterComponent } from './create-cluster.component';
import { MultiAutocompleteModule } from '@iux/live';

@NgModule({
  declarations: [CreateClusterComponent],
  imports: [CommonModule, BaseModule, MultiAutocompleteModule],
  exports: [CreateClusterComponent]
})
export class CreateClusterModule {}
