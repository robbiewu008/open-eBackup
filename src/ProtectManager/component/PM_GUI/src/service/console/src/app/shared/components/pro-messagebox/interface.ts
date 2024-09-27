import { ModalButton } from '@iux/live';
import { Observable } from 'rxjs';
import { TableConfig } from '../pro-table';

export interface ProMessageboxOptions {
  modalKey?: string;
  width?: number | string;
  height?: number | string;
  title?: string;
  icon?:
    | 'question'
    | 'info'
    | 'success'
    | 'error'
    | 'warning'
    | 'danger'
    | string;
  panelClass?: string;
  content: ProMessageboxContentOptions;
  closeButtonDisplay?: boolean;
  beforeOpen?: ProMessageboxCallback; // 无返回参数
  afterOpen?: ProMessageboxCallback; // 返回参数：modal
  beforeClose?: ProMessageboxCallback; // 返回参数：modal、result
  afterClose?: ProMessageboxCallback; // 返回参数：result

  okButton?: {
    showLoading?: boolean;
    loadingText?: string;
    disabled?: boolean;
    onClick: ProMessageboxCallback; // 返回参数：modal
  };
  cancelButton?: {
    showLoading?: boolean;
    loadingText?: string;
    disabled?: boolean;
    onClick: ProMessageboxCallback; // 返回参数：modal
  };
  customButtons?: ModalButton[] | null;
}

export type ProMessageboxCallback = (
  ...args: any[]
) =>
  | (boolean | void | {})
  | Promise<boolean | void | {}>
  | Observable<boolean | void | {}>;

export interface ProMessageboxContentOptions {
  description?: {
    type?: 'html' | 'text';
    value?: string;
  };
  list?: TableConfig;
  checkbox?: {
    data?: { value: string; label: string; disabled?: boolean }[];
    value?: string[];
    valueChange?: (value, modal, checkboxs) => any;
  };
}
