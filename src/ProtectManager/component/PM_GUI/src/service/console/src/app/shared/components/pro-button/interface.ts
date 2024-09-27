import { TemplateRef } from '@angular/core';
import { Observable } from 'rxjs';

export interface ProButton {
  label: string | TemplateRef<any>;
  id?: string;
  icon?: string;
  type?: 'primary' | 'link' | 'default';
  size?: 'large' | 'default' | 'small' | 'auto';
  permission?: number | string;
  showLoading?: boolean;
  loadingText?: string;
  disabledTips?: string;
  popoverContent?: string;
  popoverShow?: boolean;
  divide?: boolean;
  items?: ProButton[];
  displayCheck?: (...args: any[]) => boolean;
  disableCheck?: (...args: any[]) => boolean;
  onClick?: (
    ...args: any[]
  ) =>
    | (boolean | void | {})
    | Promise<boolean | void | {}>
    | Observable<boolean | void | {}>;
}
