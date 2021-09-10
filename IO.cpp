#include "C:\RTExamples\rt.h"
#include "C:\RTExamples\Duplex_Elevator_System\Dispatcher\Monitor.h"
#include "C:\RTExamples\Duplex_Elevator_System\IO\PassengerActiveClass.h"
#include <stdio.h>

ElevatorStatus Elevator_1_Status;
ElevatorStatus Elevator_2_Status;
char c1;
char c2;
int activeClassState = 0;
//CCondition EV2("Passenger2");
//CCondition EV1("Passenger1");
CMutex MP("MP");

UINT __stdcall Thread1(void*) {
	//Starting Datapool called Elevator1
	CMutex M1("M1");
	Monitor Monitor1("Monitor1");
	while (1) {
		// print out the data in the datapool that was stored there by the parent
		Elevator_1_Status = Monitor1.Get_Elevator_Status_IO();
		M1.Wait();
		CURSOR_OFF();
		int x = 0;
		int y = 11;
		int elevatorIndex = 1;
		if (Elevator_1_Status.emergencyStop == 1) {
			TEXT_COLOUR(12, 0); //Text RED;
		}
		else if (Elevator_1_Status.emergencyStop == 0 && Elevator_1_Status.onFault == 1) {
			TEXT_COLOUR(5, 0);//Text Dark Magenta when on fault/disabled;
		}
		else {
			TEXT_COLOUR(14, 0); //Yellow when operating
		}
		animateElevator(Elevator_1_Status, x, y, elevatorIndex);
		/*
		cout << "Monitor 1 Read value for Floor = " << Elevator_1_Status.floor << endl;
		cout << "Monitor 1 Read value for Direction = " << Elevator_1_Status.direction << endl;

		printf("Monitor 1 Read values for floor array = ");
		for (int i = 0; i < 10; i++)
			cout << Elevator_1_Status.floors[i] << " ";
		cout << "\n\n";
		*/
		M1.Signal();
	}
	return 0;
}

UINT __stdcall Thread2(void*) {
	//Starting Datapool called Elevator2
	CMutex M1("M1");
	Monitor Monitor2("Monitor2");
	while (1) {
		// print out the data in the datapool that was stored there by the parent
		Elevator_2_Status = Monitor2.Get_Elevator_Status_IO();
		M1.Wait();
		CURSOR_OFF();
		int x = 45;
		int y = 11;
		int elevatorIndex = 2;
		if (Elevator_2_Status.emergencyStop == 1) {
			TEXT_COLOUR(12, 0); //Text RED;
		}
		else if (Elevator_2_Status.emergencyStop == 0 && Elevator_2_Status.onFault == 2) {
			TEXT_COLOUR(5, 0);//Text Dark Magenta when on fault/disabled;
		}
		else {
			TEXT_COLOUR(14, 0); // Yellow when operating
		}
		animateElevator(Elevator_2_Status, x, y, elevatorIndex);
		/*
		cout << "Monitor 2 Read value for Floor = " << Elevator_2_Status.floor << endl;
		cout << "Monitor 2 Read value for Direction = " << Elevator_2_Status.direction << endl;

		printf("Child Read values for floor array = ");
		for (int i = 0; i < 10; i++)
			cout << Elevator_2_Status.floors[i] << " ";
		cout << "\n\n";
		*/

		M1.Signal();
	}
	return 0;
}

