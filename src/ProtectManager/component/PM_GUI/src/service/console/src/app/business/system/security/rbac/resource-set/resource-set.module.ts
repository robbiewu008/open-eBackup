import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { ProButtonModule } from 'app/shared/components/pro-button';
import { ProTableModule } from 'app/shared/components/pro-table';
import { CreateResourcesetModule } from './create-resourceset/create-resourceset.module';
import { ResourceSetComponent } from './resource-set.component';

@NgModule({
  declarations: [ResourceSetComponent],
  imports: [
    CommonModule,
    BaseModule,
    ProTableModule,
    ProButtonModule,
    CreateResourcesetModule
  ],
  exports: [ResourceSetComponent]
})
export class ResourceSetModule {}
