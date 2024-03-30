/*
 * mytime.c
 *
 *  Created on: 2022年5月16日
 *      Author: WangJianping
 *  时间日期数据相关通用操作
 */
 
#include "mytime.h"
#include <stdio.h>

#define UTC_BASE_YEAR 1970
#define MONTH_PER_YEAR 12
#define DAY_PER_YEAR 365
#define SEC_PER_DAY 86400
#define SEC_PER_HOUR 3600
#define SEC_PER_MIN 60

unsigned char applib_dt_is_leap_year(unsigned short year);//判断是否是闰年
unsigned char applib_dt_last_day_of_mon(unsigned char month, unsigned short year);//得到每个月有多少天


/* 每个月的天数 */
const unsigned char g_day_per_mon[MONTH_PER_YEAR] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

 /*
 * 功能：
 *     判断是否是闰年
 * 参数：
 *     year：需要判断的年份数
 *
 * 返回值：
 *     闰年返回1，否则返回0
 */
unsigned char applib_dt_is_leap_year(unsigned short year)
{
    /*----------------------------------------------------------------*/
    /* Local Variables                                                */
    /*----------------------------------------------------------------*/

    /*----------------------------------------------------------------*/
    /* Code Body                                                      */
    /*----------------------------------------------------------------*/
    if ((year % 400) == 0) {
        return 1;
    } else if ((year % 100) == 0) {
        return 0;
    } else if ((year % 4) == 0) {
        return 1;
    } else {
        return 0;
    }
}


/*
 * 功能：
 *     得到每个月有多少天
 * 参数：
 *     month：需要得到天数的月份数
 *     year：该月所对应的年份数
 *
 * 返回值：
 *     该月有多少天
 *
 */
unsigned char applib_dt_last_day_of_mon(unsigned char month, unsigned short year)
{
    /*----------------------------------------------------------------*/
    /* Local Variables                                                */
    /*----------------------------------------------------------------*/

    /*----------------------------------------------------------------*/
    /* Code Body                                                      */
    /*----------------------------------------------------------------*/
    if ((month == 0) || (month > 12)) {
        return g_day_per_mon[1] + applib_dt_is_leap_year(year);
    }

    if (month != 2) {
        return g_day_per_mon[month - 1];
    } else {
        return g_day_per_mon[1] + applib_dt_is_leap_year(year);
    }
}

/*
 * 功能：
 *     根据给定的日期得到对应的星期
 * 参数：
 *     year：给定的年份
 *     month：给定的月份
 *     day：给定的天数
 *
 * 返回值：
 *     对应的星期数，7 - 星期天 ... 6 - 星期六 ... 1-星期一
 */
unsigned char applib_dt_dayindex(unsigned short year, unsigned char month, unsigned char day)
{
    char century_code, year_code, month_code, day_code;
    int week = 0;

    century_code = year_code = month_code = day_code = 0;

    if (month == 1 || month == 2) {
        century_code = (year - 1) / 100;
        year_code = (year - 1) % 100;
        month_code = month + 12;
        day_code = day;
    } else {
        century_code = year / 100;
        year_code = year % 100;
        month_code = month;
        day_code = day;
    }

    /* 根据蔡勒公式计算星期 */
    week = year_code + year_code / 4 + century_code / 4 - 2 * century_code + 26 * ( month_code + 1 ) / 10 + day_code - 1;
    week = week > 0 ? (week % 7) : ((week % 7) + 7);
		
		if(week == 0)
			week = 7;

    return week;
}

/*
 * 功能：
 *     根据UTC时间戳得到对应的日期
 * 参数：
 *     utc_sec：给定的UTC时间戳
 *     result：计算出的结果
 *
 * 返回值：
 *     无
 */
