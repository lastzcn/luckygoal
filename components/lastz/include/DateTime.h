// DateTime.h

#ifndef __LASTZ_DATETIME_H__
#define __LASTZ_DATETIME_H__

#include "lastz.h"

namespace lastz{

	class LASTZ_API Date{
	public:
		Date(){};
		Date(int year, int month, int day) : m_year(year), m_month(month), m_day(day){};
		Date(const Date& other) : m_year(other.m_year), m_month(other.m_month), m_day(other.m_day){};

		int Year() const{return m_year;};
		int Month() const{return m_month;};
		int Day() const{return m_day;};

        static int GetMonthDays(int year, int month);
        
	private:
		int m_year;
		int m_month;
		int m_day;
	};

	class LASTZ_API Time{
	public:
		Time(){};
		Time(int hour, int minute, int sec, int microsec = 0):m_hour(hour), m_minute(minute), m_sec(sec), m_microsec(microsec){};
		Time(const Time& other) : m_hour(other.m_hour), m_minute(other.m_minute), m_sec(other.m_sec), m_microsec(other.m_microsec){};

		int Hour() const{return m_hour;};
		int Minute() const{return m_minute;};
		int Second() const{return m_sec;};
		int MicroSecond() const{return m_microsec;};

        long TotalMicroSeconds() const;
        
	private:
		int m_hour;
		int m_minute;
		int m_sec;
		int m_microsec;
	};

	class LASTZ_API DateTime{
	public:
         DateTime(){};
    DateTime(const Date& day, const Time& time) : m_day(day), m_time(time){};
		static DateTime Now();
        static Time Duration(const DateTime& dt1, const DateTime& dt2);

		std::string ToString() const;
        std::string DateTimeString() const;

        Date GetDate() const {return m_day;};
        Time GetTime() const {return m_time;};
        long TotalFractionalSeconds();
        
	private:
		Date m_day;
		Time m_time;
	};
}

#endif // __LASTZ_DATETIME_H__

