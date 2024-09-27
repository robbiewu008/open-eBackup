import { LvDateTimeOption } from '@iux/live';

export interface ProFilterDateConfig {
  dateRange?: boolean;
  showTime?: boolean | LvDateTimeOption;
  format?: string;
  timezoneOffset?: number;
  disabledDate?: (date: Date) => boolean;
  onlyShowActiveCell?: boolean;
  showTodayButton?: boolean;
  showNowButton?: boolean;
  placeholder?: string | string[];
  presetRanges?: { [key: string]: Date[] }[];
}
