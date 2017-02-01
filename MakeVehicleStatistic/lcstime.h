#ifndef LCSTIME_H
#define LCSTIME_H
#include <QString>
#include <QTime>

class TLcsTime
{
public:
  TLcsTime();
  TLcsTime(unsigned int long dw_date_init);
  unsigned long int dw_date;
  QString asDate;
  QString asTime;
  int rok;
  int mesic;
  int den;
  int hod;
  int min;
  int sec;
  void dw2datum(void);
  QString toDateString(void);
  QString toTimeString(void);
  static unsigned long fnDateToEpocha(unsigned char * pDate);
 };

#endif // LCSTIME_H