UINT __stdcall Thread3(void*) {
	//Polling User Input
	CMutex M1("M1");
	CPipe  pipe3("IOcommand", 1024);
	while (1) {
		//system("cls");

		c1 = _getch();
		if (c1 == 'u') {
			IOcmd.direction = 1;
			IOcmd.insideElevatorDesignation = 0;
			IOcmd.faultElevator = 0;
			c2 = _getch();
			if (c2 - '0' <= TOPFLOOR && c2 - '0' >= 0) {
				IOcmd.outsidefloor = c2 - '0';
			}
			else {
				//cout << "invalid 2nd input, enter again.\n";
			}
			MP.Wait();
			pipe3.Write(&IOcmd, sizeof(IOcmd));
			MP.Signal();
		}
		else if (c1 == 'd') {

			c2 = _getch();
			if (c2 == '+')
			{
				//Start active class
				activeClassState = 1;
			}
			else if (c2 == '-') {
				//Stop active class
				activeClassState = 0;
			}
			else {
				IOcmd.direction = 2;
				IOcmd.insideElevatorDesignation = 0;
				IOcmd.faultElevator = 0;
				if (c2 - '0' <= TOPFLOOR && c2 - '0' >= 0) {
					IOcmd.outsidefloor = c2 - '0';
				}
				else {
					//cout << "invalid 2nd input, enter again.\n";
				}
				MP.Wait();
				pipe3.Write(&IOcmd, sizeof(IOcmd));
				MP.Signal();
			}

		}
		else if (c1 == '1') { //designate elevator 1 to receive INSIDE floor request
			IOcmd.insideElevatorDesignation = 1;
			IOcmd.direction = Elevator_1_Status.direction;
			IOcmd.faultElevator = 0;
			c2 = _getch();
			if (c2 - '0' <= TOPFLOOR && c2 - '0' >= 0) {
				IOcmd.insidefloor = c2 - '0';
			}
			pipe3.Write(&IOcmd, sizeof(IOcmd));
		}
		else if (c1 == '2') {//designate elevator 2 to receive INSIDE floor request
			IOcmd.insideElevatorDesignation = 2;
			IOcmd.direction = Elevator_2_Status.direction;
			IOcmd.faultElevator = 0;
			c2 = _getch();
			if (c2 - '0' <= TOPFLOOR && c2 - '0' >= 0) {
				IOcmd.insidefloor = c2 - '0';
			}
			pipe3.Write(&IOcmd, sizeof(IOcmd));
		}
		else if (c1 == '-') {
			c2 = _getch();
			if (c2 == '1' ) {
				IOcmd.faultElevator = 1; //elevator 1 on fault
				PlaySound(TEXT("elevatorDisable.wav"), NULL, SND_ASYNC);
			}
			else if (c2 == '2') {
				IOcmd.faultElevator = 2; //elevator 1 on fault
				PlaySound(TEXT("elevatorDisable.wav"), NULL, SND_ASYNC);
			}
			pipe3.Write(&IOcmd, sizeof(IOcmd));
		}
		else if (c1 == '+') {
			c2 = _getch();
			if (c2 == '1' ) {
				IOcmd.faultElevator = 3; //message to enable elevator 1
				PlaySound(TEXT("enable.wav"), NULL, SND_ASYNC);
			}
			else if (c2 == '2') {
				IOcmd.faultElevator = 4; //message to enable elevator 2
				PlaySound(TEXT("enable.wav"), NULL, SND_ASYNC);
			}
			pipe3.Write(&IOcmd, sizeof(IOcmd));
		}
		else if (c1 == 'e') {
			c2 = _getch();
			if (c2 == 'e') {
				PlaySound(TEXT("marioDeath.wav"), NULL, SND_ASYNC);
				IOcmd.eStop = 1;
			}
			else {
				IOcmd.eStop = 0;
			}
			pipe3.Write(&IOcmd, sizeof(IOcmd));
		}

		M1.Wait();
		MOVE_CURSOR(80, 35);
		cout << "IO command: " << c1 << c2 << endl;
		M1.Signal();
		/*if (user == "p") {
			M1.Wait();
			system("cls");
			cout << "-Welcome to WHAT A Elevator-" << endl;
			cout << "Are you going up or down? Please enter [u:up; d:down]: ";
			cin >> user;
			if (user == "u") {
				cout << "Going Up\n";
				IOcmd.direction = 1;
				cout << "Tell me your current floor: ";
				cin >> user;
				IOcmd.outsidefloor = stoi(user);
				cout << "Elevator coming to pick you up from " << user << " floor.\n";
				pipe3.Write(&IOcmd, sizeof(IOcmd));
			}
			else if (user == "d") {
				cout << "Going Down\n";
				IOcmd.direction = 2;
				cout << "Tell me your current floor: ";
				cin >> user;
				IOcmd.outsidefloor = stoi(user);
				cout << "Elevator coming to pick you up from " << user << " floor.\n";
				pipe3.Write(&IOcmd, sizeof(IOcmd));
			}
			else {
				cout << "invalid input, enter 'p' again.\n";
			}
			M1.Signal();
		}*/
	}
	return 0;
}

