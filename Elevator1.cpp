#include "C:\RTExamples\rt.h"
#include "C:\RTExamples\Dispatcher\Monitor.h"

ElevatorStatus status = { 0, 0, 0, {0,0,0,0,0,0,0,0,0,0}, 0, 0, 0 , 0,0 };
UINT  Message;
UINT MessageDatapool;
CMailbox ElevatorMailbox;
CMailbox Elevator1DatapoolMailbox;
Monitor Monitor1("Monitor1");
Elevator Elevator1;

CSemaphore CS_ELE1("CS_Elevator1", 1, 1);
CSemaphore PS_ELE1("PS_Elevator1", 0, 1);

UINT _stdcall Thread1(void*) {
	CMutex M1("Elevator1Mutex");
	while (1) {
		PS_ELE1.Wait();
		//M1.Wait();
		if (status.onFault == 1 && status.emergencyStop == 0) {
			cout << "Elevator 1 Disabled\n";
			//PlaySound(TEXT("elevatorDisable.wav"), NULL, SND_ASYNC);
			Elevator1.Disable();
			Monitor1.Update_Status(Elevator1.reportStatus());
		}
		else if (status.onFault == 3 && status.emergencyStop == 0) {
			cout << "Elevator 1 Re-enabled\n";
			Elevator1.Enable();
			Monitor1.Update_Status(Elevator1.reportStatus());
		}
		else if (status.emergencyStop == 1) {
			//PlaySound(TEXT("marioDeath.wav"), NULL, SND_SYNC);
			cout << "Elevator 1 Emergency Stop\n";
			Elevator1.Disable();
			if (status.floor > 0) {
				Elevator1.Descend();
			}
			Monitor1.Update_Status(Elevator1.reportStatus());
			SLEEP(1000);
		}
		else {
			if (status.direction == 1) {
				//is floor reached?
				if (Elevator1.IsFloorReached()) {
					Elevator1.Halt();
					Elevator1.DoorOpen();
					Monitor1.Update_Status(Elevator1.reportStatus());
					SLEEP(1000);
					Elevator1.DoorClose();
					Monitor1.Update_Status(Elevator1.reportStatus());
					SLEEP(1000);
					if (!Elevator1.IsMoreRequest())
					{//if elevator exhausts requests in the direction of motion, switch downwards
						Elevator1.SwitchDownwards();
						Monitor1.Update_Status(Elevator1.reportStatus());
					}
				}
				else {
					cout << "Elevator in motion, Lifting\n";
					Elevator1.Lift();
					Monitor1.Update_Status(Elevator1.reportStatus());
					SLEEP(1000);
				}
			}
			else if (status.direction == 2) {
				//is floor reached?
				if (Elevator1.IsFloorReached()) {
					Elevator1.Halt();
					Elevator1.DoorOpen();
					Monitor1.Update_Status(Elevator1.reportStatus());
					SLEEP(1000);
					Elevator1.DoorClose();
					Monitor1.Update_Status(Elevator1.reportStatus());
					SLEEP(1000);
					if (!Elevator1.IsMoreRequest())
					{//if elevator exhausts requests in the direction of motion, switch downwards
						cout << "Requests exhausted, start going down\n";
						Elevator1.SwitchDownwards();
					}
				}
				else {
					cout << "Elevator in motion, Descending\n";
					Elevator1.Descend();
					Monitor1.Update_Status(Elevator1.reportStatus());
					SLEEP(1000);
				}
			}
			else {
				//cout << "Elevator Idling\n";
				if (Elevator1.IsMoreRequest())
				{//if elevator is idling on first floor and there is more requests, switch Upwards
					Elevator1.DoorClose();
					Elevator1.SwitchUpwards();
					Monitor1.Update_Status(Elevator1.reportStatus());
				}
				else {
					if (status.floor != 0)
						Elevator1.SwitchDownwards();
					else {
						Elevator1.DoorClose();
					}
					Monitor1.Update_Status(Elevator1.reportStatus());
				}
				SLEEP(1000);
			}
		}

		//report ElevatorStatus to process global variable and the CDataPool
		status = Elevator1.reportStatus();
		printf("Thread1 On Fault Check: %d\n", Elevator1.reportStatus().onFault);
		cout << "Updating Elevator 1 Status to the Monitor\n";
		Monitor1.Update_Status(Elevator1.reportStatus());
		//M1.Signal();
		CS_ELE1.Signal();
	}
	return 0;
}

UINT _stdcall SoundThread(void*) {
	while (1) {
		if (status.direction == 1) {
			//is floor reached?
			if (Elevator1.IsFloorReached())
				PlaySound(TEXT("elevatording.wav"), NULL, SND_ASYNC);
		}
		else if (status.direction == 2) {
			//is floor reached?
			if (Elevator1.IsFloorReached()) {
				PlaySound(TEXT("elevatording.wav"), NULL, SND_ASYNC);
			}
		}
	}
}
//
//UINT _stdcall Thread2(void*) {
//	CMutex M2("M1");
//	while (1) {
//		M2.Wait();
		//cout << "Updating Elevator 1 Status to the Monitor\n";
		//Monitor1.Update_Status(Elevator1.reportStatus());
//		SLEEP(200);
//		M2.Signal();
//	}
//}
int main(void) {
	//Elevator1.receiveStatus(status);
	CThread	t1(Thread1, ACTIVE, NULL); //Elevator Action Thread
	//CThread t2(SoundThread, ACTIVE, NULL);
	//CThread t2(Thread2, ACTIVE, NULL); //Thread used to updating status of the elevator real time
	CMutex M1("Elevator1Mutex");
	// Now that we have the pointer to the datapool, we can put data into it
	while (1) {
		//M1.Wait();
		CS_ELE1.Wait();
		//update status of elevator 1 to the datapool within the monitor
		if (ElevatorMailbox.TestForMessage() == TRUE)
		{
			Message = ElevatorMailbox.GetMessage();
			printf("Message check: %d \n", Message);
			//UINT BIT5 = (Message / 100000) % 10; //Elevator Designation from inside request
			UINT BIT4 = (Message / 10000) % 10; //Direction
			UINT BIT3 = (Message / 1000) % 10; //OutsideFloorReq
			UINT BIT2 = (Message / 100) % 10;	//Inside floor request
			UINT BIT1 = (Message / 10) % 10;	//Onfault 1-disable; 3 - enable;
			UINT BIT0 = Message % 10;	//Emergency stop
			status.emergencyStop = (int)BIT0;
			status.onFault = (int)BIT1;
			status.insideFloorReq = (int)BIT2;
			printf("Requested floor for Elevator 1 %d\n", BIT3);
			status.outsideFloorReq = (int)BIT3;
			//status.elevatorDesignation = (int)BIT5;
			status.floors[status.outsideFloorReq] = (int)BIT4;
			
		}
		//elevator status message passed to Process Global Variable
		Elevator1.receiveStatus(status);
		//M1.Signal();
		PS_ELE1.Signal();
	}
	return 0;
}
