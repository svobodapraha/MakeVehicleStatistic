DROP TABLE myTMP;
CREATE TABLE myTMP ( 
                     VehicleId INT(11), 
                     ProjectEventIndex INT(11), 
                     ProjectSourceUnit enum('MASTER1', 'MASTER2', 'SLAVE', 'JAMIC', 'NOTE'), 
                     FailureCount INT(11)
                    );
                    
               
INSERT INTO myTMP

SELECT 10001 AS `VehicleId`, `EventIndex` AS `ProjectEventIndex` , `SourceUnit` AS
`ProjectSourceUnit` , `FailureCount`

FROM
(
  SELECT `EventIndex` , `SourceUnit` , COUNT( * ) AS `FailureCount`
  FROM
  (
    SELECT *
    FROM `tVehicleID10001EventLog`
    WHERE `EventDate` >= "2016-01-01"
      AND `EventDate` <= "2016-03-30"
  )
  AS `ttDataRangeID10001`
  GROUP BY `EventIndex` , `SourceUnit`
)
AS `CountID10001` ;


INSERT INTO myTMP

SELECT 10002 AS `VehicleId`, `EventIndex` AS `ProjectEventIndex` , `SourceUnit` AS
`ProjectSourceUnit` , `FailureCount`

FROM
(
  SELECT `EventIndex` , `SourceUnit` , COUNT( * ) AS `FailureCount`
  FROM
  (
    SELECT *
    FROM `tVehicleID10002EventLog`
    WHERE `EventDate` >= "2016-01-01"
      AND `EventDate` <= "2016-03-30"
  )
  AS `ttDataRangeID10002`
  GROUP BY `EventIndex` , `SourceUnit`
)
AS `CountID10002` ;

SELECT `ProjectEventIndex`, `ProjectSourceUnit`, SUM(`FailureCount`) AS `Total`  
FROM myTMP 
GROUP BY
`ProjectEventIndex`, `ProjectSourceUnit`
ORDER BY Total;

