import { Pipe, PipeTransform } from '@angular/core';
import { find } from 'lodash';

@Pipe({ name: 'find', pure: false })
export class FindPipe implements PipeTransform {
  /**
   * 查找集合中指定属性的值
   *
   * @param   {any[]}   items  目标集合
   * @param   {string}  key    属性名称
   * @param   {string}  value  属性值
   *
   * @return  {[]}             [return description]
   */
  transform(items: any[], key: string, value: string | number | boolean) {
    return find(items, item => item[key] === value);
  }
}
