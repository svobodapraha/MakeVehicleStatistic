#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "qdebug.h"
#include "stdint.h"



#if defined(Q_OS_WIN)
  #include "windows.h"
#endif

#include "SmtpMime"

//GLOBAL VAR

QSqlDatabase TramDB;
QSqlDatabase LocalStatisticDB;
QList<TVehicle*> VehicleList;



int col_VehicleList_idVehicle = -1;
int col_VehicleList_VehicleNumber = -1;
int col_VehicleList_VehicleStatus = -1;
int col_VehicleList_IPV4 = -1;
int col_VehicleList_idProject = -1;
int col_VehicleList_ReadLogInterval = -1;
int col_VehicleList_ReadOpDataInterval = -1;

int col_SpamList_idSpamList = -1;
int col_SpamList_tUsers_idUser = -1;
int col_SpamList_EmailAddress = -1;
int col_SpamList_boSendDBEvents = -1;
int col_SpamList_boSendNewVehicle = -1;
int col_SpamList_boReadVehicle = -1;
int col_SpamList_boWriteLogVehicle = -1;
int col_SpamList_boPeriodicalReport = -1;
int col_SpamList_tProjectList_idProject = -1;


//GLOBAL VAR - END

MainWindow::MainWindow(QWidget *parent) :
  QMainWindow(parent),
  ui(new Ui::MainWindow)

{
  ui->setupUi(this);
  iLastSelectedVehicleForStatistic = -1;

//Int Debug File
  GeneralDebugFile.setFileName
  (
    QFileInfo( QCoreApplication::applicationFilePath() ).filePath().section(".",0,0)+".generalDebug.txt"
  );
  boGeneralDebugOK = GeneralDebugFile.open(QIODevice::WriteOnly | QIODevice::Text);

  if(boGeneralDebugOK)
  {
    GeneralDebugStream.setDevice(&GeneralDebugFile);
  }

  if(boGeneralDebugOK)
  {
    GeneralDebugStream << DBGINFO << ";" << TIMEINFO << endl;
    GeneralDebugStream << "Program Start" << endl;
    GeneralDebugStream.flush();
  }

  int iResult = 0;
  bool boResult = false;
  boForceFillVehicleFromDB = false;
  iResetTimerReq_Log = -1;
  iResetTimerReq_OpData = -1;


  //display version
  setWindowTitle("MakeVehicleStatistic V:" + QApplication::applicationVersion());

  //ini ui components
  ui->plainTextEdit_dBInfo->setReadOnly(true);

  //get connection parameters from ini file
  asDatabaseName ="";
  asUserName     ="";
  asPassword     ="";
  asHostName     ="";

  //get SMTP parameters form ini file too
  asSMTPServer         ="";
  iSMTPPort            =DEFAULT_SMTP_SERVER_PORT;
  asSMTPSenderAddress  ="";
  asDefautRecipient    ="";
  boSMTPAuth           = false;
  boSMTPSsl            = false;
  asSMTPUser           ="";
  asSMTPPass           ="";




  QString asIniFileName = QFileInfo( QCoreApplication::applicationFilePath() ).filePath().section(".",0,0)+".ini";
  QSettings settings(asIniFileName, QSettings::IniFormat);
  qDebug() << DBGINFO << settings.status();
  QVariant VarTemp;

  settings.beginGroup("DATABASE");


    bool boDBSettingsComplete = true;

    VarTemp = settings.value("DatabaseName");
    if (VarTemp.isValid())
    {
      asDatabaseName = VarTemp.toString();
    }
    else
    {
      asDatabaseName = "Missing Value";
      boDBSettingsComplete = false;
    }

    VarTemp = settings.value("UserName");
    if (VarTemp.isValid())
    {
      asUserName = VarTemp.toString();
    }
    else
    {
      asUserName = "Missing Value";
      boDBSettingsComplete = false;
    }

    VarTemp = settings.value("Password");
    if (VarTemp.isValid())
    {
      asPassword = VarTemp.toString();
    }
    else
    {
      asPassword = "Missing Value";
      boDBSettingsComplete = false;
    }

    VarTemp = settings.value("HostName");
    if (VarTemp.isValid())
    {
      asHostName = VarTemp.toString();
    }
    else
    {
      asHostName = "Missing Value";
      boDBSettingsComplete = false;
    }

  settings.endGroup();




  if (!boDBSettingsComplete)
  {
    QPlainTextEdit PlainTextEditInfo;

    PlainTextEditInfo.show();
    PlainTextEditInfo.appendPlainText("Some DB Connection parameters are missing");
    PlainTextEditInfo.appendPlainText("");
    PlainTextEditInfo.appendPlainText(asDatabaseName);
    PlainTextEditInfo.appendPlainText(asUserName);
    PlainTextEditInfo.appendPlainText(asPassword);
    PlainTextEditInfo.appendPlainText(asHostName);

    QMessageBox::information(this, "Missing connection parameters", "Program will be closed", QMessageBox::Ok);
    fnSendEmail(NULL, 0, QString("%1 @ %2: To DefautRecipient-> Missing connection parameters").arg(__LINE__).arg(__FILENAME__), ecSendDBEvents);
    exit(-1);
  }

  //SMTP Params
  settings.beginGroup("SMTPEmail");
    boSMTPSettingsComplete = true;

    VarTemp = settings.value("SMTPServer");
    if (VarTemp.isValid())
    {
      asSMTPServer = VarTemp.toString();
    }
    else
    {
      asSMTPServer = "Missing Value";
      boSMTPSettingsComplete = false;
    }

    VarTemp = settings.value("SMTPPort");
    if (VarTemp.isValid())
    {
      bool boOK = false;
      iSMTPPort = VarTemp.toInt(&boOK);
      if (!boOK)
      {
        iSMTPPort = DEFAULT_SMTP_SERVER_PORT;
        boSMTPSettingsComplete = false;
      }
    }
    else
    {
      iSMTPPort = DEFAULT_SMTP_SERVER_PORT;
    }


    VarTemp = settings.value("SMTPSenderAddress");
    if (VarTemp.isValid())
    {
      asSMTPSenderAddress = VarTemp.toString();
    }
    else
    {
      asSMTPSenderAddress = "Missing Value";
      boSMTPSettingsComplete = false;
    }

    VarTemp = settings.value("DefautRecipient");
    if (VarTemp.isValid())
    {
      asDefautRecipient = VarTemp.toString();
    }
    else
    {
      asDefautRecipient = "Missing Value";
      boSMTPSettingsComplete = false;
    }

    VarTemp = settings.value("SMTPAuth");
    if (VarTemp.isValid())
    {
      bool boOK = false;
      boSMTPAuth = (bool)VarTemp.toInt(&boOK);
      if (!boOK)
      {
        boSMTPAuth = false;
        boSMTPSettingsComplete = false;
      }
    }
    else
    {
      boSMTPAuth = false;
      boSMTPSettingsComplete = false;
    }

    VarTemp = settings.value("SMTPSsl");
    if (VarTemp.isValid())
    {
      bool boOK = false;
      boSMTPSsl = (bool)VarTemp.toInt(&boOK);
      if (!boOK)
      {
        boSMTPSsl = false;
        boSMTPSettingsComplete = false;
      }
    }
    else
    {
      boSMTPSsl = false;
    }


    if (boSMTPAuth)
    {
      VarTemp = settings.value("SMTPUser");
      if (VarTemp.isValid())
      {
        asSMTPUser = VarTemp.toString();
      }
      else
      {
        asSMTPUser = "Missing Value";
        boSMTPSettingsComplete = false;
      }

      VarTemp = settings.value("SMTPPass");
      if (VarTemp.isValid())
      {
        asSMTPPass = VarTemp.toString();
      }
      else
      {
        asSMTPPass = "Missing Value";
        boSMTPSettingsComplete = false;
      }

    }


  settings.endGroup();

  ui->plainTextEdit_SMTPInfo->clear();
  if (boSMTPSettingsComplete)
  {
    ui->plainTextEdit_SMTPInfo->appendPlainText("SMPT OK");
    ui->plainTextEdit_SMTPInfo->appendPlainText(asSMTPServer);
    ui->plainTextEdit_SMTPInfo->appendPlainText(QString("Port: %1").arg(iSMTPPort));
    ui->plainTextEdit_SMTPInfo->appendPlainText(asSMTPSenderAddress);
    ui->plainTextEdit_SMTPInfo->appendPlainText(QString("Ath %1").arg(boSMTPAuth));
    ui->plainTextEdit_SMTPInfo->appendPlainText(QString("SSL %1").arg(boSMTPSsl));
    if (boSMTPAuth)
    {
      ui->plainTextEdit_SMTPInfo->appendPlainText(asSMTPUser);
      ui->plainTextEdit_SMTPInfo->appendPlainText(asSMTPPass);
    }
  } else
  {
    ui->plainTextEdit_SMTPInfo->appendPlainText("SMPT Param. Error");
  }



  LastDateTimeConnected = QDateTime::currentDateTimeUtc();
  LastDateTimeDisConnected = QDateTime::currentDateTimeUtc();

  //connect to db
  TramDB = QSqlDatabase::addDatabase("QMYSQL");
  mVehicleModel = new VehicleRelationalTableModel(this);
  mSpamModel = new QSqlRelationalTableModel(this);

  int iDBResult = 0;
  iDBResult = fnConnectToDB(true);





  //***** TO DO ****

  //set dbTables for vehicles and find columns
  mVehicleModel->setEditStrategy(QSqlTableModel::OnManualSubmit);
  mVehicleModel->setTable("tVehicleList");
  mVehicleModel->select();
  ui->tableView_Vehicles->setModel(mVehicleModel);

  //find columns
  col_VehicleList_idVehicle = mVehicleModel->fieldIndex("idVehicle");
    //EXIT_WHEN_FIELD_NOT_FOUND(col_VehicleList_idVehicle);
  col_VehicleList_VehicleNumber = mVehicleModel->fieldIndex("VehicleNumber");
    //EXIT_WHEN_FIELD_NOT_FOUND(col_VehicleList_VehicleNumber);
  col_VehicleList_VehicleStatus = mVehicleModel->fieldIndex("VehicleStatus");
    //EXIT_WHEN_FIELD_NOT_FOUND(col_VehicleList_VehicleStatus);
  col_VehicleList_IPV4 = mVehicleModel->fieldIndex("IPV4");
    //EXIT_WHEN_FIELD_NOT_FOUND(col_VehicleList_IPV4);
  col_VehicleList_idProject = mVehicleModel->fieldIndex("tProjectList_idProject");
    //EXIT_WHEN_FIELD_NOT_FOUND(col_VehicleList_idProject);
  col_VehicleList_ReadLogInterval = mVehicleModel->fieldIndex("ReadLogInterval");
    //EXIT_WHEN_FIELD_NOT_FOUND(col_VehicleList_ReadLogInterval);
  col_VehicleList_ReadOpDataInterval = mVehicleModel->fieldIndex("ReadOpDataInterval");
    //EXIT_WHEN_FIELD_NOT_FOUND(col_VehicleList_ReadOpDataInterval);


  //set query table header
  QStringList tableHeader = QStringList() <<"ID"
                                          <<"JamID"
                                          <<"IPV4"
                                          <<"TIMER"
                                          <<"STATUS"
                                          <<"ATTEMPTS"
                                          <<"ROWS"
                                          <<"WHEN (UTC)"
                                          <<"WAIT DATA"
                                          <<"TIMER_OD"
                                          <<"STATUS OD"
                                          <<"ATTEMPTS OD"
                                          <<"WHEN OD"
                                          <<"WAIT DATA OD"
                                          <<"WAIT_DATA_VS"
                                          <<"HB_REQ";                                            ;


  ui->tableWidget_QueryStatus->setColumnCount(tableHeader.count());
  ui->tableWidget_QueryStatus->setHorizontalHeaderLabels(tableHeader);

  //set table for SMTP a find columns
  mSpamModel->setEditStrategy(QSqlTableModel::OnManualSubmit);
  mSpamModel->setTable("tSpamList");
  mSpamModel->select();

  col_SpamList_idSpamList            = mSpamModel->fieldIndex("idSpamList");
  //EXIT_WHEN_FIELD_NOT_FOUND(col_SpamList_idSpamList);
  col_SpamList_tUsers_idUser         = mSpamModel->fieldIndex("tUsers_idUser");
  //EXIT_WHEN_FIELD_NOT_FOUND(col_SpamList_tUsers_idUser);
  col_SpamList_EmailAddress          = mSpamModel->fieldIndex("EmailAddress");
  //EXIT_WHEN_FIELD_NOT_FOUND(col_SpamList_EmailAddress);
  col_SpamList_boSendDBEvents         = mSpamModel->fieldIndex("boSendDBEvents");
  //EXIT_WHEN_FIELD_NOT_FOUND(col_SpamList_boSendDBEvents);
  col_SpamList_boSendNewVehicle       = mSpamModel->fieldIndex("boSendNewVehicle");
  //EXIT_WHEN_FIELD_NOT_FOUND(col_SpamList_boSendNewVehicle);
  col_SpamList_boReadVehicle          = mSpamModel->fieldIndex("boReadVehicle");
  //EXIT_WHEN_FIELD_NOT_FOUND(col_SpamList_boReadVehicle);
  col_SpamList_boWriteLogVehicle      = mSpamModel->fieldIndex("boWriteLogVehicle");
  //EXIT_WHEN_FIELD_NOT_FOUND(col_SpamList_boWriteLogVehicle);
  col_SpamList_boPeriodicalReport     = mSpamModel->fieldIndex("boPeriodicalReport");
  //EXIT_WHEN_FIELD_NOT_FOUND(col_SpamList_boPeriodicalReport);
  col_SpamList_tProjectList_idProject = mSpamModel->fieldIndex("tProjectList_idProject");
  //EXIT_WHEN_FIELD_NOT_FOUND(col_SpamList_tProjectList_idProject)


  //connest to SQLITE Database...
  LocalStatisticDB = QSqlDatabase::addDatabase("QSQLITE", "StatisticDB");
  mFailureCountModel =        new QSqlRelationalTableModel(this, LocalStatisticDB);
  mProjectFailureCountModel = new QSqlRelationalTableModel(this, LocalStatisticDB);
  LocalStatisticDB.setDatabaseName(":memory:");
  boResult = LocalStatisticDB.open();
  if(boResult)
  {
    qDebug() << "Statistic db opened OK";
  }
  else
  {
    qDebug() << "Error open statistic db " << LocalStatisticDB.lastError().text();
  }

  QSqlQuery StatisticQuery(LocalStatisticDB);

  //create statistic table for vehicles:
  boResult = StatisticQuery.exec("DROP TABLE tFailureCount; ");
  boResult = StatisticQuery.exec
  (
       "create table tFailureCount ("
       "id integer primary key autoincrement, "
       "VehicleId int, "
       "EventIndex int, "
       "SourceUnit varchar, "
       "FailureCount int "
       ");"
  );
  qDebug() << DBGINFO << boResult << "create" << StatisticQuery.lastError().text();

//  //Test data
//  boResult = StatisticQuery.exec
//  (
//    "insert into tFailureCount values (1, 1, 2, \"JAMIC\", 2);"
//  );
//  qDebug() << boResult << "insert" << StatisticQuery.lastError().text();

  //create statistic table for project:
  boResult = StatisticQuery.exec("DROP TABLE tProjectFailureCount; ");
  boResult = StatisticQuery.exec
  (
       "create table tProjectFailureCount ("
       "id integer primary key autoincrement, "
       "ProjectId int, "
       "EventIndex int, "
       "SourceUnit varchar, "
       "ProjectFailureCount int "
       ");"
  );
  qDebug() << DBGINFO << boResult << "create" << StatisticQuery.lastError().text();



  //set models a tables
  mFailureCountModel->setEditStrategy(QSqlTableModel::OnManualSubmit);
  mFailureCountModel->setTable("tFailureCount");
  boResult = mFailureCountModel->select();
  ui->tableView_FailureCount->setModel(mFailureCountModel);

  mProjectFailureCountModel->setEditStrategy(QSqlTableModel::OnManualSubmit);
  mProjectFailureCountModel->setTable("tProjectFailureCount");
  boResult = mProjectFailureCountModel->select();
  ui->tableView_ProjectFailureCount->setModel(mProjectFailureCountModel);


  //start timers
  iMainTimerID = this->startTimer(MAIN_TIMER_PERIOD);
  iAuxTimerID = this->startTimer(AUX_TIMER_PERIOD);

//#define HTTP_LISTEN_PORT 9080
  #define HTTP_LISTEN_PORT 9081


  //start http server for program monitoring
  httpServer = new QTcpServer(this);
  boResult = httpServer->listen(QHostAddress::Any, HTTP_LISTEN_PORT);
  qDebug() << DBGINFO  << "HTTP server listen " << boResult;

  connect(httpServer, SIGNAL(newConnection()),
          this,       SLOT(fnAcceptHTTPConnection()));


  //registr port for BCU
  UdpSocketOBU = new QUdpSocket(this);
  boResult = UdpSocketOBU->bind(QHostAddress(0U), HEARTBEAT_RECEIVE_PORT, QAbstractSocket::DontShareAddress);
  qDebug() << DBGINFO << "UDP HEARTBEAT RECEIVE" << boResult;
  if (!boResult)
  {
    fnSendEmail(NULL, 0, QString("%1 @ %2: Can not registr Hearbeat Port").arg(__LINE__).arg(__FILENAME__), ecSendDBEvents);
  }
  connect(UdpSocketOBU, SIGNAL(readyRead()), this, SLOT(fnOBUPacketReceived()));


  //cookies
  iResult = iResult;

  //send message about program start
  fnSendEmail(NULL, 0, QString("%1 @ %2: Program start").arg(__LINE__).arg(__FILENAME__), ecSendDBEvents);
  if (iDBResult >= 0)
  {
    fnSendEmail(NULL, 0, QString("%1 @ %2: Connected do DB").arg(__LINE__).arg(__FILENAME__), ecSendDBEvents);
  }
}

