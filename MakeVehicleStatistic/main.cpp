#include "mainwindow.h"
#include <QApplication>

//Global
QFile outFile;

void myMessageHandler(QtMsgType type, const QMessageLogContext &, const QString & str)
{
 QString txt;
 switch (type) {
 case QtDebugMsg:
   txt = QString("Debug: %1").arg(str);
   break;
 case QtWarningMsg:
   txt = QString("Warning: %1").arg(str);
 break;
 case QtCriticalMsg:
   txt = QString("Critical: %1").arg(str);
 break;
 case QtFatalMsg:
   txt = QString("Fatal: %1").arg(str);
   abort();
 break;
 default:
 break;
 }
 QTextStream ts(&outFile);
 ts << txt << endl;
}

int main(int argc, char *argv[])
{
  QApplication a(argc, argv);
  //set version
  a.setApplicationVersion(APP_VERSION);


  //Debug to Log File...
  QString asLogFileName = QFileInfo( QCoreApplication::applicationFilePath() ).filePath().section(".",0,0)+".log";
  outFile.setFileName(asLogFileName);
  outFile.open(QIODevice::WriteOnly | QIODevice::Append);
  //TODO uncomment next line to allow login to file
  //qInstallMessageHandler(myMessageHandler);
  qDebug() << DBGINFO << "Program Start" << QDate::currentDate().toString() << QTime::currentTime().toString();





  MainWindow w;
  w.show();

  return a.exec();
}
