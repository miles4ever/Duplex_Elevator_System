#include "C:\RTExamples\rt.h"
#include "C:\RTExamples\Duplex_Elevator_System\Dispatcher\Monitor.h"

ElevatorStatus status = { 0, 0, 0, {0,0,0,0,0,0,0,0,0,0}, 0, 0, 0,0 ,0}; 
UINT  Message;
UINT MessageDatapool;
CMailbox ElevatorMailbox;
CMailbox Elevator2DatapoolMailbox;
Monitor Monitor2("Monitor2");
Elevator Elevator2;

CSemaphore CS_ELE2("CS_Elevator2", 1, 1);
CSemaphore PS_ELE2("PS_Elevator2", 0, 1);

UINT _stdcall Thread1(void*) {
	CMutex M1("Elevator2Mutex");
	while (1) {
		//M1.Wait();
		PS_ELE2.Wait();
		if (status.onFault == 2 && status.emergencyStop == 0) {
			//PlaySound(TEXT("elevatorDisable.wav"), NULL, SND_ASYNC);
			cout << "Elevator 2 Disabled\n";
			Elevator2.Disable();
			Monitor2.Update_Status(Elevator2.reportStatus());
		}
		else if (status.onFault == 4 && status.emergencyStop == 0) {
			cout << "Elevator 2 Re-enabled\n";
			Elevator2.Enable();
			Monitor2.Update_Status(Elevator2.reportStatus());
		}
		else if (status.emergencyStop == 1) {
			cout << "Elevator 2 Emergency Stop\n";
			//PlaySound(TEXT("marioDeath.wav"), NULL, SND_SYNC);
			Elevator2.Disable();
			if (status.floor > 0) {
				Elevator2.Descend();
			}
			Monitor2.Update_Status(Elevator2.reportStatus());
			SLEEP(1000);
		}
		else {
			if (status.direction == 1) {
				//is floor reached?
				if (Elevator2.IsFloorReached()) {
					Elevator2.Halt();
					Elevator2.DoorOpen();
					Monitor2.Update_Status(Elevator2.reportStatus());
					SLEEP(1000);
					Elevator2.DoorClose();
					Monitor2.Update_Status(Elevator2.reportStatus());
					SLEEP(1000);
					if (!Elevator2.IsMoreRequest())
					{//if elevator exhausts requests in the direction of motion, switch downwards
						Elevator2.SwitchDownwards();
						Monitor2.Update_Status(Elevator2.reportStatus());
					}
				}
				else {
					cout << "Elevator in motion, Lifting\n";
					Elevator2.Lift();
					Monitor2.Update_Status(Elevator2.reportStatus());
					SLEEP(1000);
				}
			}
			else if (status.direction == 2) {
				//is floor reached?
				if (Elevator2.IsFloorReached()) {
					Elevator2.Halt();
					Elevator2.DoorOpen();
					Monitor2.Update_Status(Elevator2.reportStatus());
					SLEEP(1000);
					Elevator2.DoorClose();
					Monitor2.Update_Status(Elevator2.reportStatus());
					SLEEP(1000);
					if (!Elevator2.IsMoreRequest())
					{//if elevator exhausts requests in the direction of motion, switch downwards
						cout << "Requests exhausted, start going down\n";
						Elevator2.SwitchDownwards();
					}
				}
				else {
					cout << "Elevator in motion, Descending\n";
					Elevator2.Descend();
					Monitor2.Update_Status(Elevator2.reportStatus());
					SLEEP(1000);
				}
			}
			else {
				//cout << "Elevator Idling\n";
				if (Elevator2.IsMoreRequest())
				{//if elevator is idling on first floor and there is more requests, switch Upwards
					Elevator2.DoorClose();
					Elevator2.SwitchUpwards();
					Monitor2.Update_Status(Elevator2.reportStatus());
				}
				else {
					if (status.floor != 0)
						Elevator2.SwitchDownwards();
					else {
						Elevator2.DoorClose();
					}
					Monitor2.Update_Status(Elevator2.reportStatus());
				}
				SLEEP(1000);
			}
		}

		//report ElevatorStatus to process global variable and the CDataPool
		status = Elevator2.reportStatus();
		printf("Thread1 On Fault Check: %d\n", Elevator2.reportStatus().onFault);
		cout << "Updating Elevator 2 Status to the Monitor\n";
		Monitor2.Update_Status(Elevator2.reportStatus());
		//M1.Signal();
		CS_ELE2.Signal();
	}
	return 0;
}

UINT _stdcall SoundThread(void*) {
	while (1) {
		if (status.direction == 1) {
			//is floor reached?
			if (Elevator2.IsFloorReached())
				PlaySound(TEXT("elevatording.wav"), NULL, SND_ASYNC);
		}
		else if (status.direction == 2) {
			//is floor reached?
			if (Elevator2.IsFloorReached()) {
				PlaySound(TEXT("elevatording.wav"), NULL, SND_ASYNC);
			}
		}
	}
}
int main(void) {
	CThread	t1(Thread1, ACTIVE, NULL); //Elevator Action Thread
	//CThread t2(SoundThread, ACTIVE, NULL);
	CMutex M1("Elevator2Mutex");
	// Now that we have the pointer to the datapool, we can put data into it
	while (1) {
		//M1.Wait();
		CS_ELE2.Wait();
		//update status of elevator 1 to the datapool within the monitor
		if (ElevatorMailbox.TestForMessage() == TRUE)
		{
			Message = ElevatorMailbox.GetMessage();
			printf("Message check: %d \n", Message);
			//UINT BIT5 = (Message / 100000) % 10; //Elevator Designation from inside request
			UINT BIT4 = (Message / 10000) % 10; //Direction
			UINT BIT3 = (Message / 1000) % 10; //OutsideFloorReq
			UINT BIT2 = (Message / 100) % 10;	//Inside floor request
			UINT BIT1 = (Message / 10) % 10;	//Onfault 2-disable; 4 - enable;
			UINT BIT0 = Message % 10;	//Emergency stop
			status.emergencyStop = (int)BIT0;
			status.onFault = (int)BIT1;
			status.insideFloorReq = (int)BIT2;
			printf("Requested floor for Elevator 1 %d\n", BIT3);
			status.outsideFloorReq = (int)BIT3;
			//status.elevatorDesignation = (int)BIT5;
			status.floors[status.outsideFloorReq] = (int)BIT4;
			
		}
		Elevator2.receiveStatus(status);
		PS_ELE2.Signal();
		//M1.Signal();
	}
	return 0;
}