int main(void) {
	CMutex M1("M1");
	int i = 0;
	CURSOR_OFF();
	MOVE_CURSOR(0, 0);
	PlaySound(TEXT("begin.wav"), NULL, SND_ASYNC);
	cout << "-Welcome to WHAT A Elevator-" << endl;
	cout << "External Request: Please enter [u:up; d:down] followed by your current floor number [0-9], e.g. u9" << endl;
	cout << "Internal Request: Please enter [1/2] followed by the target floor [0-9]; Emergency Stop: 'ee'; " << endl;
	cout << " ________   ________      ____________   ____________   ________      __         __        __" << endl;
	cout << "|   _____| |    _   |    |   _________| |   ________|  |  ______|    /  \\       |  |      |  |" << endl;
	cout << "|  |___    |   |_|   |   |  |_____      |  |_____      |  |___      / _\\ \\      |  |	  |  |" << endl;
	cout << "|   ___|   |     ___|    |   _____|     |   _____|     |   ___|    /  __  \\     |  |      |  |" << endl;
	cout << "|  |       |     \\       |  |           |  |	       |  |       /  /  \\  \\    |  |	  |  |" << endl;
	cout << "|  |       |  |\\  \\      |  |____       |  |___	       |  |      /  /    \\  \\   |  |____  |  |____" << endl;
	cout << "|__|       |__| \\__\\     |_______|      |______|       |__|     /__/      \\__\\  |_______| |_______|" << endl;
	CThread	t1(Thread1, ACTIVE, NULL);
	CThread	t2(Thread2, ACTIVE, NULL);
	CThread t3(Thread3, ACTIVE, NULL);
	PassengerActiveClass* passenger[1];
	CSemaphore CB1("CB1", 1, 1);
	CSemaphore PB1("PB1", 0, 1);
	CPipe  pipe2("passengercmd", 1024);
	CPipe  pipe3("IOcommand", 1024);
	struct passengerData activeClassData;
	while (1) {
		CURSOR_OFF();
		while (activeClassState == 1)
		{
			//printf("Initiate active class...\n");
			passenger[i] = new PassengerActiveClass(i);
			SLEEP(2000);
			passenger[i]->Resume();
			i++;
			if ((pipe2.TestForData()) >= sizeof(activeClassData))
			{
				pipe2.Read(&activeClassData, sizeof(activeClassData));
				IOcmd.outsidefloor = activeClassData.outsideFloorReq;
				//printf("Outside floor req: %d\n", IOcmd.outsidefloor);
				IOcmd.direction = activeClassData.directionReq;
				IOcmd.insidefloor = activeClassData.insideFloorReq;
				M1.Wait();
				CURSOR_OFF();
				MOVE_CURSOR(71, 31);
				cout << "Actice class activate" << endl;
				M1.Signal();
				MP.Wait();
				pipe3.Write(&IOcmd, sizeof(IOcmd));
				MP.Signal();
				SLEEP(2000);
			}
		}
		M1.Wait();
		MOVE_CURSOR(71, 31);
		cout << "Actice class de-activate" << endl;
		M1.Signal();
		//if (Elevator_1_Status.floor == activeClassData.outsideFloorReq)
		//{
		//	if (Elevator_1_Status.passenger < 4)
		//	{
		//		EV1.Signal(); // Passenger enter elevator1
		//		Elevator_1_Status.passenger++;
		//		EV1.Reset(); // Close elevator door
		//	}
		//}
		//else if (Elevator_2_Status.floor == activeClassData.outsideFloorReq) {
		//	if (Elevator_2_Status.passenger < 4)
		//	{
		//		EV2.Signal(); // Passenger enter elevator1
		//		Elevator_2_Status.passenger++;
		//		EV2.Reset(); //Close elvator door
		//	}
		//}

		//if (Elevator_1_Status.floor == activeClassData.insideFloorReq)
		//{
		//	Elevator_1_Status.passenger--;
		//}
		//else if (Elevator_2_Status.floor == activeClassData.insideFloorReq)
		//{
		//	Elevator_2_Status.passenger--;
		//}

	}

	//child1.WaitForProcess();
	//child2.WaitForProcess();
	//child3.WaitForProcess();
	return 0;
}

