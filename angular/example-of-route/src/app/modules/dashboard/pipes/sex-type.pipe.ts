import { Pipe, PipeTransform } from '@angular/core';
import { SexType } from '../services/human.service';

@Pipe({
  name: 'sexType'
})
export class SexTypePipe implements PipeTransform {
  transform(value: any, args?: any): any {
    switch (value) {
      case SexType.Unknown:
        return "未知";
      case SexType.Man:
        return "男";
      case SexType.Woman:
        return "女";
    }
    return value;
  }
}
