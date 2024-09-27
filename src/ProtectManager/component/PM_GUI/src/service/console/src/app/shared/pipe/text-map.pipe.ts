import { Pipe, PipeTransform } from '@angular/core';
import { DataMapService } from '../services';

@Pipe({
  name: 'textMap'
})
export class TextMapPipe implements PipeTransform {
  constructor(public dataMap: DataMapService) {}

  transform(value: any, configKey: string): any {
    return this.dataMap.getLabel(configKey, value);
  }
}
