#ifndef VEHICLE_H
#define VEHICLE_H

#include <QObject>

typedef enum {
               prjUNKNOWN,
               prjTRAM,
               prjTROL,
               prjEBUS,
               prjOTHER
           }TeProjectType;


class TVehicle
{
public:
   explicit TVehicle(int iVehicleIdInit);
   ~TVehicle();


   //variables
   int iVehicleId;
   int iVehicleNumber;
   int iProjectId;
   TeProjectType eProjectType;
   QString asProjectName;
   QString asVehicleIP;
   unsigned long dwVehicleIP;



};

#endif // VEHICLE_H