//
//int main(void) {
//	CMutex M1("M1");
//	int i = 0;
//	CURSOR_OFF();
//	PlaySound(TEXT("begin.wav"), NULL, SND_ASYNC);
//	MOVE_CURSOR(0, 0);
//	cout << "-Welcome to WHAT A Elevator-" << endl;
//	cout << "External Request Please enter [u:up; d:down] followed by your current floor number [0-9], e.g. u9" << endl;
//	cout << "Emergency Stop: 'ee'" << endl;
//	CThread	t1(Thread1, ACTIVE, NULL);
//	CThread	t2(Thread2, ACTIVE, NULL);
//	CThread t3(Thread3, ACTIVE, NULL);
//	PassengerActiveClass* passenger[1];
//	CSemaphore CB1("CB1", 1, 1);
//	CSemaphore PB1("PB1", 0, 1);
//	CPipe  pipe2("passengercmd", 1024);
//	CPipe  pipe3("IOcommand", 1024);
//	struct passengerData activeClassData;
//	while (1) {
//		//PlaySound(TEXT("elevatording.wav"), NULL, SND_SYNC);
//		while (activeClassState == 1)
//		{
//			//printf("Initiate active class...\n");
//			passenger[i] = new PassengerActiveClass(i);
//			SLEEP(2000);
//			passenger[i]->Resume();
//			i++;
//			if ((pipe2.TestForData()) >= sizeof(activeClassData))
//			{
//				pipe2.Read(&activeClassData, sizeof(activeClassData));
//				IOcmd.outsidefloor = activeClassData.outsideFloorReq;
//				//printf("Outside floor req: %d\n", IOcmd.outsidefloor);
//				IOcmd.direction = activeClassData.directionReq;
//				IOcmd.insidefloor = activeClassData.insideFloorReq;
//				MP.Wait();
//				pipe3.Write(&IOcmd, sizeof(IOcmd));
//				MP.Signal();
//				SLEEP(2000);
//			}
//		}
//		//if (Elevator_1_Status.floor == activeClassData.outsideFloorReq)
//		//{
//		//	if (Elevator_1_Status.passenger < 4)
//		//	{
//		//		EV1.Signal(); // Passenger enter elevator1
//		//		Elevator_1_Status.passenger++;
//		//		EV1.Reset(); // Close elevator door
//		//	}
//		//}
//		//else if (Elevator_2_Status.floor == activeClassData.outsideFloorReq) {
//		//	if (Elevator_2_Status.passenger < 4)
//		//	{
//		//		EV2.Signal(); // Passenger enter elevator1
//		//		Elevator_2_Status.passenger++;
//		//		EV2.Reset(); //Close elvator door
//		//	}
//		//}
//
//		//if (Elevator_1_Status.floor == activeClassData.insideFloorReq)
//		//{
//		//	Elevator_1_Status.passenger--;
//		//}
//		//else if (Elevator_2_Status.floor == activeClassData.insideFloorReq)
//		//{
//		//	Elevator_2_Status.passenger--;
//		//}
//
//	}
//
//	//child1.WaitForProcess();
//	//child2.WaitForProcess();
//	//child3.WaitForProcess();
//	return 0;
//}

/*
int main(void){
	CThread	t1(Thread1, ACTIVE, NULL);
	CThread	t2(Thread2, ACTIVE, NULL);
	cout << "IO Process Creating the Pipeline.....\n";
	CPipe	IOpipe("IOcommand",1024);							// Create a pipe with the name "Pipe1"
	 Now link to obtain address

	int direction = 1;
	int selectElevator = 1; // 1 or 2
	int floor = 3;

	while(1) {
		cout << "Parent Process Creating Child Process writing to Pipeline.....\n";
		IOcmd = { floor, direction, selectElevator };
		IOpipe.Write(&IOcmd, sizeof(IOcmd));
		SLEEP(1900);
	}

	return 0;
}
*/
