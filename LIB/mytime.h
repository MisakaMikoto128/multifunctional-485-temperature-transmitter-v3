/*
 * mytime.h
 *
 *  Created on: 2020年5月16日
 *      Author: WangJianping
 *      时间相关通用操作
 *
 *
 *
 */
#ifndef MYTIME_H_
#define MYTIME_H_

#ifdef __cplusplus
extern "C"
{
#endif

  /* 自定义的时间结构体 */
  typedef struct
  {
    unsigned short nYear; //>@UTC_BASE_YEAR:1970
    unsigned char nMonth;
    unsigned char nDay;
    unsigned char nHour;
    unsigned char nMin;
    unsigned char nSec;
    unsigned char DayIndex; /* 0 = Sunday */
    unsigned short wSub;    // 亚秒
  } mytime_struct;

  unsigned char applib_dt_dayindex(unsigned short year, unsigned char month, unsigned char day); // 根据给定的日期得到对应的星期
  void utc_sec_2_mytime(unsigned int utc_sec, mytime_struct *result);                            // 根据UTC时间戳得到对应的日期
  unsigned int mytime_2_utc_sec(mytime_struct *currTime);                                        // 根据时间计算出UTC时间戳
  char *mytime_Format(unsigned int utc_sec, char *pBuf);                                         // 根据UTC时间戳得到对应的日期字符串

  void mytime_add_hours(mytime_struct *currTime, unsigned int hours);
  void mytime_sub_hours(mytime_struct *currTime, unsigned int hours);
  unsigned char time_is_equal(mytime_struct *currTime, mytime_struct *targetTime);
#ifdef __cplusplus
}
#endif
#endif
