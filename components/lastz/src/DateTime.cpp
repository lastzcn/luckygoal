// DateTime.cpp

#include "DateTime.h"
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

using namespace boost::gregorian;
using namespace boost::posix_time;

// **************** LastZ Check Leaks **********************************
// Define LASTZ_CHECKLEAKS macro in compile option to enable leak check.
// And put this code behind all #include in your .cpp file.
//#include "lastz/CheckLeaks.h"
// *********************************************************************

using namespace lastz;

int Date::GetMonthDays(int year, int month) 
{
    date_duration dd(1);
    if ( month < 12 ){
        month++;
    } else {
        return 31;
    }
    
    date d(year, month, 1);
    d -= dd;
    return d.day();
}


long Time::TotalMicroSeconds() const
{
    time_duration td(m_hour, m_minute, m_sec, m_microsec * 1000);
    return td.total_microseconds() / 1000;
}

lastz::DateTime DateTime::Now(){
	ptime now = microsec_clock::local_time();
	date today = now.date();
	time_duration td = now.time_of_day();

	return DateTime(Date(today.year(), today.month(), today.day()),
					Time(td.hours(), td.minutes(), td.seconds(), td.fractional_seconds() / 1000));
}

std::string DateTime::ToString() const{
    char buf[1024];
    sprintf(buf, "%04d-%02d-%02d %02d:%02d:%02d.%03d",
            m_day.Year(), m_day.Month(), m_day.Day(),
            m_time.Hour(), m_time.Minute(), m_time.Second(), m_time.MicroSecond());
    std::string strTime = buf;
    
	return strTime;
}

std::string DateTime::DateTimeString() const
{
    char buf[1024];
    sprintf(buf, "%04d%02d%02d%02d%02d%02d%03d",
            m_day.Year(), m_day.Month(), m_day.Day(),
            m_time.Hour(), m_time.Minute(), m_time.Second(), m_time.MicroSecond());
    std::string strTime = buf;
    
	return strTime;
}

ptime ToPTime(const DateTime& dt)
{
    Date date1 = dt.GetDate();
    Time time1 = dt.GetTime();
    
    int y1 = date1.Year();
    int m1 = date1.Month();
    int d1 = date1.Day();
    int h1 = time1.Hour();
    int mi1 = time1.Minute();
    int s1 = time1.Second();
    long fractional_seconds = time1.MicroSecond() * 1000;

    return ptime(date(y1, m1, d1), time_duration(h1, mi1, s1, fractional_seconds));
}

long DateTime::TotalFractionalSeconds()
{
    ptime p1 = ToPTime(*this);
    ptime p0 = ToPTime(DateTime(Date(1970,1,1), Time(0,0,0)));
    time_duration td = p1 - p0 + time_duration(8,0,0,0);
    return td.fractional_seconds();
    
}

Time DateTime::Duration(const DateTime& dt1, const DateTime& dt2)
{
    ptime pt1 = ToPTime(dt1);
    ptime pt2 = ToPTime(dt2);

    time_duration td = pt2 - pt1;
    int ms = td.fractional_seconds() / 1000;
    
    return Time(td.hours(), td.minutes(), td.seconds(), ms);
}