MainWindow::~MainWindow()
{

  if(boGeneralDebugOK)
  {
    GeneralDebugStream << DBGINFO << ";" << TIMEINFO << endl;
    GeneralDebugStream << "Program Stop" << endl;
    GeneralDebugStream.flush();
  }

  GeneralDebugFile.close();
  delete ui;
}











int MainWindow::fnConnectToDB(bool boForceExit)
{
  int iReturnValue = 0;

  TramDB.setDatabaseName(asDatabaseName);
  TramDB.setUserName(asUserName);
  TramDB.setPassword(asPassword);
  TramDB.setHostName(asHostName);
  bool boResult = TramDB.open();
  QSqlError TramDB_Error =  TramDB.lastError();

  qDebug() << DBGINFO << boResult;
  qDebug() << DBGINFO << TramDB_Error.databaseText()<< "/" <<TramDB_Error.driverText();
  qDebug() << DBGINFO << TramDB.hostName();


  if(!boResult)
  {

    fnSendEmail(NULL, 0, QString("%1 @ %2: To DefautRecipient-> Not Connected do DB").arg(__LINE__).arg(__FILENAME__), ecSendDBEvents);

    if (boForceExit)
    {
      QMessageBox::information(this, "Problem to connect to DB", TramDB_Error.databaseText() , QMessageBox::Ok);
      exit(-1);
      return(-1);
    }

    iNumberOfConnectAttempts++;
    LastDateTimeDisConnected = QDateTime::currentDateTimeUtc();
    ui->plainTextEdit_dBInfo->clear();
    ui->plainTextEdit_dBInfo->appendPlainText("DISCONNECTED");
    ui->plainTextEdit_dBInfo->appendPlainText(TramDB.databaseName());
    ui->plainTextEdit_dBInfo->appendPlainText(TramDB.hostName());
    ui->plainTextEdit_dBInfo->appendPlainText(QString("Number of con. attemps %1").arg(iNumberOfConnectAttempts));
    ui->plainTextEdit_dBInfo->appendPlainText(QString("Last Con: ") + LastDateTimeConnected.toString( Qt::SystemLocaleShortDate));

    iReturnValue = -2;
  }
  else
  {
    fnSendEmail(NULL, 0, QString("%1 @ %2: To DefautRecipient-> Connected do DB").arg(__LINE__).arg(__FILENAME__), ecSendDBEvents);

    iNumberOfConnectAttempts = 0;
    LastDateTimeConnected = QDateTime::currentDateTimeUtc();
    ui->plainTextEdit_dBInfo->clear();
    ui->plainTextEdit_dBInfo->appendPlainText("CONNECTED");
    ui->plainTextEdit_dBInfo->appendPlainText(TramDB.databaseName());
    ui->plainTextEdit_dBInfo->appendPlainText(TramDB.userName());
    ui->plainTextEdit_dBInfo->appendPlainText(TramDB.password());
    ui->plainTextEdit_dBInfo->appendPlainText(TramDB.hostName());
    ui->plainTextEdit_dBInfo->appendPlainText(QString("Number of con. attemps %1").arg(iNumberOfConnectAttempts));
    ui->plainTextEdit_dBInfo->appendPlainText(QString("Last Con: ") + LastDateTimeDisConnected.toString( Qt::SystemLocaleShortDate));
    iReturnValue = 0;
  }

  return(iReturnValue);

}


