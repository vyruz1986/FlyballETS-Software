import { TestBed, inject } from '@angular/core/testing';

import { EtsdataService } from './etsdata.service';

describe('EtsdataService', () => {
  beforeEach(() => {
    TestBed.configureTestingModule({
      providers: [EtsdataService]
    });
  });

  it('should be created', inject([EtsdataService], (service: EtsdataService) => {
    expect(service).toBeTruthy();
  }));
});
