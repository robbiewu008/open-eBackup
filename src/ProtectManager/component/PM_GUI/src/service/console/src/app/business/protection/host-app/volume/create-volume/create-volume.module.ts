import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { ResourceFilterModule } from 'app/shared/components/resource-filter/resource-filter.module';
import { CreateVolumeComponent } from './create-volume.component';
import { CustomTableSearchModule } from 'app/shared/components/custom-table-search/custom-table-search.module';

@NgModule({
  declarations: [CreateVolumeComponent],
  imports: [
    CommonModule,
    BaseModule,
    ResourceFilterModule,
    CustomTableSearchModule
  ],
  exports: [CreateVolumeComponent]
})
export class CreateVolumeModule {}
