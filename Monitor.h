#ifndef __Monitor__
#define __Monitor__
#define TOPFLOOR 9
#include "C:\RTExamples\rt.h"
#include <iostream>
#include <string.h>
#include <Windows.h>
#include <mmsystem.h>
using namespace std;

struct ElevatorStatus {	// start of structure template
	int floor;		// floor corresponding to lifts current position
	int direction;		// direction of travel of elevator: 0 = idle, 1 = up, 2 = down
	int doorStatus;		// 1 = Open; 0 = Closed
	int floors[10];		// an array representing the floors and whether requests are set and the values represnet direction; 0 = idle, 1 = up, 2 = down
	int outsideFloorReq; //***Added one Nov 1st : store requested floor from the outside
	int insideFloorReq; // **Added one store inside floor request by the passenger
	int onFault;		// 0 - enabled; 1 - disable Elevator 1; 2 - disable Elevator 2; 3- re-enable elevator 1; 4 - re-enable elevator 2
	int emergencyStop;
	int passenger;
};				// end of structure template


struct mypipelinedata {
	int outsidefloor;	//Show what is the floor outside the elevator
	int insidefloor;
	int insideElevatorDesignation;
	int doorStatus;		//Show door open/close status
	int direction;		// Show direction up/down
	int eStop;			// 0 = eStop disabled; 1 = eStop enabled
	int faultElevator;	//indicate which elevator is on fault
};

struct mypipelinedata IOcmd;


/*
CSemaphore PS1(string("PS1"), 0, 1);
CSemaphore CS1(string("CS1"), 1, 1);
CSemaphore PS2(string("CS2"), 0, 1);
CSemaphore CS2(string("PS2"), 1, 1);
CSemaphore PS3(string("PS3"), 0, 1);
CSemaphore CS3(string("CS3"), 1, 1);
CSemaphore PS4(string("CS4"), 0, 1);
CSemaphore CS4(string("PS4"), 1, 1);
*/
class Monitor {
private:

	CDataPool* ElevatorStatusDataPool;
	CMutex* theMutex;
	ElevatorStatus* D_ptr;
	CSemaphore* PS1;
	CSemaphore* CS1;
	CSemaphore* PS2;
	CSemaphore* CS2;

public:
	void Update_Status(struct ElevatorStatus status);
	struct ElevatorStatus Get_Elevator_Status_Dispatcher(void);
	struct ElevatorStatus Get_Elevator_Status_IO(void);
	Monitor(string name);
	~Monitor();
};

void Monitor::Update_Status(struct ElevatorStatus status) {

	CS1->Wait();
	CS2->Wait();
	//update elevator status
	*D_ptr = status;
	PS1->Signal();
	PS2->Signal();
	
}

ElevatorStatus Monitor::Get_Elevator_Status_Dispatcher(void) {
	PS1->Wait();
	struct ElevatorStatus copyStatus;
	//get elevator status
	copyStatus = *D_ptr;
	CS1->Signal();
	return copyStatus;
}

ElevatorStatus Monitor::Get_Elevator_Status_IO(void) {
	PS2->Wait();
	struct ElevatorStatus copyStatus;
	//get elevator status
	copyStatus = *D_ptr;
	CS2->Signal();
	return copyStatus;
}
Monitor::Monitor(string name) {
	theMutex = new CMutex(string("__Mutux__") + string(name));
	ElevatorStatusDataPool = new CDataPool(string("__DataPool__") + string(name),sizeof(struct ElevatorStatus));
	D_ptr = (struct ElevatorStatus*)(ElevatorStatusDataPool->LinkDataPool());
	
	PS1 = new CSemaphore(string("PS1") + string(name), 0, 1);
	CS1 = new CSemaphore(string("CS1") + string(name), 1, 1);
	PS2 = new CSemaphore(string("PS2") + string(name), 0, 1);
	CS2 = new CSemaphore(string("CS2") + string(name), 1, 1);
	
}

Monitor::~Monitor() {
	delete(ElevatorStatusDataPool);
	delete(theMutex);
	delete(D_ptr);
	delete(PS1);
	delete(CS1);
	delete(PS2);
	delete(CS2);
}