int MainWindow::fnTestConnection(void)
{
  int iReturnValue = 0;
  QSqlQuery TestConQuery;
  bool boResult;
  boResult = TestConQuery.exec("SELECT 1 FROM tVehicleList");
//  qDebug() << DBGINFO << boResult
//           << TestConQuery.lastError().number()
//           << TestConQuery.lastError()
//           << TestConQuery.lastError().type()
//           << TestConQuery.lastError().text();
  if(boResult)
  {
    ui->Lb_ConnStatus->setText("CONNECTED");
    iReturnValue = 0;
  }
  else
  {
    fnSendEmail(NULL, 0, QString("%1 @ %2: DISCONNECTED").arg(__LINE__).arg(__FILENAME__), ecSendDBEvents);
    ui->Lb_ConnStatus->setText("DISCONNECTED");
    ui->plainTextEdit_dBInfo->clear();
    ui->plainTextEdit_dBInfo->appendPlainText("DISCONNECTED");
    ui->plainTextEdit_dBInfo->appendPlainText(TramDB.databaseName());
    ui->plainTextEdit_dBInfo->appendPlainText(TramDB.hostName());
    iReturnValue = -1;
  }

#define MYSQL_SERVER_HAS_GONE_AWAY 2006

//  if(!boResult && TestConQuery.lastError() == MYSQL_SERVER_HAS_GONE_AWAY)
  if(!boResult)
  {
    fnConnectToDB();

  }
  return(iReturnValue);
}




