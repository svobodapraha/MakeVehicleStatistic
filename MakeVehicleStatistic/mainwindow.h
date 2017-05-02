#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlTableModel>
#include <QSqlRelationalTableModel>
#include <QSqlRelationalDelegate>
#include <QSqlQuery>
#include <QSettings>
#include <QSqlDriver>
#include <QFileDialog>
#include <QMessageBox>
#include <QUdpSocket>
#include "lcstime.h"
#include "commonfunc.h"
#include <QDateTime>
#include <QTcpServer>
#include <QTcpSocket>
#include <QProcess>
#include <QSqlRecord>
#include "const.h"
#include "vehicle.h"

//defines - constants
#define MAIN_TIMER_PERIOD 250
#define AUX_TIMER_PERIOD  2500
#define DEFAULT_SMTP_SERVER_PORT 25




class VehicleRelationalTableModel : public QSqlRelationalTableModel
{
  Q_OBJECT
public:
  VehicleRelationalTableModel(QObject * parent = 0, QSqlDatabase db = QSqlDatabase())
    :QSqlRelationalTableModel(parent, db){;}

  Qt::ItemFlags flags(const QModelIndex &index) const
  {
      Qt::ItemFlags flags = QAbstractTableModel::flags(index);
      return  flags;
  }

  QVariant data ( const QModelIndex & index, int role = Qt::DisplayRole ) const
  {
    return QSqlRelationalTableModel::data(index,role);
  }
};



namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
  Q_OBJECT

public:
  explicit MainWindow(QWidget *parent = 0);
  ~MainWindow();

private slots:

  void on_btn_Debug1_clicked();

  void on_Btn_FillVehicleList_fromDB_clicked();

  void on_tableView_Vehicles_clicked(const QModelIndex &index);

  void on_tableWidget_QueryStatus_clicked(const QModelIndex &index);


private:
  Ui::MainWindow *ui;
  QTcpServer * httpServer;
  bool boForceFillVehicleFromDB;
  int iResetTimerReq_Log;
  int iResetTimerReq_OpData;
  QFile GeneralDebugFile;
  QTextStream GeneralDebugStream;
  bool boGeneralDebugOK;
  QUdpSocket * UdpSocketOBU;
  int iLastSelectedVehicleForStatistic;
  int iLastSelectedProjectForStatistic;
  QDate StatisticDateFrom;
  QDate StatisticDateTo;



//models
  VehicleRelationalTableModel *mVehicleModel;
  QSqlRelationalTableModel    *mSpamModel;
  QSqlRelationalTableModel    *mFailureCountModel;
  QSqlRelationalTableModel    *mProjectFailureCountModel;

//timer and comm part
protected:
  void timerEvent(QTimerEvent * event);
  int fnConnectToDB(bool boForceExit = false);
  int fnTestConnection(void);
  int fnTestVehicleChanged(void);
  int iNumberOfConnectAttempts;
  QDateTime LastDateTimeConnected;
  QDateTime LastDateTimeDisConnected;
  int iMainTimerID;
  int iAuxTimerID;

  //DB info
  QString asDatabaseName;
  QString asUserName;
  QString asPassword;
  QString asHostName;

  //SMTP info
  bool boSMTPSettingsComplete;
  QString asSMTPServer;
  int     iSMTPPort;
  QString asSMTPSenderAddress;
  QString asDefautRecipient;
  bool    boSMTPAuth;
  bool    boSMTPSsl;
  QString asSMTPUser;
  QString asSMTPPass;

public:

  QString fnAssembleHTMLReport();

public slots:
  void fnSendEmail(TVehicle*, int iInfo, QString asEmailMessageToSend, TeEmailCategory eMailCategory, QString asAddToSubject = "");


private slots:
  void fnAcceptHTTPConnection(void);
  void fnReadFromSocket(void);
  void fnClosingClient(void);

  void on_Btn_SendTestEmail_clicked();
  void on_Btn_HtmlTest_clicked();
  void on_calendarWidget_statisticFrom_selectionChanged();
  void on_btnMakeVehicleFailureStatistic_clicked();
  void on_btnMakeProjectFailureStatistic_clicked();

  void on_btn_ClearFailureCount_clicked();
  void on_btn_ClearProjectFailureCount_clicked();
  void on_btn_LastWeek_clicked();
  void on_btn_LastMonth_clicked();
  void on_btn_LastYear_clicked();
};



#endif // MAINWINDOW_H
