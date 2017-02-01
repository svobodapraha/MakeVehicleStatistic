#include "lcstime.h"


TLcsTime::TLcsTime()
{
  this->dw_date = 0;
}

TLcsTime::TLcsTime(unsigned int long dw_date_init)
{
  this->dw_date = dw_date_init;
  dw2datum();
}

void TLcsTime::dw2datum(void)
{
  int  sec, dny, roky, prest, pocet_dnu, min, hod;
  int  den, mesic;
//  char  str[20];

  dny = (int)(this->dw_date / 86400);
  sec = this->dw_date - (dny * 86400);
  roky = ((long)dny * 4)/1461;
  prest = (int)(roky / 4);
  pocet_dnu = dny - (365 * (roky - prest) + (366 * prest));

  hod = (int) (sec / 3600);
  sec =  sec - (hod * 3600);
  min = (int) (sec / 60);
  sec = sec - (min * 60);

  if (pocet_dnu <= 59)
  {
    if ((prest * 4) == roky)
    {
      if (pocet_dnu <= 30)
      {
        den = pocet_dnu + 1;
        mesic = 1;
      }//if (pocet_dnu <= 30)
      else
      {
        den = pocet_dnu - 30;
        mesic = 2;
      }//else
    }//if ((prest * 4) == roky)
    else
    {
      if (pocet_dnu <= 31)
      {
        den = pocet_dnu;
        mesic = 1;
      }//if (pocet_dnu <= 31)
      else
      {
        den = pocet_dnu - 31;
        mesic = 2;
      }//else
    }//else
  }//if (pocet dnu <= 59)

  else if (pocet_dnu <= 90)
  {
    den = pocet_dnu - 59;
    mesic = 3;
  }//else if (pocet_dnu <= 90)

  else if (pocet_dnu <= 120)
  {
    den = pocet_dnu - 90;
    mesic = 4;
  }//else if (pocet_dnu <= 120)

  else if (pocet_dnu <= 151)
  {
    den = pocet_dnu - 120;
    mesic = 5;
  }//else if (pocet_dnu <= 151)

  else if (pocet_dnu <= 181)
  {
    den = pocet_dnu - 151;
    mesic = 6;
  }//else if (pocet_dnu <= 181)

  else if (pocet_dnu <= 212)
  {
    den = pocet_dnu - 181;
    mesic = 7;
  }//else if (pocet_dnu <= 212)

  else if (pocet_dnu <= 243)
  {
    den = pocet_dnu - 212;
    mesic = 8;
  }//else if (pocet_dnu <= 243)

  else if (pocet_dnu <= 273)
  {
    den = pocet_dnu - 243;
    mesic = 9;
  }//else if (pocet_dnu <= 273)

  else if (pocet_dnu <= 304)
  {
    den = pocet_dnu - 273;
    mesic = 10;
  }//else if (pocet_dnu <= 304)

  else if (pocet_dnu <= 334)
  {
    den = pocet_dnu - 304;
    mesic = 11;
  }//else if (pocet_dnu <= 334)

  else
  {
    den = pocet_dnu - 334;
    mesic = 12;
  }//else (prosinec)

  roky +=2000;
  this->rok = roky;
  this->mesic = mesic;
  this->den = den;
  this->hod = hod;
  this->min = min;
  this->sec = sec;

  this->asDate = QDate(this->rok, this->mesic, this->den).toString("yyyy'-'MM'-'dd");
  this->asTime = QTime(this->hod, this->min,   this->sec).toString("hh:mm:ss");
}


QString TLcsTime::toDateString(void)
{
  return(this->asDate);
}

QString TLcsTime::toTimeString(void)
{
  return(this->asTime);
}



const unsigned char CET_CAS_DaysInMonth[12] = {31,28,31,30,31,30,31,31,30,31,30,31};

unsigned int CET_CAS_DaysInYear(unsigned char bYear)
{  if((bYear % 4)==0) return 366;
   else return 365;
}

unsigned long TLcsTime::fnDateToEpocha(unsigned char * pDate)
{
  unsigned long  l;
  unsigned char  c;
  if (pDate[0] == 24) { pDate[0] = 0;
                        pDate[3] ++;
                      }
  l = 0;
  for (c = 0; c < pDate[5]; c++) l += CET_CAS_DaysInYear(c);
  if ((CET_CAS_DaysInYear(pDate[5])==366) && (pDate[4] >= 3) ) {
    l++;
  }
  for (c=(pDate[4]-(unsigned char)1); c > 0; c--) {
    l +=  CET_CAS_DaysInMonth[c-1];
  }
  l += (pDate[3]-(unsigned char)1);
  l *= 24;
  l += pDate[0];
  l *= 60;
  l += pDate[1];
  l *= 60;
  l += pDate[2];
  return(l);
}