void utc_sec_2_mytime(unsigned int utc_sec, mytime_struct *result)
{
    /*----------------------------------------------------------------*/
    /* Local Variables                                                */
    /*----------------------------------------------------------------*/
    int sec, day;
    unsigned short y;
    unsigned char m;
    unsigned short d;
    //unsigned char dst;

    /*----------------------------------------------------------------*/
    /* Code Body                                                      */
    /*----------------------------------------------------------------*/

    //if (daylightSaving) {
    //    utc_sec += SEC_PER_HOUR;
    //}

    /* hour, min, sec */
    /* hour */
    sec = utc_sec % SEC_PER_DAY;
    result->nHour = sec / SEC_PER_HOUR;

    /* min */
    sec %= SEC_PER_HOUR;
    result->nMin = sec / SEC_PER_MIN;

    /* sec */
    result->nSec = sec % SEC_PER_MIN;

    /* year, month, day */
    /* year */
    /* year */
    day = utc_sec / SEC_PER_DAY;
    for (y = UTC_BASE_YEAR; day > 0; y++) {
        d = (DAY_PER_YEAR + applib_dt_is_leap_year(y));
        if (day >= d)
        {
            day -= d;
        }
        else
        {
            break;
        }
    }

    result->nYear = y;

    for (m = 1; m < MONTH_PER_YEAR; m++) {
        d = applib_dt_last_day_of_mon(m, y);
        if (day >= d) {
            day -= d;
        } else {
            break;
        }
    }

    result->nMonth = m;
    result->nDay = (unsigned char) (day + 1);
    /* 根据给定的日期得到对应的星期 */
    result->DayIndex = applib_dt_dayindex(result->nYear, result->nMonth, result->nDay);
}

/*
 * 功能：
 *     根据时间计算出UTC时间戳
 * 参数：
 *     currTime：给定的时间
 *
 * 返回值：
 *     UTC时间戳
 */
unsigned int mytime_2_utc_sec(mytime_struct *currTime)
{
    /*----------------------------------------------------------------*/
    /* Local Variables                                                */
    /*----------------------------------------------------------------*/
    unsigned short i;
    unsigned int no_of_days = 0;
    int utc_time;
    //unsigned char dst;

    /*----------------------------------------------------------------*/
    /* Code Body                                                      */
    /*----------------------------------------------------------------*/
    if (currTime->nYear < UTC_BASE_YEAR) {
        return 0;
    }

    /* year */
    for (i = UTC_BASE_YEAR; i < currTime->nYear; i++) {
        no_of_days += (DAY_PER_YEAR + applib_dt_is_leap_year(i));
    }

    /* month */
    for (i = 1; i < currTime->nMonth; i++) {
        no_of_days += applib_dt_last_day_of_mon((unsigned char) i, currTime->nYear);
    }

    /* day */
    no_of_days += (currTime->nDay - 1);

    /* sec */
    utc_time = (unsigned int) no_of_days * SEC_PER_DAY + (unsigned int) (currTime->nHour * SEC_PER_HOUR +
                                                                currTime->nMin * SEC_PER_MIN + currTime->nSec);

    //if (dst && daylightSaving) {
    //    utc_time -= SEC_PER_HOUR;
    //}

    return utc_time;
}

//根据UTC时间戳得到对应的日期字符串
//utc_sec-utc秒
//pBuf-存放字符串数组
//返回-字符串数组
char *mytime_Format(unsigned int utc_sec,char *pBuf)
{
	mytime_struct stuTime;
	utc_sec_2_mytime(utc_sec, &stuTime);
	sprintf(pBuf, "%04d-%02d-%02d %02d:%02d:%02d", stuTime.nYear,stuTime.nMonth,stuTime.nDay,stuTime.nHour,stuTime.nMin,stuTime.nSec);
	return pBuf;
}


void mytime_add_hours(mytime_struct *currTime, unsigned int hours)
{
	unsigned int timestamp = 0;
	timestamp = mytime_2_utc_sec(currTime);
	timestamp += hours * 60UL * 60UL;
	utc_sec_2_mytime(timestamp,currTime);
}

void mytime_sub_hours(mytime_struct *currTime, unsigned int hours)
{
	unsigned int timestamp = 0;
	timestamp = mytime_2_utc_sec(currTime);
	timestamp -= hours * 60UL * 60UL;
	utc_sec_2_mytime(timestamp,currTime);
}

/**
 * @brief 判断当前时间的时分秒是否与目标时间相同
 * 
 * @param currTime 
 * @param targetTime 
 * @return unsigned char 
 */
unsigned char time_is_equal(mytime_struct *currTime,mytime_struct *targetTime)
{
    return currTime->nHour == targetTime->nHour && currTime->nMin == targetTime->nMin && currTime->nSec == targetTime->nSec;
}