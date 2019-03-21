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
  Email: string;
}

export const HumanList: Human[] = [
  { id: 0, name: "May", sex: SexType.Woman, Email: "may@human.com" },
  { id: 1, name: "Joy", sex: SexType.Man, Email: "joy@human.com" },
  { id: 2, name: "David", sex: SexType.Man, Email: "davia@human.com" },
  { id: 3, name: "MM", sex: SexType.Unknown, Email: "mm1@human.com" },
  { id: 4, name: "John", sex: SexType.Man, Email: "john@human.com" },
]
