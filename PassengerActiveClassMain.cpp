#include "C:\RTExamples\Duplex_Elevator_System\IO\PassengerActiveClass.h"
#include <ctime>
CPipe  pipe2("passengercmd", 1024);
struct passengerData activeClassData;
CMutex M2("M2");
//CCondition EV2("Passenger2");
//CCondition EV1("Passenger1");

PassengerActiveClass::PassengerActiveClass(int number) {
	//printf("Created active class passenger no: %d....\n", number);
}

PassengerActiveClass::~PassengerActiveClass() {
	//printf("In passenger destructive class for passenger....\n");
}

//UINT __stdcall PassengerThread1(void*) {
//	while (1) {
//		EV1.Wait(); // Wait for elevator 1 to come
//		printf("Passenger is in Elevator1..\n");
//	}
//}
//
//UINT __stdcall PassengerThread2(void*) {
//	while (1) {
//		EV2.Wait(); // Wait for elevator 2 to come
//		printf("Passenger is in Elevator2..\n");
//	}
//}


int PassengerActiveClass::main(void) {
	//printf("Hello from Passenger active class\n");
	/*CThread t1(PassengerThread1, ACTIVE, NULL);
	CThread t2(PassengerThread2, ACTIVE, NULL);*/
	srand(time(NULL));
	activeClassData.insideFloorReq = rand() % 10; //generate random number from 0 to 9 (Inside floor req)
	activeClassData.outsideFloorReq = rand() % 10; //generate random number from 0 to 9 (Outside floor req)
	//printf("Rand outsidefloor: %d\n", activeClassData.outsideFloorReq);
	activeClassData.directionReq = (rand() % 2) + 1; //generate random number from 1 to 2 (direction req)
	//printf("Rand direction req: %d\n", activeClassData.directionReq);
	M2.Wait();
	pipe2.Write(&activeClassData, sizeof(activeClassData));
	M2.Signal();
	return 0;
}
