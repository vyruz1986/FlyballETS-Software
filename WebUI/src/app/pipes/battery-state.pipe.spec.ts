import { BatteryStatePipe } from './battery-state.pipe';

describe('BatteryStatePipe', () => {
  it('create an instance', () => {
    const pipe = new BatteryStatePipe();
    expect(pipe).toBeTruthy();
  });
});