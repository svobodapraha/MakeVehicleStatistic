//functin for common operation
#include "commonfunc.h"


//covert word to major-minor version string
QString fnWordToStringVer(word wWordVer)
{
  byte bMajorVer = byte1(wWordVer);
  byte bMinorVer = byte0(wWordVer);

  return (QString("%1.%2").arg(bMajorVer, 2, 10, QChar('0')).arg(bMinorVer, 2, 10, QChar('0')));
}
