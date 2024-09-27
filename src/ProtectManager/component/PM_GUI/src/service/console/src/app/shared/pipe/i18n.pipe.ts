import { Pipe, PipeTransform } from '@angular/core';
import { I18NService } from '../services/i18n.service';

@Pipe({ name: 'i18n', pure: false })
export class I18NPipe implements PipeTransform {
  constructor(private I18N: I18NService) {}

  transform(value: any, args: any[] = null, colon: boolean = false): any {
    return this.I18N.get(value, args, colon);
  }
}
