import { CapacityCalculateLabel } from './capacity.pipe';
import { I18NPipe } from './i18n.pipe';
import { NilPipe } from './nil.pipe';
import { TextMapPipe } from './text-map.pipe';
import { FindPipe } from './find.pipe';
import { TimestampPipe } from './timestamp.pipe';

export * from './capacity.pipe';
export * from './i18n.pipe';
export * from './nil.pipe';
export * from './text-map.pipe';
export * from './find.pipe';

export const PIPES = [
  TextMapPipe,
  I18NPipe,
  CapacityCalculateLabel,
  NilPipe,
  TimestampPipe,
  FindPipe
];