#define SET_TABLE_TEXT(irow, icol, text)\
ui->tableWidget_QueryStatus->item(irow,icol)->setText(#text)

#define SET_TABLE_STRING(irow, icol, val)\
ui->tableWidget_QueryStatus->item(irow,icol)->setText(val)

#define SET_TABLE_INT(irow, icol, val)\
ui->tableWidget_QueryStatus->item(irow,icol)->setData(Qt::EditRole, val);


void MainWindow::on_Btn_FillVehicleList_fromDB_clicked()
{
  bool boResult;
  int iVehicleId;
  int iVehicleNumber;
  int iProjectId;
  int iReadInterval_Log;
  int iReadInterval_OpData;
  QString asIPAddr;
  QString asVehicleStatus;
  QHostAddress DstAddr;
  //refresh spam table
  mSpamModel->select();    //refresh spam model


  //clear old list
  foreach (TVehicle *tmpVehicle, VehicleList)
  {
    if (tmpVehicle != NULL)
    {
      //delete tmpQuerySchedulerItem;
      //tmpVehicle->deleteLater(); //TODO test it, must be Qbject!!
      tmpVehicle = NULL;
    }
  }

  VehicleList.clear();

  //and fill it again...

  //Debug code. Write info about last vehicle log state to file. Open the file
  QString asIndexDebugTestFileName = QFileInfo( QCoreApplication::applicationFilePath() ).filePath().section(".",0,0)+".indexDebug.txt";
  QFile IndexDebugTestFile(asIndexDebugTestFileName);
  IndexDebugTestFile.open(QIODevice::WriteOnly | QIODevice::Text);
  QTextStream IndexDebugTestStream(&IndexDebugTestFile);


  mVehicleModel->select(); //refresh vehicle model
  int iValidVehicles = 0;
  for (int i = 0; i < mVehicleModel->rowCount(); ++i)
  {

    iVehicleId           = mVehicleModel->data(mVehicleModel->index(i,col_VehicleList_idVehicle)).toInt(&boResult);
    iVehicleNumber       = mVehicleModel->data(mVehicleModel->index(i,col_VehicleList_VehicleNumber)).toInt(&boResult);
    iProjectId           = mVehicleModel->data(mVehicleModel->index(i,col_VehicleList_idProject)).toInt(&boResult);
    iReadInterval_Log    = mVehicleModel->data(mVehicleModel->index(i,col_VehicleList_ReadLogInterval)).toInt(&boResult);
    iReadInterval_OpData = mVehicleModel->data(mVehicleModel->index(i,col_VehicleList_ReadOpDataInterval)).toInt(&boResult);

    asIPAddr = mVehicleModel->data(mVehicleModel->index(i,col_VehicleList_IPV4)).toString();
    asVehicleStatus = mVehicleModel->data(mVehicleModel->index(i, col_VehicleList_VehicleStatus)).toString();
    asVehicleStatus = asVehicleStatus.trimmed();
    asVehicleStatus = asVehicleStatus.toUpper();

    qDebug() << DBGINFO << iVehicleId << asIPAddr << asVehicleStatus;

    //check ip addr format
    boResult = DstAddr.setAddress(asIPAddr);
    if(!boResult) asIPAddr.clear();
    if(asIPAddr.isEmpty()) asIPAddr = "";
    if (asIPAddr == "" || asVehicleStatus != "ACTIVE")
    {
      continue;
    }

    //**********************
    //*create item to query*
    //**********************
    TVehicle *tmpVehicle;
    tmpVehicle = new TVehicle(iVehicleId);
    tmpVehicle->asVehicleIP          = asIPAddr;
    tmpVehicle->dwVehicleIP          = QHostAddress(tmpVehicle->asVehicleIP).toIPv4Address();
    tmpVehicle->iProjectId           = iProjectId;
    tmpVehicle->iVehicleNumber       = iVehicleNumber;

    //add to list
    VehicleList.append(tmpVehicle);





    //**************************
    // find Project Name a Type
    //**************************
#define PROJ_TYPE_TRAM  "TRAM"
#define PROJ_TYPE_TROL  "TROL"
#define PROJ_TYPE_EBUS  "E-BUS"
#define PROJ_TYPE_OTHER "OTHER"

    qDebug() << DBGINFO << "A";

    QSqlQuery ProjectTableQuery;
    boResult = ProjectTableQuery.exec("SELECT `ProjectName`, `ProjectType`"
                                      "FROM `tProjectList`"
                                      "WHERE ("
                                      "`idProject` =" + QString::number(iProjectId) +
                                      ");"
                                      );


    if(boResult && ProjectTableQuery.size() == 1)
    {
      ProjectTableQuery.next();
      tmpVehicle->asProjectName = ProjectTableQuery.value(0).toString();
      QString asProjectType                = ProjectTableQuery.value(1).toString();

      if      (asProjectType == PROJ_TYPE_TRAM)  tmpVehicle->eProjectType = prjTRAM;
      else if (asProjectType == PROJ_TYPE_TROL)  tmpVehicle->eProjectType = prjTROL;
      else if (asProjectType == PROJ_TYPE_EBUS)  tmpVehicle->eProjectType = prjEBUS;
      else if (asProjectType == PROJ_TYPE_OTHER) tmpVehicle->eProjectType = prjOTHER;
      else                                       tmpVehicle->eProjectType = prjUNKNOWN;
    }
    else
    {
      tmpVehicle->asProjectName = "UNKNOWN";
    }

    qDebug() << DBGINFO << "B";

    if (!boResult)
    {
      fnSendEmail(tmpVehicle, 0, QString("%1 @ %2: %3 Error:\r\n\r\n %4").arg(__LINE__).arg(__FILENAME__).arg("tProjectList").arg(ProjectTableQuery.lastError().text()), ecSendDBEvents);
    }










  }
  //Debug code. Write info about last vehicle log state to file. Close file
  IndexDebugTestFile.close();


  fnSendEmail(NULL, 0, QString("%1 @ %2: Filled Vehicle Table, %3 Vehicles found").arg(__LINE__).arg(__FILENAME__).arg(mVehicleModel->rowCount()), ecSendDBEvents);


  //******************************
  //*Prepare table to show status*
  //******************************

  //delete old if exist
  for (int irow = 0; irow < ui->tableWidget_QueryStatus->rowCount(); ++irow)
  {
    for (int icol = 0; icol < ui->tableWidget_QueryStatus->columnCount(); ++icol)
    {
      if(ui->tableWidget_QueryStatus->item(irow, icol) != NULL)
      {
        delete ui->tableWidget_QueryStatus->item(irow, icol);
        ui->tableWidget_QueryStatus->setItem(irow, icol, NULL);
      }

    }
  }


  //create new
  ui->tableWidget_QueryStatus->setRowCount(VehicleList.count());
  //add Items to Table
  for (int irow = 0; irow < ui->tableWidget_QueryStatus->rowCount(); ++irow)
  {
    for (int icol = 0; icol < ui->tableWidget_QueryStatus->columnCount(); ++icol)
    {
      ui->tableWidget_QueryStatus->setItem(irow, icol, new QTableWidgetItem("cell text"));
    }
  }
}



#define col_infotable_vehicle_id         0
#define col_infotable_jamic_id           1
#define col_infotable_IPV4               2
#define col_infotable_timer              3
#define col_infotable_status             4
#define col_infotable_attempts           5
#define col_infotable_rows               6
#define col_infotable_when               7
#define col_infotable_newdata            8
#define col_infotable_timer_opdata       9
#define col_infotable_status_opdata     10
#define col_infotable_attempts_opdata   11
#define col_infotable_when_opdata       12
#define col_infotable_newdata_opdata    13
#define col_infotable_newdata_versions  14
#define col_infotable_hb_rqe            15

void MainWindow::timerEvent(QTimerEvent * event)
{
  if (event->timerId() == iMainTimerID)
  {
    //qDebug()<< DBGINFO << "MAIN" << MAIN_TIMER_PERIOD;

  }



  if (event->timerId() == iAuxTimerID)
  {
    static bool boWasConnected = false;

    //qDebug() << DBGINFO << "AUX" << AUX_TIMER_PERIOD << boWasConnected;

    //check connection before reading from JAMIC...
    if(fnTestConnection() < 0)
    {
      qDebug() << DBGINFO << "AUX" << "NOT CONNECTED TO DB";
      boWasConnected = false;
      return;
    }

    //check changes in vehicle table
    if(fnTestVehicleChanged() > 0)
    {
      fnSendEmail(NULL, 0, QString("%1 @ %2: Change in table tVehicleList detected").arg(__LINE__).arg(__FILENAME__), ecSendNewVehicle);
      on_Btn_FillVehicleList_fromDB_clicked();
    }

    //fill vehicle table from database
    if (!boWasConnected)
    {
      boWasConnected = true;
      on_Btn_FillVehicleList_fromDB_clicked();
    }

    if (boForceFillVehicleFromDB)
    {
      boForceFillVehicleFromDB = false;
      on_Btn_FillVehicleList_fromDB_clicked();
    }


    //Send statistical message every day
    //TODO this alghoritm does work well around the midnight.
    {
     #define SENT_STAT_EMAIL_AT_HOUR  7
     //#define SENT_STAT_EMAIL_AT_MS    (int)(SENT_STAT_EMAIL_AT_HOUR * 60 * 60 *1000)


      static bool boAlreadySentToday = false;
      static QDate LastSendDate = QDate::currentDate();
      int iTimeToSendMS = QTime::currentTime().msecsTo(QTime(SENT_STAT_EMAIL_AT_HOUR, 0));
      if (iTimeToSendMS < 0 && !boAlreadySentToday)
      {
        fnSendEmail(NULL, 0, QString("%1 @ %2: Periodic report").arg(__LINE__).arg(__FILENAME__), ecPeriodicalReport, "Periodic rep.");
        LastSendDate = QDate::currentDate();
        boAlreadySentToday = true;
      }

      if (QDate::currentDate() > LastSendDate)
      {
        boAlreadySentToday = false;
      }

    }

    //Call each Item Timer

    int iRow = 0;
    foreach (TVehicle *tmpVehicle, VehicleList)
    {

        //update info table
        SET_TABLE_INT    (iRow, col_infotable_vehicle_id,      tmpVehicle->iVehicleId);

        SET_TABLE_STRING (iRow, col_infotable_IPV4,            tmpVehicle->asVehicleIP);

        SET_TABLE_TEXT   (iRow, col_infotable_hb_rqe, CLICK);

      iRow++;
    }
  }
}



void MainWindow::on_btn_Debug1_clicked()
{
  //debug to send request
  if(VehicleList.isEmpty()) return;
  TVehicle *tmpVehicle = VehicleList.at(0);
  if(tmpVehicle == NULL) return;

}


void MainWindow::on_tableView_Vehicles_clicked(const QModelIndex &index)
{
  bool boResult = false;
  //cookie
  boResult = boResult;
  qDebug() << DBGINFO << index.row() << " " << index.column();
  int iVehicleId = mVehicleModel->data(mVehicleModel->index(index.row(), mVehicleModel->fieldIndex("idVehicle"))).toInt(&boResult);
  if (boResult)
  {
    iLastSelectedVehicleForStatistic = iVehicleId;
  }
  else
  {
    iLastSelectedVehicleForStatistic = -1;
  }

  int iProjectId = mVehicleModel->data(mVehicleModel->index(index.row(), mVehicleModel->fieldIndex("tProjectList_idProject"))).toInt(&boResult);
  if (boResult)
  {
    iLastSelectedProjectForStatistic = iProjectId;
  }
  else
  {
    iLastSelectedProjectForStatistic = -1;
  }


  //qDebug() << DBGINFO << boResult  << iLastSelectedVehicleForStatistic << iLastSelectedProjectForStatistic;
}


//for debuging purposes. When clicek, then tick timer is cleared
void MainWindow::on_tableWidget_QueryStatus_clicked(const QModelIndex &index)
{
  int iVehicleId;
  bool boResult = false;
  //cookie
  boResult = boResult;
  iVehicleId = ui->tableWidget_QueryStatus->item(index.row(), col_infotable_vehicle_id)->data(Qt::DisplayRole).toInt();
  qDebug() << DBGINFO << index.row() << " " << index.column() << iVehicleId;
  if(iVehicleId >= 0)
  {
    //search for Vehicle id VehicleList Items
    foreach (TVehicle *tmpVehicle, VehicleList)
    {
      if (tmpVehicle != NULL)
      {
        if (tmpVehicle->iVehicleId == iVehicleId)
        {
          switch (index.column())
          {
            case col_infotable_vehicle_id:

            break;

            case col_infotable_IPV4:

            break;

            case col_infotable_timer:

            break;

            case col_infotable_status:

            break;

            case col_infotable_attempts:

            break;

            case col_infotable_when:

            break;

            case col_infotable_rows:

            break;

            case col_infotable_newdata:

            break;

            case col_infotable_timer_opdata:

            break;

            case col_infotable_status_opdata:

            break;

            case col_infotable_attempts_opdata:

            break;

            case col_infotable_when_opdata:

            break;

            case col_infotable_newdata_opdata:

            break;

            case col_infotable_newdata_versions:

            break;

            case col_infotable_hb_rqe:

            break;

            default:

            break;
          }


        }
      }

    }
  }
}












int MainWindow::fnTestVehicleChanged(void)
{
  bool boResult;
  int iFlag;
  int iReturnValue = 0;
  QSqlQuery FlagTableQuerySelect;
  FlagTableQuerySelect.prepare("SELECT `FlagValue` FROM `tFlags` WHERE `FlagName` = 'VEHICLE_TABLE_CHANGED';");
  boResult = FlagTableQuerySelect.exec();
  //qDebug() << DBGINFO << "Flag table select" << boResult;
  if(boResult && FlagTableQuerySelect.size() == 1)
  {
    //Test Flag denoting change in Vehicle table
    FlagTableQuerySelect.next();
    iFlag = FlagTableQuerySelect.value(0).toInt(&boResult);
    if (boResult && iFlag == 1)
    {
      //Flag set - indicate change
      iReturnValue = 1;
      //Clear the flag
      FlagTableQuerySelect.prepare("UPDATE `tFlags` SET `FlagValue` = '0' WHERE `FlagName` = 'VEHICLE_TABLE_CHANGED';");
      boResult = FlagTableQuerySelect.exec();
      qDebug() << DBGINFO << "Flag table update" << boResult;
    }

  }

  return (iReturnValue);
}



//***************
//* HTTP SERVER *
//***************
void MainWindow::fnAcceptHTTPConnection(void)
{
  QTcpSocket * socket = httpServer->nextPendingConnection();
  qDebug() << DBGINFO << "fnAcceptHTTPConnection" << socket;
  connect(socket, SIGNAL(readyRead()), this, SLOT(fnReadFromSocket()));
  connect(socket, SIGNAL(disconnected()), this, SLOT(fnClosingClient()));
}


void  MainWindow::fnReadFromSocket(void)
{

  bool boResult;
  static int iSocketCounter = 0;
  static int iButtonCounter = 0;
  QString asOSName;
  QString asTemp;
  QString asBtnReset_Log;
  QString asBtnReset_OpData;

  qlonglong iFreeMemory = -1;
  qlonglong static iFreeMemoryFirstPass = -1;
  QByteArray ReadData;
  QTcpSocket *SenderSocket = (QTcpSocket *) sender();
  qDebug() << DBGINFO << "fnReadFromSocket" << SenderSocket;
  iSocketCounter++;

  ReadData = SenderSocket->read(1000);
  QByteArray TestHTTP = ReadData.left(6);
  qDebug() << DBGINFO << iSocketCounter;
  qDebug() << DBGINFO << ReadData <<  ReadData.size();

  //check HTTP acces
  if(TestHTTP.size() < 5 || (!TestHTTP.contains("GET /") && !TestHTTP.contains("POST /")))
  {
    SenderSocket->disconnectFromHost();
    return;
  }

  //check button pressed
  if(ReadData.contains("cmdRefreshVehicleTable"))
  {
    iButtonCounter++;
    boForceFillVehicleFromDB = true;
    SenderSocket->disconnectFromHost();
    return;
  }

  //check timer reset

  if(ReadData.contains("cmdResetTimer_Log_"))
  {
    iResetTimerReq_Log = -1;
    QByteArray cmdText = ReadData;
    int cmdPos = cmdText.indexOf("cmdResetTimer_Log_");
#define CMD_RESET_TIMER_LEN_LOG 18
    cmdText.remove(0, cmdPos+CMD_RESET_TIMER_LEN_LOG);
    cmdPos = cmdText.indexOf("_");
    cmdText.remove(cmdPos, cmdText.size());


    iResetTimerReq_Log = cmdText.toInt(&boResult);
    if(!boResult) iResetTimerReq_Log = -1;
    if(iResetTimerReq_Log < 0) iResetTimerReq_Log = -1;
    SenderSocket->disconnectFromHost();
    ui->lbDebug1->setText(QString::number(iResetTimerReq_Log));
    return;
  }

  if(ReadData.contains("cmdResetTimer_OpData_"))
  {
    iResetTimerReq_OpData = -1;
    QByteArray cmdText = ReadData;
    int cmdPos = cmdText.indexOf("cmdResetTimer_OpData_");
#define CMD_RESET_TIMER_LEN_OPDATA 21
    cmdText.remove(0, cmdPos+CMD_RESET_TIMER_LEN_OPDATA);
    cmdPos = cmdText.indexOf("_");
    cmdText.remove(cmdPos, cmdText.size());


    iResetTimerReq_OpData = cmdText.toInt(&boResult);
    if(!boResult) iResetTimerReq_OpData = -1;
    if(iResetTimerReq_OpData < 0) iResetTimerReq_OpData = -1;
    SenderSocket->disconnectFromHost();
    ui->lbDebug1->setText(QString::number(iResetTimerReq_OpData));
    return;
  }





  SenderSocket->write("HTTP/1.1 200 OK\r\n");       // \r needs to be before \n
  SenderSocket->write("Content-Type: text/html\r\n");
  SenderSocket->write("Connection: close\r\n");
  SenderSocket->write("Refresh: 3\r\n");     //refreshes web browser three every second. Require two \r\n.
  SenderSocket->write("Pragma: no-cache\r\n");
  SenderSocket->write("\r\n");
  SenderSocket->write("<!DOCTYPE html>\r\n");
  SenderSocket->write("<html><body>ReadLogFromVehicle status: <BR>");
  SenderSocket->write("Version: ");
  SenderSocket->write(QApplication::applicationVersion().toLatin1());
  SenderSocket->write("<BR>");
  SenderSocket->write(ui->Lb_ConnStatus->text().toLatin1());
  SenderSocket->write("<BR>");
  SenderSocket->write("Connected from: ");
  SenderSocket->write(LastDateTimeConnected.toString( Qt::SystemLocaleShortDate).toLatin1());
  SenderSocket->write("<BR>");
  SenderSocket->write("Disconnected from: ");
  SenderSocket->write(LastDateTimeDisConnected.toString( Qt::SystemLocaleShortDate).toLatin1());
  SenderSocket->write("<BR>");
  SenderSocket->write(QString("Counter: %1").arg(iSocketCounter).toLatin1());
  SenderSocket->write("<BR>");
  SenderSocket->write("<BR>");



  //Get free memory status
#if defined(Q_OS_WIN)
    asOSName = "windows";
    MEMORYSTATUSEX MemInfo = { sizeof(MemInfo),0,0,0,0,0,0,0,0 };
    GlobalMemoryStatusEx(&MemInfo);
    iFreeMemory = MemInfo.ullAvailPhys;
    if(iFreeMemory >= 0)
    {
      iFreeMemory /= (1024 * 1024);
    }

#elif defined(Q_OS_LINUX)
    asOSName = "linux";
    QProcess p;
    p.start("awk", QStringList() << "/MemFree/ { print $2 }" << "/proc/meminfo");
    p.waitForFinished();
    QString memory = p.readAllStandardOutput();
    p.close();
    memory = memory.trimmed();
    iFreeMemory = memory.toInt(&boResult);
    if(!boResult) iFreeMemory = -1;
    if(iFreeMemory >= 0)
    {
      iFreeMemory /= 1024;
    }

#else
    asOSName = "unknown";
    iFreeMemory = -1;

#endif


  if((iFreeMemoryFirstPass == -1) && (iFreeMemory != -1))
  {
    iFreeMemoryFirstPass = iFreeMemory;
  }

  if(iFreeMemory != -1)
  {
    asTemp = QString("Running under %1 <BR> Free memory %2 MB <BR> Memory difference %3 MB <BR><BR>")
            .arg(asOSName)
            .arg(iFreeMemory)
            .arg(iFreeMemory - iFreeMemoryFirstPass);
    SenderSocket->write(asTemp.toLatin1());
  }



  //Button and JavaScrtipt..
  asTemp =
       "<button type=\"button\" onclick=\"proceed()\">Refresh Vehicle Table</button>"
       "<script>"
       "function proceed() {"
         "var xhttp = new XMLHttpRequest();"
         "xhttp.open(\"POST\", \"\", true);"
         "xhttp.send(\"cmdRefreshVehicleTable\");"
       "}"
       "</script>";

  SenderSocket->write(asTemp.toLatin1());
  SenderSocket->write("<BR>");



  //table
  SenderSocket->write("<table border=\"1\" cellpadding=\"5\">");

  //Headers
  SenderSocket->write("<tr>");

  SenderSocket->write("<th>ID</th>");
  SenderSocket->write("<th>Number</th>");
  SenderSocket->write("<th>Project</th>");
  SenderSocket->write("<th>IPV4</th>");
  SenderSocket->write("<th>JAMIC <BR> ID</th>");
  SenderSocket->write("<th>RESET <BR> TIMER</th>");
  SenderSocket->write("<th>TIMER <BR> [s]</th>");
  SenderSocket->write("<th>STATUS</th>");
  SenderSocket->write("<th>ATTEMPTS</th>");
  SenderSocket->write("<th>DEF <BR> DATE <BR> (LOCAL)</th>");
  SenderSocket->write("<th>DEF <BR> TIME <BR> (LOCAL)</th>");
  SenderSocket->write("<th>ROWS <BR> READ </th>");
  SenderSocket->write("<th>WHEN <BR>(UTC)</th>");
  SenderSocket->write("<th>ROWS <BR> WRITTEN </th>");
  SenderSocket->write("<th>WHEN <BR>(UTC)</th>");
  SenderSocket->write("<th>WAIT <BR> DATA</th>");
  SenderSocket->write("<th>RESET <BR> TIMER</th>");
  SenderSocket->write("<th>TIMER <BR> [s]</th>");
  SenderSocket->write("<th>STATUS</th>");
  SenderSocket->write("<th>ATTEMPTS</th>");
  SenderSocket->write("<th>WHEN <BR> UTC </th>");
  SenderSocket->write("<th>WAIT <BR> DATA OP</th>");
  SenderSocket->write("<th>WAIT <BR> DATA VE</th>");

  SenderSocket->write("</tr>");



  foreach (TVehicle *tmpVehicle, VehicleList)
  {
    if (tmpVehicle != NULL)
    {

      asBtnReset_Log =
             QString("<button type=\"button\" onclick=\"proceed_Log%1()\">R</button>"
             "<script>"
             "function proceed_Log%1() {"
               "var xhttp = new XMLHttpRequest();"
               "xhttp.open(\"POST\", \"\", true);"
               "xhttp.send(\"cmdResetTimer_Log_%1_\");"
             "}"
             "</script>").arg(tmpVehicle->iVehicleId);

      asBtnReset_OpData =
             QString("<button type=\"button\" onclick=\"proceed_OpData%1()\">R</button>"
             "<script>"
             "function proceed_OpData%1() {"
               "var xhttp = new XMLHttpRequest();"
               "xhttp.open(\"POST\", \"\", true);"
               "xhttp.send(\"cmdResetTimer_OpData_%1_\");"
             "}"
             "</script>").arg(tmpVehicle->iVehicleId);









      SenderSocket->write("<tr>");

      SenderSocket->write("<td>");
      SenderSocket->write(QString("%1").arg(tmpVehicle->iVehicleId).toLatin1());
      SenderSocket->write("</td>");

      SenderSocket->write("<td>");
      SenderSocket->write(QString("%1").arg(tmpVehicle->iVehicleNumber).toLatin1());
      SenderSocket->write("</td>");

      SenderSocket->write("<td>");
      SenderSocket->write(QString("%1").arg(tmpVehicle->asProjectName).toLatin1());
      SenderSocket->write("</td>");

      SenderSocket->write("<td>");
      SenderSocket->write(QString("%1").arg(tmpVehicle->asVehicleIP).toLatin1());
      SenderSocket->write("</td>");

      SenderSocket->write("<td>");
      SenderSocket->write("<div style=\"font-family:Courier New;\">");
      SenderSocket->write(QString(" ").toLatin1());
      SenderSocket->write("</div>");
      SenderSocket->write("</td>");

      SenderSocket->write("<td>");
      SenderSocket->write(asBtnReset_Log.toLatin1());
      SenderSocket->write("</td>");

      SenderSocket->write("<td>");
      SenderSocket->write(QString(" ").toLatin1());
      SenderSocket->write("</td>");

      SenderSocket->write("<td>");
      SenderSocket->write(QString(" ").toLatin1());
      SenderSocket->write("</td>");

      SenderSocket->write("<td>");
      SenderSocket->write(QString(" ").toLatin1());
      SenderSocket->write("</td>");

      SenderSocket->write("<td>");
      SenderSocket->write(QString(" ").toLatin1());
      SenderSocket->write("</td>");

      SenderSocket->write("<td>");
      SenderSocket->write(QString(" ").toLatin1());
      SenderSocket->write("</td>");

      SenderSocket->write("<td>");
      SenderSocket->write(QString(" ").toLatin1());
      SenderSocket->write("</td>");

      SenderSocket->write("<td>");
      SenderSocket->write(QString(" ").toLatin1());
      SenderSocket->write("</td>");

      SenderSocket->write("<td>");
      SenderSocket->write(QString(" ").toLatin1());
      SenderSocket->write("</td>");

      SenderSocket->write("<td>");
      SenderSocket->write(QString(" ").toLatin1());
      SenderSocket->write("</td>");

      SenderSocket->write("<td>");
      SenderSocket->write(QString(" ").toLatin1());
      SenderSocket->write("</td>");

      SenderSocket->write("<td>");
      SenderSocket->write(asBtnReset_OpData.toLatin1());
      SenderSocket->write("</td>");

      SenderSocket->write("<td>");
      SenderSocket->write(QString(" ").toLatin1());
      SenderSocket->write("</td>");


      SenderSocket->write("<td>");
      SenderSocket->write(QString(" ").toLatin1());
      SenderSocket->write("</td>");


      SenderSocket->write("<td>");
      SenderSocket->write(QString(" ").toLatin1());
      SenderSocket->write("</td>");

      SenderSocket->write("<td>");
      SenderSocket->write(QString(" ").toLatin1());
      SenderSocket->write("</td>");

      SenderSocket->write("<td>");
      SenderSocket->write(QString(" ").toLatin1());
      SenderSocket->write("</td>");

      SenderSocket->write("<td>");
      SenderSocket->write(QString(" ").toLatin1());
      SenderSocket->write("</td>");




      SenderSocket->write("</tr>");
    }
    //cookies
    boResult = boResult;
  }




  SenderSocket->write("</table>");
  SenderSocket->write("<BR>");

  asTemp =         "Status:<BR>"
                   "0... INIT<BR>"
                   "1... READY<BR>"
                   "2... WAIT FOR NEW QUERY<BR>"
                   "3... WAIT FOR NEW ATTEMPT<BR>";
  SenderSocket->write(asTemp.toLatin1());



  SenderSocket->write("</body></html>");
  SenderSocket->flush();
  SenderSocket->disconnectFromHost();

}


void  MainWindow::fnClosingClient(void)
{
    QTcpSocket *SenderSocket = (QTcpSocket *) sender();
    qDebug() << DBGINFO << "fnClosingClient"  << SenderSocket;
    SenderSocket->deleteLater();
}



//*********************
//* HTTP SERVER - END *
//*********************


//***************************
//* EMAIL CLIENT TEST       *
//***************************
void MainWindow::on_Btn_SendTestEmail_clicked()
{
  TeEmailCategory eMailCategory = ecSendDBEvents;   //debug, will be function parameter
  fnSendEmail(NULL, 0, QString("%1 @ %2: Test Button Pressed").arg(__LINE__).arg(__FILENAME__), eMailCategory);

}

void MainWindow::fnSendEmail(TVehicle *tmpVehicle, int iInfo, QString asEmailMessageToSend, TeEmailCategory eMailCategory, QString asAddToSubject)
{

  int iVehicleProjectId = -1;


  if(!boSMTPSettingsComplete)
  {
    //no valid smtp setting
    return;
  }


  bool boSpamTableOK = true;
  if (
       (col_SpamList_idSpamList < 0)         ||
       (col_SpamList_tUsers_idUser < 0)      ||
       (col_SpamList_EmailAddress < 0)       ||
       (col_SpamList_boSendDBEvents < 0)     ||
       (col_SpamList_boSendNewVehicle < 0)   ||
       (col_SpamList_boReadVehicle < 0)      ||
       (col_SpamList_boWriteLogVehicle < 0)  ||
       (col_SpamList_boPeriodicalReport < 0)


     )
  {
    boSpamTableOK = false;
  }


  QList <EmailAddress *> EmailRecipientList;
  bool boSendDBEvents = false;
  bool boSendNewVehicle = false;
  bool boReadVehicle = false;
  bool boWriteLogVehicle = false;
  bool boPeriodicalReport = false;
  int  iRecipientProjectId = -1;
  EmailRecipientList.clear();
  if(boSpamTableOK)
  {
    bool boResult = mSpamModel->select(); //refresh table
    if (boResult)
    {
      for (int i = 0; i < mSpamModel->rowCount(); ++i)
      {

        bool boResult = false;
        QVariant tmpVariant;
        QString asTempEmailAddress = mSpamModel->data(mSpamModel->index(i, col_SpamList_EmailAddress)).toString();
        boSendDBEvents             = mSpamModel->data(mSpamModel->index(i, col_SpamList_boSendDBEvents)).toBool();
        boSendNewVehicle           = mSpamModel->data(mSpamModel->index(i, col_SpamList_boSendNewVehicle)).toBool();
        boReadVehicle              = mSpamModel->data(mSpamModel->index(i, col_SpamList_boReadVehicle)).toBool();
        boWriteLogVehicle          = mSpamModel->data(mSpamModel->index(i, col_SpamList_boWriteLogVehicle)).toBool();
        boPeriodicalReport         = mSpamModel->data(mSpamModel->index(i, col_SpamList_boPeriodicalReport)).toBool();
        tmpVariant                 = mSpamModel->data(mSpamModel->index(i, col_SpamList_tProjectList_idProject));
        if (!tmpVariant.isNull())
        {
          iRecipientProjectId     = tmpVariant.toInt(&boResult);
        }
        else
        {
          boResult = false;
        }
        if(!boResult) iRecipientProjectId = -1;

        qDebug() << asTempEmailAddress;
        if(
            (
              (eMailCategory == ecSendDBEvents      && boSendDBEvents)
              ||
              (eMailCategory == ecSendNewVehicle    && boSendNewVehicle)
              ||
              (eMailCategory == ecReadVehicle       && boReadVehicle)
              ||
              (eMailCategory == ecWriteLogVehicle   && boWriteLogVehicle)
              ||
              (eMailCategory == ecPeriodicalReport  && boPeriodicalReport)
            )
            &&
            (
              (iVehicleProjectId <= 0)
              ||
              (iRecipientProjectId <= 0)
              ||
              (iVehicleProjectId == iRecipientProjectId)
            )
          )
        {
          EmailRecipientList.append(new EmailAddress(asTempEmailAddress));
        }
      }
    }
    else
    {
      EmailRecipientList.append(new EmailAddress(asDefautRecipient));
    }
  }
  else
  {
    EmailRecipientList.append(new EmailAddress(asDefautRecipient));
  }

  if (EmailRecipientList.isEmpty())
  {
    //nothing to send
    return;
  }

  SmtpClient   InfoSmtp(asSMTPServer, iSMTPPort, boSMTPSsl ? SmtpClient::SslConnection : SmtpClient::TcpConnection );
  EmailAddress InfoMessageSender(asSMTPSenderAddress);

  MimeMessage  InfoMessage;
  InfoMessage.setSender(&InfoMessageSender);

  //Send to sending address for diagnostic purpose
  InfoMessage.addRecipient(&InfoMessageSender);

  foreach (EmailAddress *RecipientEmailAdress, EmailRecipientList)
  {
     InfoMessage.addBcc(RecipientEmailAdress);
  }


  //subject
  InfoMessage.setSubject("Message from ReadLogFromVehicles_T1. Do not reply " + asAddToSubject);

  //Text
  //msg introduction
  QString asIntroductionText(
                               QString("Generated by ReadLogFromVehicles_T1 ")
                             + QApplication::applicationVersion() +"\n\r "
                             + QDateTime::currentDateTimeUtc().toString() + "\n\r"
                             + "from: " +  asDatabaseName + " @ " + asHostName + "\n\r"
                             + QString("VehPrjId: %1").arg(iVehicleProjectId)
                            );

  if (tmpVehicle != NULL)
  {
    asIntroductionText += QString("\n\rVehicle id: %1").arg(tmpVehicle->iVehicleId);
  }

  asIntroductionText += "\r\n";
  asIntroductionText += asEmailMessageToSend;
  MimeText InfoMessageIntroductionText;
  InfoMessageIntroductionText.setText(asIntroductionText);
  InfoMessage.addPart(&InfoMessageIntroductionText);


//  //body
//  MimeText InfoMessageText;
//  InfoMessageText.setText(asEmailMessageToSend);
//  InfoMessage.addPart(&InfoMessageText);


  MimeHtml InfoMessageHtml;
  if (eMailCategory == ecPeriodicalReport)
  {
    //html text

    InfoMessageHtml.setHtml(fnAssembleHTMLReport());
    InfoMessage.addPart(&InfoMessageHtml);
  }

//goto LabelfnSendEmailEnd;

  if (!InfoSmtp.connectToHost())
  {

    //QMessageBox::information(this, "Connection Failed", "E-mail will not be sent", QMessageBox::Ok);
    qDeleteAll(EmailRecipientList);
    EmailRecipientList.clear();
    return;
  }

  if (boSMTPAuth)
  {
    if (InfoSmtp.login(asSMTPUser,asSMTPPass))
    {
      //QMessageBox::information(this, "Authentification Failed", "E-mail will not be sent", QMessageBox::Ok);
      qDeleteAll(EmailRecipientList);
      EmailRecipientList.clear();
      return;
    }
  }


  if (!InfoSmtp.sendMail(InfoMessage))
  {
    //QMessageBox::information(this, "Mail sending failed", "E-mail will not be sent", QMessageBox::Ok);
    qDeleteAll(EmailRecipientList);
    EmailRecipientList.clear();
    return;
  }

//LabelfnSendEmailEnd:

  qDeleteAll(EmailRecipientList);
  EmailRecipientList.clear();

  iInfo = iInfo;

}





/*
void MainWindow::on_Btn_SendTestEmail_clicked()
{
  SmtpClient InfoSmtp("10.175.62.17");
  EmailAddress *InfoMessageSender     = new EmailAddress("svoboda@cegelec.cz");
  EmailAddress *InfoMessageRecipient =  new EmailAddress("svoboda@cegelec.cz");

  MimeMessage InfoMessage;
  InfoMessage.setSender(InfoMessageSender);
  InfoMessage.addRecipient(InfoMessageRecipient);
  InfoMessage.setSubject("Test Message from ReadLogFromVehicles");


  if (!InfoSmtp.connectToHost())
  {

    QMessageBox::information(this, "Connection Failed", "E-mail will not be sent", QMessageBox::Ok);
    return;
  }

  if (!InfoSmtp.sendMail(InfoMessage))
  {
    QMessageBox::information(this, "Mail sending failed", "E-mail will not be sent", QMessageBox::Ok);
    return;
  }

}
*/


QString MainWindow::fnAssembleHTMLReport(void)
{
#define ADD_TO_HTML_RETURN_TEXT(TextToAdd)  asReturnText+=QString(TextToAdd)
#define ADD_TO_HTML_NEW_LINE                asReturnText+="\n"

  QString asReturnText;
  QString asOSName;
  QString asTemp;
  asReturnText.clear();

  qlonglong iFreeMemory = -1;
  qlonglong static iFreeMemoryFirstPass = -1;




  ADD_TO_HTML_RETURN_TEXT ("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">");

  ADD_TO_HTML_NEW_LINE;

  ADD_TO_HTML_RETURN_TEXT("<html>");
  ADD_TO_HTML_RETURN_TEXT("<head><meta name=\"qrichtext\" content=\"1\" />");
  ADD_TO_HTML_RETURN_TEXT("<style type=\"text/css\">");
  ADD_TO_HTML_RETURN_TEXT("p, li { white-space: pre-wrap; }");
  ADD_TO_HTML_RETURN_TEXT("</style>");
  ADD_TO_HTML_RETURN_TEXT("</head>");
  ADD_TO_HTML_NEW_LINE;
  ADD_TO_HTML_RETURN_TEXT("<body style=\" font-family:'MS Shell Dlg 2'; font-size:8.25pt; font-weight:400; font-style:normal;\">");
  ADD_TO_HTML_RETURN_TEXT("<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">");

  ADD_TO_HTML_NEW_LINE;

  ADD_TO_HTML_RETURN_TEXT("ReadLogFromVehicle status: <BR>");
  ADD_TO_HTML_RETURN_TEXT("Version: ");
  ADD_TO_HTML_RETURN_TEXT(QApplication::applicationVersion().toLatin1());
  ADD_TO_HTML_RETURN_TEXT("<BR>");
  ADD_TO_HTML_RETURN_TEXT(ui->Lb_ConnStatus->text().toLatin1());
  ADD_TO_HTML_RETURN_TEXT("<BR>");
  ADD_TO_HTML_RETURN_TEXT("Connected from: ");
  ADD_TO_HTML_RETURN_TEXT(LastDateTimeConnected.toString( Qt::SystemLocaleShortDate).toLatin1());
  ADD_TO_HTML_RETURN_TEXT("<BR>");
  ADD_TO_HTML_RETURN_TEXT("Disconnected from: ");
  ADD_TO_HTML_RETURN_TEXT(LastDateTimeDisConnected.toString( Qt::SystemLocaleShortDate).toLatin1());
  ADD_TO_HTML_RETURN_TEXT("<BR>");
  ADD_TO_HTML_RETURN_TEXT("<BR>");
  ADD_TO_HTML_RETURN_TEXT("<BR>");

  ADD_TO_HTML_NEW_LINE;

  //Get free memory status
#if defined(Q_OS_WIN)
    asOSName = "windows";
    MEMORYSTATUSEX MemInfo = { sizeof(MemInfo),0,0,0,0,0,0,0,0 };
    GlobalMemoryStatusEx(&MemInfo);
    iFreeMemory = MemInfo.ullAvailPhys;
    if(iFreeMemory >= 0)
    {
      iFreeMemory /= (1024 * 1024);
    }

#elif defined(Q_OS_LINUX)
    asOSName = "linux";
    QProcess p;
    p.start("awk", QStringList() << "/MemFree/ { print $2 }" << "/proc/meminfo");
    p.waitForFinished();
    QString memory = p.readAllStandardOutput();
    p.close();
    memory = memory.trimmed();
    bool boResult;
    iFreeMemory = memory.toInt(&boResult);
    if(!boResult) iFreeMemory = -1;
    if(iFreeMemory >= 0)
    {
      iFreeMemory /= 1024;
    }

#else
    asOSName = "unknown";
    iFreeMemory = -1;

#endif


  if((iFreeMemoryFirstPass == -1) && (iFreeMemory != -1))
  {
    iFreeMemoryFirstPass = iFreeMemory;
  }

  if(iFreeMemory != -1)
  {
    asTemp = QString("Running under %1 <BR> Free memory %2 MB <BR> Memory difference %3 MB <BR><BR>")
            .arg(asOSName)
            .arg(iFreeMemory)
            .arg(iFreeMemory - iFreeMemoryFirstPass);
    ADD_TO_HTML_RETURN_TEXT(asTemp.toLatin1());
  }


  ADD_TO_HTML_RETURN_TEXT("<BR>");

  //table
  ADD_TO_HTML_RETURN_TEXT("<table border=\"1\" cellpadding=\"5\">");

  //Headers
  ADD_TO_HTML_RETURN_TEXT("<tr>");

  ADD_TO_HTML_RETURN_TEXT("<th>ID</th>");
  ADD_TO_HTML_RETURN_TEXT("<th>Number</th>");
  ADD_TO_HTML_RETURN_TEXT("<th>Project</th>");
  ADD_TO_HTML_RETURN_TEXT("<th>IPV4</th>");
  ADD_TO_HTML_RETURN_TEXT("<th>JAMIC <BR> ID</th>");
  ADD_TO_HTML_RETURN_TEXT("<th>RESET <BR> TIMER</th>");
  ADD_TO_HTML_RETURN_TEXT("<th>TIMER <BR> [s]</th>");
  ADD_TO_HTML_RETURN_TEXT("<th>STATUS</th>");
  ADD_TO_HTML_RETURN_TEXT("<th>ATTEMPTS</th>");
  ADD_TO_HTML_RETURN_TEXT("<th>DEF <BR> DATE <BR> (LOCAL)</th>");
  ADD_TO_HTML_RETURN_TEXT("<th>DEF <BR> TIME <BR> (LOCAL)</th>");
  ADD_TO_HTML_RETURN_TEXT("<th>ROWS <BR> READ </th>");
  ADD_TO_HTML_RETURN_TEXT("<th>WHEN <BR>(UTC)</th>");
  ADD_TO_HTML_RETURN_TEXT("<th>ROWS <BR> WRITTEN </th>");
  ADD_TO_HTML_RETURN_TEXT("<th>WHEN <BR>(UTC)</th>");
  ADD_TO_HTML_RETURN_TEXT("<th>WAIT <BR> DATA</th>");
  ADD_TO_HTML_RETURN_TEXT("<th>RESET <BR> TIMER</th>");
  ADD_TO_HTML_RETURN_TEXT("<th>TIMER <BR> [s]</th>");
  ADD_TO_HTML_RETURN_TEXT("<th>STATUS</th>");
  ADD_TO_HTML_RETURN_TEXT("<th>ATTEMPTS</th>");
  ADD_TO_HTML_RETURN_TEXT("<th>WHEN <BR> UTC </th>");
  ADD_TO_HTML_RETURN_TEXT("<th>WAIT <BR> DATA OP</th>");
  ADD_TO_HTML_RETURN_TEXT("<th>WAIT <BR> DATA VE</th>");

  ADD_TO_HTML_RETURN_TEXT("</tr>");


  ADD_TO_HTML_NEW_LINE;
  foreach (TVehicle *tmpVehicle, VehicleList)
  {
    ADD_TO_HTML_NEW_LINE;

    if (tmpVehicle != NULL)
    {

      QString asBtnReset_Log = "x";
      QString asBtnReset_OpData = "x";

      ADD_TO_HTML_RETURN_TEXT("<tr>");

      ADD_TO_HTML_RETURN_TEXT("<td>");
      ADD_TO_HTML_RETURN_TEXT(QString("%1").arg(tmpVehicle->iVehicleId).toLatin1());
      ADD_TO_HTML_RETURN_TEXT("</td>");

      ADD_TO_HTML_RETURN_TEXT("<td>");
      ADD_TO_HTML_RETURN_TEXT(QString("%1").arg(tmpVehicle->iVehicleNumber).toLatin1());
      ADD_TO_HTML_RETURN_TEXT("</td>");

      ADD_TO_HTML_RETURN_TEXT("<td>");
      ADD_TO_HTML_RETURN_TEXT(QString("%1").arg(tmpVehicle->asProjectName).toLatin1());
      ADD_TO_HTML_RETURN_TEXT("</td>");

      ADD_TO_HTML_RETURN_TEXT("<td>");
      ADD_TO_HTML_RETURN_TEXT(QString("%1").arg(tmpVehicle->asVehicleIP).toLatin1());
      ADD_TO_HTML_RETURN_TEXT("</td>");

      ADD_TO_HTML_RETURN_TEXT("<td>");
      ADD_TO_HTML_RETURN_TEXT("<div style=\"font-family:Courier New;\">");
      ADD_TO_HTML_RETURN_TEXT(QString(" ").toLatin1());
      ADD_TO_HTML_RETURN_TEXT("</div>");
      ADD_TO_HTML_RETURN_TEXT("</td>");

      ADD_TO_HTML_RETURN_TEXT("<td>");
      ADD_TO_HTML_RETURN_TEXT(asBtnReset_Log.toLatin1());
      ADD_TO_HTML_RETURN_TEXT("</td>");

      ADD_TO_HTML_RETURN_TEXT("<td>");
      ADD_TO_HTML_RETURN_TEXT(QString(" ").toLatin1());
      ADD_TO_HTML_RETURN_TEXT("</td>");

      ADD_TO_HTML_RETURN_TEXT("<td>");
      ADD_TO_HTML_RETURN_TEXT(QString(" ").toLatin1());
      ADD_TO_HTML_RETURN_TEXT("</td>");

      ADD_TO_HTML_RETURN_TEXT("<td>");
      ADD_TO_HTML_RETURN_TEXT(QString(" ").toLatin1());
      ADD_TO_HTML_RETURN_TEXT("</td>");

      ADD_TO_HTML_RETURN_TEXT("<td>");
      ADD_TO_HTML_RETURN_TEXT(QString(" ").toLatin1());
      ADD_TO_HTML_RETURN_TEXT("</td>");

      ADD_TO_HTML_RETURN_TEXT("<td>");
      ADD_TO_HTML_RETURN_TEXT(QString(" ").toLatin1());
      ADD_TO_HTML_RETURN_TEXT("</td>");

      ADD_TO_HTML_RETURN_TEXT("<td>");
      ADD_TO_HTML_RETURN_TEXT(QString(" ").toLatin1());
      ADD_TO_HTML_RETURN_TEXT("</td>");

      ADD_TO_HTML_RETURN_TEXT("<td>");
      ADD_TO_HTML_RETURN_TEXT(QString(" ").toLatin1());
      ADD_TO_HTML_RETURN_TEXT("</td>");

      ADD_TO_HTML_RETURN_TEXT("<td>");
      ADD_TO_HTML_RETURN_TEXT(QString(" ").toLatin1());
      ADD_TO_HTML_RETURN_TEXT("</td>");

      ADD_TO_HTML_RETURN_TEXT("<td>");
      ADD_TO_HTML_RETURN_TEXT(QString(" ").toLatin1());
      ADD_TO_HTML_RETURN_TEXT("</td>");

      ADD_TO_HTML_RETURN_TEXT("<td>");
      ADD_TO_HTML_RETURN_TEXT(QString(" ").toLatin1());
      ADD_TO_HTML_RETURN_TEXT("</td>");

      ADD_TO_HTML_RETURN_TEXT("<td>");
      ADD_TO_HTML_RETURN_TEXT(asBtnReset_OpData.toLatin1());
      ADD_TO_HTML_RETURN_TEXT("</td>");

      ADD_TO_HTML_RETURN_TEXT("<td>");
      ADD_TO_HTML_RETURN_TEXT(QString(" ").toLatin1());
      ADD_TO_HTML_RETURN_TEXT("</td>");


      ADD_TO_HTML_RETURN_TEXT("<td>");
      ADD_TO_HTML_RETURN_TEXT(QString(" ").toLatin1());
      ADD_TO_HTML_RETURN_TEXT("</td>");


      ADD_TO_HTML_RETURN_TEXT("<td>");
      ADD_TO_HTML_RETURN_TEXT(QString(" ").toLatin1());
      ADD_TO_HTML_RETURN_TEXT("</td>");

      ADD_TO_HTML_RETURN_TEXT("<td>");
      ADD_TO_HTML_RETURN_TEXT(QString(" ").toLatin1());
      ADD_TO_HTML_RETURN_TEXT("</td>");

      ADD_TO_HTML_RETURN_TEXT("<td>");
      ADD_TO_HTML_RETURN_TEXT(QString(" ").toLatin1());
      ADD_TO_HTML_RETURN_TEXT("</td>");

      ADD_TO_HTML_RETURN_TEXT("<td>");
      ADD_TO_HTML_RETURN_TEXT(QString(" ").toLatin1());
      ADD_TO_HTML_RETURN_TEXT("</td>");




      ADD_TO_HTML_RETURN_TEXT("</tr>");
    }
  }

  ADD_TO_HTML_NEW_LINE;


  ADD_TO_HTML_RETURN_TEXT("</table>");
  ADD_TO_HTML_RETURN_TEXT("<BR>");

  asTemp =         "Status:<BR>"
                   "0... INIT<BR>"
                   "1... READY<BR>"
                   "2... WAIT FOR NEW QUERY<BR>"
                   "3... WAIT FOR NEW ATTEMPT<BR>";
  ADD_TO_HTML_RETURN_TEXT(asTemp.toLatin1());
  ADD_TO_HTML_RETURN_TEXT("</p></body></html>");



  //debug write..
  QString asHTMLTestFileName = QFileInfo( QCoreApplication::applicationFilePath() ).filePath().section(".",0,0)+".html";
  QFile HtmlTestFile(asHTMLTestFileName);
  HtmlTestFile.open(QIODevice::WriteOnly | QIODevice::Text);
  QTextStream HtmlTestStream(&HtmlTestFile);
  HtmlTestStream << asReturnText;
  HtmlTestFile.close();

  return(asReturnText);
}


void MainWindow::on_Btn_HtmlTest_clicked()
{
  fnAssembleHTMLReport();
}

//**********************
//* EMAIL CLIENT - END *
//**********************


//**********************
//*    STATISTIC       *
//**********************
//clear model



//"To" date must be later than "From" date
void MainWindow::on_calendarWidget_statisticFrom_selectionChanged()
{
  ui->calendarWidget_statisticTo->setMinimumDate(ui->calendarWidget_statisticFrom->selectedDate());
}

//Project
void MainWindow::on_btnMakeProjectFailureStatistic_clicked()
{
  bool boResult;
  QString asTemp;
  //clear previous results
  on_btn_ClearFailureCount_clicked();
  on_btn_ClearProjectFailureCount_clicked();

  //make sum pro individual vehicles
  for (int i = 0; i < mVehicleModel->rowCount(); ++i)
  {
    int iProjectId = mVehicleModel->data(mVehicleModel->index(i, mVehicleModel->fieldIndex("tProjectList_idProject"))).toInt(&boResult);
    if (!boResult) iProjectId = -1;
    if(iProjectId != iLastSelectedProjectForStatistic) continue;

    int iVehicleId = mVehicleModel->data(mVehicleModel->index(i, mVehicleModel->fieldIndex("idVehicle"))).toInt(&boResult);
    if(!boResult) iVehicleId = -1;
    if(iVehicleId > 0)
    {
      iLastSelectedVehicleForStatistic = iVehicleId;
      on_btnMakeVehicleFailureStatistic_clicked();
    }
    qDebug() << DBGINFO << "Static for Project: " << iLastSelectedProjectForStatistic;
  }

  //make sum for projects
  asTemp.clear();
  asTemp = QString
           (
              "SELECT %1 AS `ProjectId`, `EventIndex` , `SourceUnit` , SUM(`FailureCount`) AS `ProjectFailureCount`  "
              "FROM  "
              "("
              "  SELECT * "
              "  FROM `tFailureCount` "
              ")"
              "AS `ttDataRangeI` "
              "GROUP BY `EventIndex` , `SourceUnit` "
            ).arg(iLastSelectedProjectForStatistic);
  qDebug() << DBGINFO << asTemp;

  QSqlQuery StatisticQuery(LocalStatisticDB);
  boResult = StatisticQuery.exec(asTemp);
  qDebug() << DBGINFO << boResult << StatisticQuery.lastError().text();
  if (boResult)
  {
    while (StatisticQuery.next())  //SQLITE does not return size...
    {
      qDebug() << DBGINFO
               <<  StatisticQuery.value("EventIndex").toString()
               <<  StatisticQuery.value("SourceUnit").toString()
               <<  StatisticQuery.value("ProjectFailureCount").toString();
      QSqlRecord tmpRecord = StatisticQuery.record();
      boResult = mProjectFailureCountModel->insertRecord(-1, tmpRecord);
      qDebug() << DBGINFO << boResult << mProjectFailureCountModel->lastError().text() << tmpRecord;
    }

  }
  mProjectFailureCountModel->submitAll();
  mProjectFailureCountModel->select();

}

//Vehicle
void MainWindow::on_btnMakeVehicleFailureStatistic_clicked()
{
  QDate StatisticDateFrom;
  QDate StatisticDateTo;
  QString asStatisticDateFrom;
  QString asStatisticDateTo;
  QString asTemp;
  bool boResult = false;
  //get desired data for statistic
  StatisticDateFrom = ui->calendarWidget_statisticFrom->selectedDate();
  StatisticDateTo   = ui->calendarWidget_statisticTo->selectedDate();
  asStatisticDateFrom = StatisticDateFrom.toString("yyyy'-'MM'-'dd");
  asStatisticDateTo = StatisticDateTo.toString("yyyy'-'MM'-'dd");
  qDebug() << DBGINFO << "Vehicle:" << iLastSelectedVehicleForStatistic << "DATE FROM:" << asStatisticDateFrom << "TO:" << asStatisticDateTo;

  //ASSEMBE QUERY
  asTemp.clear();
  asTemp = QString
           (
              "SELECT %1 AS `VehicleId`, `EventIndex` , `SourceUnit` , COUNT( * ) AS `FailureCount` "
              "FROM "
              "("
              "  SELECT * "
              "  FROM `tVehicleID%1EventLog` "
              "  WHERE `EventDate` >= \"%2\" "
              "  AND   `EventDate` <= \"%3\" "
              ")"
              "AS `ttDataRangeI` "
              "GROUP BY `EventIndex` , `SourceUnit` "
            ).arg(iLastSelectedVehicleForStatistic).arg(asStatisticDateFrom).arg(asStatisticDateTo);
  qDebug() << DBGINFO << asTemp;
  QSqlQuery StatisticQuery;
  boResult = StatisticQuery.exec(asTemp);
  qDebug() << DBGINFO << boResult << StatisticQuery.lastError().text();
  if (boResult && StatisticQuery.size() >= 1)
  {
    for (int i = 0; i < StatisticQuery.size(); ++i)
    {
      StatisticQuery.next();
      qDebug() << DBGINFO
               <<  i
               <<  StatisticQuery.value("EventIndex").toString()
               <<  StatisticQuery.value("SourceUnit").toString()
               <<  StatisticQuery.value("FailureCount").toString()
               <<  StatisticQuery.value(2).toInt();
      QSqlRecord tmpRecord = StatisticQuery.record();
      boResult = mFailureCountModel->insertRecord(-1, tmpRecord);
      qDebug() << DBGINFO << boResult << mFailureCountModel->lastError().text() << tmpRecord;
    }

  }
  mFailureCountModel->submitAll();
  mFailureCountModel->select();
}
//**********************
//* STATISTIC    - END *
//**********************






void MainWindow::on_btn_ClearFailureCount_clicked()
{
  mFailureCountModel->removeRows(0, mFailureCountModel->rowCount());
  mFailureCountModel->submitAll();
  mFailureCountModel->select();
}

void MainWindow::on_btn_ClearProjectFailureCount_clicked()
{
  mProjectFailureCountModel->removeRows(0, mProjectFailureCountModel->rowCount());
  mProjectFailureCountModel->submitAll();
  mProjectFailureCountModel->select();
}
