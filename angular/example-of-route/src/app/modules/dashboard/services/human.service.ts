import { Injectable } from '@angular/core';
import { Resolve, ActivatedRouteSnapshot } from '@angular/router';
import { Observable, of } from 'rxjs';

@Injectable({
  providedIn: 'root'
})
export class HumanService implements Resolve<Human[]> {
  constructor() { }

  async resolve(route: ActivatedRouteSnapshot) {
    return HumanList;
  }

  listHuman(): Observable<Human[]> {
    return of(HumanList);
  }

  getHuman(id: number): Observable<Human> {
    return of(HumanList.find(human => human.id == id));
  }
}

export enum SexType {
  Unknown = 0,
  Man,
  Woman,
}

export interface Human {
  id: number;
  name: string;
  sex: SexType;
  email: string;
}

export const HumanList: Human[] = [
  { id: 0, name: "May", sex: SexType.Woman, email: "may@human.com" },
  { id: 1, name: "Joy", sex: SexType.Man, email: "joy@human.com" },
  { id: 2, name: "David", sex: SexType.Man, email: "davia@human.com" },
  { id: 3, name: "MM", sex: SexType.Unknown, email: "mm1@human.com" },
  { id: 4, name: "John", sex: SexType.Man, email: "john@human.com" },
]