class Elevator {
private:
	ElevatorStatus status;
	int enableFLG = 0;
public:
	Elevator() { status.doorStatus = 0; status.direction = 0; }
	~Elevator() {}
	void receiveStatus(ElevatorStatus message) { status = message; }
	ElevatorStatus reportStatus() { return status; }
	void Lift() {
		if (status.floor < 9){
			PlaySound(TEXT("elevatorUp.wav"), NULL, SND_ASYNC);
			status.floor++;
		}
		else
			status.direction = 2;
	}
	void Descend() {
		if (status.floor > 0) {
			status.floor--;
			PlaySound(TEXT("elevatorDown.wav"), NULL, SND_ASYNC);
		}
		else {
			DoorOpen();
			PlaySound(TEXT("elevatording.wav"), NULL, SND_ASYNC);
			//Disable();
			status.direction = 0;
		}
	}
	void SwitchDownwards() {//when elevator exhausts requests in the direction of motion, switch downwards
		cout << "Requests exhausted, start going down\n";
		status.direction = 2;
	}
	void SwitchUpwards() {//when elevator exhausts requests in the direction of motion, switch downwards
		cout << "start going up\n";
		status.direction = 1;
	}
	int IsFloorReached() {
		if (IsMoreRequest()) {
			if (status.floors[status.floor] == status.direction) {
				PlaySound(TEXT("elevatording.wav"), NULL, SND_ASYNC);
				return true;
			}
			else
				return false;
		}
		else {
			if (status.floors[status.floor] == 2 && status.direction == 1) //when elevator exhausts upward requests, and it stops at floor requested to go down above its current floor
			{
				PlaySound(TEXT("elevatording.wav"), NULL, SND_ASYNC);
				return true;
			}
			else if (status.floors[status.floor] == status.direction) {
				PlaySound(TEXT("elevatording.wav"), NULL, SND_ASYNC);
				return true;
			}
			else
				return false;
		}
	}
	int IsMoreRequest() {//check if there is more request in the direction of motion when one elevator reaches a requested floor
		int j;
		if (status.direction == 1)//traverse up for requests
		{
			for (j = status.floor+1; j <= TOPFLOOR; j++)
			{
				if (status.floors[j] != 0) {
					cout << "More requests UPWARDS\n";
					return true;
				}
			}
		}
		else if (status.direction == 2)//tracerse down for requests
		{
			for (j = status.floor-1; j >= 0; j--) {
				if (status.floors[j] != 0) {
					cout << "More requests DOWNWARDS\n";
					return true;
				}
			}
		}
		else if (status.direction == 0) {
			for (j = status.floor; j <= TOPFLOOR; j++)
			{
				if (status.floors[j] != 0) {
					cout << "More requests UPWARDS\n";
					return true;
				}
			}
		}
		cout << "NO MORE REQUESTS\n";
		return false;
	}
	void Halt() { //clear bit in the floor request array and halt elevator to allow passengers to enter/exit
		cout << "Target floor reached, halt.\n";
		status.floors[status.floor] = 0;
	}
	void DoorOpen() {
		cout << "Target floor reached, door OPEN.\n";
		status.doorStatus = 1;
	}
	void DoorClose() {
		cout << "Target floor reached, door CLOSED.\n";
		status.doorStatus = 0;
	}
	void Disable() {
		if (enableFLG == 0) {
			enableFLG = 1;
		}
		for (int i = 0; i <= TOPFLOOR; i++)
		{
			status.floors[i] = 0;
		}
		status.direction = 0;
	}
	void Enable() {
		if (enableFLG == 1) {
			//PlaySound(TEXT("elevatorDisable.wav"), NULL, SND_ASYNC);
			enableFLG = 0;
		}
		status.onFault = 0;
	}
};
void animateElevator(ElevatorStatus	status, int x, int y, int elevatorIndex) {
	int i;
	MOVE_CURSOR(x, y);
	cout << " -Current Floor-" << endl;
	MOVE_CURSOR(x, y + 1);
	cout << ".    _______   ." << endl;
	MOVE_CURSOR(x, y + 2);
	cout << "|   //| " << status.floor << " |\\\\  |" << endl;
	MOVE_CURSOR(x, y + 3);
	cout << "|  //       \\\\ |" << endl;
	int xlocal = x;
	int ylocal = y + 4;
	MOVE_CURSOR(xlocal, ylocal);
	cout << "(69696969696969) Floor Requested:" << endl;
	//cout << "_____"<<status.floor<< "_____" << endl;
	for (i = 1; i < 20; i++) {
		MOVE_CURSOR(xlocal, ylocal + i);
		if (i % 2 == 0)
			cout << "||------------||" << endl;
		else
			cout << "||            || " << status.floors[int(9.5 - 0.5 * i)] << endl;
	}
	MOVE_CURSOR(xlocal, (ylocal + 19 - 2 * status.floor));
	if (status.doorStatus == 0)
		cout << "||I====][====I||" << endl;
	else if (status.doorStatus == 1)
		cout << "||I=]      [=I||" << endl;

	MOVE_CURSOR(xlocal + 23, ylocal + 9);
	cout << "Elevator " << elevatorIndex << " Status";
	MOVE_CURSOR(xlocal + 23, ylocal + 10);
	if (status.direction == 1)
		cout << "Direction:   UP\n";
	else if (status.direction == 2)
		cout << "Direction: Down\n";
	else if (status.direction == 0)
		cout << "Direction: Idle\n";
	MOVE_CURSOR(xlocal + 23, ylocal + 11);
	if (status.doorStatus == 0)
		cout << "Door Status: Closed\n";
	else if (status.doorStatus == 1)
		cout << "Door Status:   Open\n";

	MOVE_CURSOR(xlocal, ylocal + 20);
	cout << "XXXXXXXXXXXXXXXX" << endl;

	fflush(stdout);
}
//void animateElevator(ElevatorStatus	status, int x, int y, int elevatorIndex) {
//	int i;
//	MOVE_CURSOR(x, y);
//	cout << "__________ Floor Requested:"<< endl;
//	for (i = 1; i < 20; i++) {
//		MOVE_CURSOR(x, y + i);
//		if (i % 2 == 0)
//			cout << "|--------|" << endl;
//		else
//			cout << "|        | " << status.floors[int(9.5 - 0.5 * i)] << endl;
//	}
//	MOVE_CURSOR(x, (y + 19 - 2*status.floor));
//	if(status.doorStatus == 0)
//		cout << "|===||===|" << endl;
//	else if (status.doorStatus == 1)
//		cout << "|-|    |-|" << endl;
//
//	MOVE_CURSOR(x + 15, y + 9);
//	cout << "Elevator " << elevatorIndex << " Status";
//	MOVE_CURSOR(x + 15, y + 10);
//	if (status.direction == 1)
//		cout << "Direction:   UP\n";
//	else if (status.direction == 2)
//		cout << "Direction: Down\n";
//	else if (status.direction == 0)
//		cout << "Direction: Idle\n";
//	MOVE_CURSOR(x + 15, y + 11);
//	if (status.doorStatus == 0)
//		cout << "Door Status: Closed\n";
//	else if (status.doorStatus == 1)
//		cout << "Door Status:   Open\n";
//
//	MOVE_CURSOR(x, y + 20);
//	cout << "##########" << endl;
//
//	fflush(stdout);
//}
#endif