import { Injectable } from '@angular/core';
import { Title } from '@angular/platform-browser';
import { HttpClient } from '@angular/common/http';
import { tap, map, finalize } from 'rxjs/operators';
import { Observable } from 'rxjs';
import {
  each,
  endsWith,
  first,
  includes,
  isNil,
  join,
  last,
  map as __map,
  split,
  startsWith,
  trim
} from 'lodash';

export interface IoemInfo {
  brand: string;
  copyright_en: string;
  copyright_zh: string;
  product: string;
  productModel: string;
  vendor: string;
  warn_en: string;
  warn_zh: string;
  website_en: string;
  website_zh: string;
}

export const IMAGE_PATH_PREFIX = 'assets/whitebox/resources/image/' as const;
const OEM_PATH = 'assets/whitebox/resources/oem.js' as const;
const USELESS_OEM_KEYS = [
  '',
  'OEM',
  'path',
  'controller_enclosure_2u_25_back',
  'controller_enclosure_2u_25_front',
  'controller_enclosure_2u_25_plam_back',
  'controller_enclosure_2u_25_plam_front',
  'controller_enclosure_2u_36_back',
  'controller_enclosure_2u_36_front',
  'disk_enclosure_2u_36_back',
  'disk_enclosure_2u_36_front',
  'disk_enclosure_25_back',
  'disk_enclosure_25_front'
] as const;

@Injectable({
  providedIn: 'root'
})
export class WhiteboxService {
  private _oem: void | IoemInfo;

  constructor(private http: HttpClient, private titleService: Title) {}

  /**
   * 是否白牌化
   */
  get isWhitebox() {
    return !isNil(this._oem);
  }

  /**
   * oem 信息
   */
  get oem() {
    return this._oem;
  }

  /**
   * 加载 oem.js
   * @param path oem.js 路径（非必传）
   */
  public loadOemFile(path: string = OEM_PATH): Observable<IoemInfo> {
    return this.http
      .get(path, {
        params: {
          akOperationTips: 'false',
          akDoException: 'false',
          akLoading: 'false'
        },
        responseType: 'text'
      })
      .pipe(
        map(res => this.parseOem(res)),
        tap((res: IoemInfo) => (this._oem = res)),
        finalize(() => this.setDocumentTitle())
      );
  }

  /**
   * 解析 oem 内容
   * @param text oem.js 内容
   * @returns 解析后的 oem 对象
   */
  private parseOem(text: string): IoemInfo {
    const result = {};
    each(
      __map(
        __map(split(text, ';'), (line: string) => split(line, '=')),
        (pair: string[]) => [pair.shift(), join(pair, '=')]
      ),
      (pair: string[]) => {
        const key = trim(last(first(pair).split('.')));
        if (!includes(USELESS_OEM_KEYS, key)) {
          result[key] = this.removeQuota(last(pair));
        }
      }
    );
    return result as IoemInfo;
  }

  /**
   * 去除多余引号
   * @param str 待处理字符串
   * @returns 处理后字符串
   */
  private removeQuota(str: string): string {
    const result = trim(str);
    const matchQuota = (symbol: string) =>
      startsWith(result, symbol) && endsWith(result, symbol);

    if (matchQuota(`'`) || matchQuota(`"`)) {
      return trim(result.substring(1, result.length - 1));
    }
    return trim(result);
  }

  /**
   * 设置 document 标题
   */
  private setDocumentTitle() {
    this.isWhitebox &&
      this.titleService.setTitle((this.oem as IoemInfo).vendor);
    return this;
  }
}
