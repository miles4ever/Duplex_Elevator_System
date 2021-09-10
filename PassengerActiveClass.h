#ifndef  __Passenger__
#define __Passenger__
#include <c:\RTExamples\rt.h>
#include <stdio.h>

//*********************************************************************PART B MUTEX********************************************************************************
//CMutex M2("M2"); // For sychronizing pipeline 2 
struct passengerData {
	int outsideFloorReq;
	int directionReq;
	int insideFloorReq;
};

//struct passengerData pipe2;
//*****************************************************************************************************************************************************************
class PassengerActiveClass : public ActiveClass
{
public:
	PassengerActiveClass(int number);
	~PassengerActiveClass();

private:
	int main(void);

};
#endif // ! __Passenger__

