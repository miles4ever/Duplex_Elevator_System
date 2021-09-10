#include "C:\RTExamples\rt.h"
#include "C:\RTExamples\Dispatcher\Monitor.h"
#include <stdio.h>
#include <iostream>
using namespace std;



//Create Child Processes
CProcess child1("C:\\RTExamples\\Duplex_Elevator_System\\Debug\\Elevator1.exe",
	NORMAL_PRIORITY_CLASS,
	OWN_WINDOW,
	ACTIVE
);
CProcess child2("C:\\RTExamples\\Duplex_Elevator_System\\Debug\\Elevator2.exe",
	NORMAL_PRIORITY_CLASS,
	OWN_WINDOW,
	ACTIVE
);
CProcess child3("C:\\RTExamples\\Duplex_Elevator_System\\Debug\\IO.exe",
	NORMAL_PRIORITY_CLASS,
	OWN_WINDOW,
	ACTIVE
);
ElevatorStatus Elevator_1_Status;
ElevatorStatus Elevator_2_Status;
UINT Message1;
UINT Message2;
int cmdFLG = 0;
//Semaphore to let the IOcommand run first
CSemaphore CIO1("CIO", 1, 1);
CSemaphore PIO1("PIO", 0, 1);
CSemaphore CM1("CM1", 1, 1);
CSemaphore PM1("PM1", 0, 1);
CSemaphore CM2("CM2", 1, 1);
CSemaphore PM2("PM2", 0, 1);

UINT __stdcall Thread1(void*) {
	//Starting Datapool called Elevator1
	Monitor Monitor1("Monitor1");
	while (1) {
		// print out the data in the datapool that was stored there by the parent
		CM1.Wait();
		Elevator_1_Status = Monitor1.Get_Elevator_Status_Dispatcher();
		PM1.Signal();
		//Message1 = Elevator_1_Status.direction * 1000 + Elevator_1_Status.insideFloorReq * 100 + Elevator_1_Status.outsideFloorReq * 10 + Elevator_1_Status.floor;
		/*
		cout << "Monitor 1 Read value for Floor = " << Elevator_1_Status.floor << endl;
		cout << "Monitor 1 Read value for Direction = " << Elevator_1_Status.direction << endl;

		printf("Monitor 1 Read values for floor array = ");
		for (int i = 0; i < 10; i++)
			cout << Elevator_1_Status.floors[i] << " ";
		cout << "\n\n";
		SLEEP(1000);
		*/
	}
	return 0;
}

UINT __stdcall Thread2(void*) {
	//Starting Datapool called Elevator2
	Monitor Monitor2("Monitor2");
	while (1) {
		// print out the data in the datapool that was stored there by the parent
		CM2.Wait();
		Elevator_2_Status = Monitor2.Get_Elevator_Status_Dispatcher();
		PM2.Signal();
		//Message2 = Elevator_2_Status.direction * 1000 + Elevator_2_Status.insideFloorReq * 100 + Elevator_2_Status.outsideFloorReq * 10 + Elevator_2_Status.floor;
		/*
		cout << "Monitor 2 Read value for Floor = " << Elevator_2_Status.floor << endl;
		cout << "Monitor 2 Read value for Direction = " << Elevator_2_Status.direction << endl;

		printf("Child Read values for floor array = ");
		for (int i = 0; i < 10; i++)
			cout << Elevator_2_Status.floors[i] << " ";
		cout << "\n\n";
		SLEEP(1000);
		*/
	}
	return 0;
}
void PackMessage1()
{
	UINT BIT4 = Elevator_1_Status.direction * 10000;
	UINT BIT3 = Elevator_1_Status.outsideFloorReq * 1000;
	UINT BIT2 = Elevator_1_Status.insideFloorReq * 100;
	UINT BIT1 = Elevator_1_Status.onFault * 10;
	UINT BIT0 = Elevator_1_Status.emergencyStop;
	//Message1 = BIT5 + BIT4 + BIT3 + BIT2 + BIT1 + BIT0;
	Message1 =BIT4 + BIT3 + BIT2 + BIT1 + BIT0;
}
void PackMessage2() {
	UINT BIT4 = Elevator_2_Status.direction * 10000;
	UINT BIT3 = Elevator_2_Status.outsideFloorReq * 1000;
	UINT BIT2 = Elevator_2_Status.insideFloorReq * 100;
	UINT BIT1 = Elevator_2_Status.onFault * 10;
	UINT BIT0 = Elevator_2_Status.emergencyStop;
	//Message2 = BIT5 + BIT4 + BIT3 + BIT2 + BIT1 + BIT0;
	Message2 = BIT4 + BIT3 + BIT2 + BIT1 + BIT0;
}
UINT __stdcall Thread3(void*)
{
	CPipe	pipe1("IOcommand", 1024);			// create the three named pipelines 
	//UINT  Message;
	while (1) {
		CIO1.Wait();
		if ((pipe1.TestForData()) >= sizeof(IOcmd))
		{
			cmdFLG = 1;
			//CIO1.Wait();
			pipe1.Read(&IOcmd, sizeof(IOcmd));		// read data from pipe 	

			cout << "Read from IO: Request from Floor: " << IOcmd.outsidefloor << " \n";
			//PIO1.Signal();
			if (IOcmd.direction == 1)
				cout << "Going Up\n";
			else if (IOcmd.direction == 2)
				cout << "Going Down\n";
			cout << "\n";

			//Message1 = IOcmd.direction * 1000 + IOcmd.outsidefloor * 10;
			//Message2 = IOcmd.direction * 1000 + IOcmd.outsidefloor * 10;
		}
		else
		{
			//printf("No data coming from IO\n");
			//child1.Post(0);
		}
		PIO1.Signal();
		//pipe1.Read(&IOcmd, sizeof(IOcmd));		// read data from pipe 	
		//cout << "Read from IO: Request from Floor: " << IOcmd.outsidefloor<< " \n";
		//if (IOcmd.direction == 1)
		//	cout << "Going Up\n";
		//else if (IOcmd.direction == 2)
		//	cout << "Going Down\n";
		//cout << "\n";
		//SLEEP(1000);
	}
	return 0;
}


int main(void) {
	int i;
	int targetDistance1;
	int targetDistance2;
	int requestDistance1 = 255;//distance between outside request floor to current if the elevator 1 in motion pass the floor of which the button is pressed
	int requestDistance2 = 255;//distance between outside request floor to current if the elevator 2 in motion pass the floor of which the button is pressed

	CThread	t1(Thread1, ACTIVE, NULL);
	CThread	t2(Thread2, ACTIVE, NULL);
	CThread t3(Thread3, ACTIVE, NULL);
	CMailbox ElevatorMailbox;
	//Create Child Processes
	/*
	cout << "Creating Child Processes...\n";
	CProcess child1("C:\\RTExamples\\Duplex_Elevator_System\\Debug\\Elevator1.exe",
		NORMAL_PRIORITY_CLASS,
		OWN_WINDOW,
		ACTIVE
	);
	CProcess child2("C:\\RTExamples\\Duplex_Elevator_System\\Debug\\Elevator2.exe",
		NORMAL_PRIORITY_CLASS,
		OWN_WINDOW,
		ACTIVE
	);
	CProcess child3("C:\\RTExamples\\Duplex_Elevator_System\\Debug\\IO.exe",
		NORMAL_PRIORITY_CLASS,
		OWN_WINDOW,
		ACTIVE
	);
	*/

	while (1) {
		//Elevator Assignment Algorithm
			//is there an elevator in motion that will pass the floor of the button pushed
		PIO1.Wait();
		PM1.Wait();
		PM2.Wait();
		//printf("Elevator1 Direction: %d\n", Elevator_1_Status.direction);
		//printf("Elevator2 Direction: %d\n", Elevator_2_Status.direction);
		/*cout << "IOcmd.insideElevatorDesignation: " << IOcmd.insideElevatorDesignation << endl;*/
		
		cout << "IOcmd.fault Elevator before Processing: " << IOcmd.faultElevator << endl;
		if(cmdFLG == 1) {
			if (IOcmd.eStop == 1) {
				Elevator_1_Status.emergencyStop = IOcmd.eStop;
				Elevator_2_Status.emergencyStop = IOcmd.eStop;
				PackMessage1();
				printf("Message1: %d\n", Message1);
				child1.Post(Message1);
				PackMessage2();
				printf("Message2: %d\n", Message2);
				child2.Post(Message2);
			}
			else if (IOcmd.faultElevator != 0) {
				if (IOcmd.faultElevator == 1 ) {//disable/enable elevator 1
					Elevator_1_Status.onFault = IOcmd.faultElevator;
					PackMessage1();
					printf("Disable/Enable Elevator 1, Message1: %d\n", Message1);
					child1.Post(Message1);
				}
				else if (IOcmd.faultElevator == 3) {
					Elevator_1_Status.onFault = 0;
					PackMessage1();
					printf("Disable/Enable Elevator 1, Message1: %d\n", Message1);
					child1.Post(Message1);
				}
				else if (IOcmd.faultElevator == 2 ) {
					Elevator_2_Status.onFault = IOcmd.faultElevator;
					PackMessage2();
					printf("Disable/Enable Elevator 2, Message2: %d\n", Message1);
					child2.Post(Message2);
				}
				else if (IOcmd.faultElevator == 4) {
					Elevator_2_Status.onFault = 0;
					PackMessage2();
					printf("Disable/Enable Elevator 2, Message2: %d\n", Message1);
					child2.Post(Message2);
				}
				IOcmd.faultElevator = 0;
			}
			else {
				if (Elevator_1_Status.onFault == 1 && Elevator_2_Status.onFault == 2) {
				}
				else if (Elevator_1_Status.onFault == 0 && Elevator_2_Status.onFault == 2) { //only elevator 1 is enabled
					if (IOcmd.insideElevatorDesignation == 1) {
						if (Elevator_1_Status.direction != 0) {
							Elevator_1_Status.direction = IOcmd.direction;
							Elevator_1_Status.outsideFloorReq = IOcmd.insidefloor;
							cout << "INSIDE request, Only enabled elevator 1 to go to " << Elevator_1_Status.outsideFloorReq << endl;
						}
						PackMessage1();
						printf("Message1: %d\n", Message1);
						child1.Post(Message1);
						IOcmd.insideElevatorDesignation = 0;
					}
					else if (IOcmd.insideElevatorDesignation == 2) {
						IOcmd.insideElevatorDesignation = 0;
					}
					else {
						cout << "OUTSIDE request, only elevator 1 is enabled\n";
						Elevator_1_Status.direction = IOcmd.direction;
						Elevator_1_Status.outsideFloorReq = IOcmd.outsidefloor;
						PackMessage1();
						printf("Message1: %d\n", Message1);
						child1.Post(Message1);
					}
				}
				else if (Elevator_1_Status.onFault == 1 && Elevator_2_Status.onFault == 0) {//only elevator 2 is enabled
					if (IOcmd.insideElevatorDesignation == 1) {
						IOcmd.insideElevatorDesignation = 0;
					}
					else if (IOcmd.insideElevatorDesignation == 2) {
						if (Elevator_2_Status.direction != 0) {
							Elevator_2_Status.direction = IOcmd.direction;
							Elevator_2_Status.outsideFloorReq = IOcmd.insidefloor;
							cout << "INSIDE request, Only enabled elevator 2 to go to " << Elevator_2_Status.outsideFloorReq << endl;
						}
						PackMessage2();
						printf("Message2: %d\n", Message2);
						child2.Post(Message2);
						IOcmd.insideElevatorDesignation = 0;
					}
					else {
						cout << "only elevator 2 is enabled" << endl;
						Elevator_2_Status.direction = IOcmd.direction;
						Elevator_2_Status.outsideFloorReq = IOcmd.outsidefloor;
						PackMessage2();
						printf("Message2: %d\n", Message2);
						child2.Post(Message2);
					}

				}
				else { //Both elevators enabled
					if (IOcmd.insideElevatorDesignation == 1) {
						if (Elevator_1_Status.direction != 0) {
							Elevator_1_Status.direction = IOcmd.direction;
							Elevator_1_Status.outsideFloorReq = IOcmd.insidefloor;
							cout << "Inside elevator 1 going to " << Elevator_1_Status.outsideFloorReq << endl;
						}
						PackMessage1();
						printf("Message1: %d\n", Message1);
						child1.Post(Message1);
						IOcmd.insideElevatorDesignation = 0;
					}
					else if (IOcmd.insideElevatorDesignation == 2) {
						if (Elevator_2_Status.direction != 0) {
							Elevator_2_Status.direction = IOcmd.direction;
							Elevator_2_Status.outsideFloorReq = IOcmd.insidefloor;
							cout << "Inside elevator 2 going to " << Elevator_2_Status.outsideFloorReq << endl;
						}
						PackMessage2();
						printf("Message2: %d\n", Message2);
						child2.Post(Message2);
						IOcmd.insideElevatorDesignation = 0;
					}
					else {
						if (IOcmd.outsidefloor == Elevator_1_Status.outsideFloorReq) {
							cout << "Repetitive outside request, Send elevator 1\n";
							Elevator_1_Status.direction = IOcmd.direction;
							Elevator_1_Status.outsideFloorReq = IOcmd.outsidefloor;
							PackMessage1();
							printf("Message1: %d\n", Message1);
							child1.Post(Message1);
						}
						else if (IOcmd.outsidefloor == Elevator_2_Status.outsideFloorReq) {
							cout << "Repetitive outside request, Send elevator 2\n";
							Elevator_2_Status.direction = IOcmd.direction;
							Elevator_2_Status.outsideFloorReq = IOcmd.outsidefloor;
							PackMessage2();
							printf("Message2: %d\n", Message2);
							child2.Post(Message2);
						}
						else {
							if (Elevator_1_Status.direction == Elevator_2_Status.direction) {
								if (Elevator_1_Status.direction == 1) //elevator 1 going up
								{
									//traverse the floors requested array for the nearest destination
									for (i = Elevator_1_Status.floor; i < TOPFLOOR; i++) {
										if (Elevator_1_Status.floors[i] == 1) //a request going up exists
										{
											if (IOcmd.outsidefloor <= i && IOcmd.outsidefloor >= Elevator_1_Status.floor) {
												requestDistance1 = IOcmd.outsidefloor - Elevator_1_Status.floors[i];
											}
											else
												requestDistance1 = 255; //when request distance does not exist, set to 255
											break;
										}
									}
								}
								else if (Elevator_1_Status.direction == 2) { //elevator 1 going down
									for (i = Elevator_1_Status.floor; i > 0; i--) {
										if (Elevator_1_Status.floors[i] == 2) //a request going down exists
										{
											if (IOcmd.outsidefloor >= i && IOcmd.outsidefloor <= Elevator_1_Status.floor) {
												requestDistance1 = IOcmd.outsidefloor - Elevator_1_Status.floors[i];
											}
											else
												requestDistance1 = 255; //when request distance does not exist, set to 255
											break;
										}
									}
								}
								if (Elevator_2_Status.direction == 1) //elevator 1 going up
								{
									//traverse the floors requested array for the nearest destination
									for (i = Elevator_2_Status.floor; i < TOPFLOOR; i++) {
										if (Elevator_2_Status.floors[i] == 1) //a request going up exists
										{
											if (IOcmd.outsidefloor <= i && IOcmd.outsidefloor >= Elevator_2_Status.floor) {
												requestDistance2 = IOcmd.outsidefloor - Elevator_2_Status.floors[i];
											}
											else
												requestDistance2 = 255; //when request distance does not exist, set to 255
											break;
										}
									}
								}
								else if (Elevator_2_Status.direction == 2) { //elevator 1 going down
									for (i = Elevator_2_Status.floor; i > 0; i--) {
										if (Elevator_2_Status.floors[i] == 2) //a request going down exists
										{
											if (IOcmd.outsidefloor >= i && IOcmd.outsidefloor <= Elevator_2_Status.floor) {
												requestDistance2 = IOcmd.outsidefloor - Elevator_2_Status.floors[i];
											}
											else
												requestDistance2 = 255; //when request distance does not exist, set to 255
											break;
										}
									}
								}

								//If Both elevator are idling
								if (Elevator_1_Status.direction == 0)
								{
									cout << "Send default elevator 1\n";
									Elevator_1_Status.direction = IOcmd.direction;
									Elevator_1_Status.outsideFloorReq = IOcmd.outsidefloor;
									PackMessage1();
									printf("Message1: %d\n", Message1);
									child1.Post(Message1);
								}
								else
								{
									//case 1: only one elevator is going to pass the outside requested floor
									if (requestDistance1 == 255 && requestDistance2 != 255) {
										//assign elevator 2
										//Elevator_2_Status.floors[IOcmd.outsidefloor] = IOcmd.direction;
										cout << "CASE 1.1" << endl;
										Elevator_2_Status.direction = IOcmd.direction;
										Elevator_2_Status.outsideFloorReq = IOcmd.outsidefloor;
										PackMessage2();
										printf("Message2: %d\n", Message2);
										child2.Post(Message2);
									}
									else if (requestDistance1 != 255 && requestDistance2 == 255) {
										//assign elevator 1
										//Elevator_1_Status.floors[IOcmd.outsidefloor] = IOcmd.direction;
										cout << "CASE 1.2" << endl;
										Elevator_1_Status.direction = IOcmd.direction;
										Elevator_1_Status.outsideFloorReq = IOcmd.outsidefloor;
										PackMessage1();
										printf("Message1: %d\n", Message1);
										child1.Post(Message1);
									}
									//case 2: one elevator has already passed the outside requested floor
									else if (IOcmd.outsidefloor > Elevator_1_Status.floor && IOcmd.outsidefloor < Elevator_2_Status.floor) {
										if (Elevator_1_Status.direction == 1) {
											//two elevators both going up, assign elevator 1 as it is lower than request floor
											   //Elevator_1_Status.floors[IOcmd.outsidefloor] = IOcmd.direction;
											cout << "CASE 2.1" << endl;
											Elevator_1_Status.direction = IOcmd.direction;
											Elevator_1_Status.outsideFloorReq = IOcmd.outsidefloor;
											PackMessage1();
											printf("Message1: %d\n", Message1);
											child1.Post(Message1);
										}
										else if (Elevator_2_Status.direction == 2) {//two elevators both going down, assign elevator 2 as it is higher than request floor
											//Elevator_2_Status.floors[IOcmd.outsidefloor] = IOcmd.direction;
											cout << "CASE 2.2" << endl;
											Elevator_2_Status.direction = IOcmd.direction;
											Elevator_2_Status.outsideFloorReq = IOcmd.outsidefloor;
											PackMessage2();
											printf("Message2: %d\n", Message2);
											child2.Post(Message2);
										}
									}
									else if (IOcmd.outsidefloor < Elevator_1_Status.floor && IOcmd.outsidefloor > Elevator_2_Status.floor) {
										if (Elevator_2_Status.direction == 1) //two elevators both going up, assign elevator 2
										{
											//Elevator_2_Status.floors[IOcmd.outsidefloor] = IOcmd.direction;
											cout << "CASE 2.3" << endl;
											Elevator_2_Status.direction = IOcmd.direction;
											Elevator_2_Status.outsideFloorReq = IOcmd.outsidefloor;
											PackMessage2();
											printf("Message2: %d\n", Message2);
											child2.Post(Message2);
										}
										else if (Elevator_1_Status.direction == 2) //two elevators both going down, assign elevator 1
										{
											//Elevator_1_Status.floors[IOcmd.outsidefloor] = IOcmd.direction;
											cout << "CASE 2.4" << endl;
											Elevator_1_Status.direction = IOcmd.direction;
											Elevator_1_Status.outsideFloorReq = IOcmd.outsidefloor;
											PackMessage1();
											printf("Message1: %d\n", Message1);
											child1.Post(Message1);
										}
									}
									else {
										//Case3: Everything else, send the elevator that is closer to the floor requested 

										targetDistance1 = abs(IOcmd.outsidefloor - Elevator_1_Status.floor);
										targetDistance2 = abs(IOcmd.outsidefloor - Elevator_2_Status.floor);
										if (targetDistance1 < targetDistance2) //send elevator 1
										{
											//Elevator_1_Status.floors[IOcmd.outsidefloor] = IOcmd.direction;
											cout << "CASE 3.1" << endl;
											Elevator_1_Status.direction = IOcmd.direction;
											Elevator_1_Status.outsideFloorReq = IOcmd.outsidefloor;
											PackMessage1();
											printf("Message1: %d\n", Message1);
											child1.Post(Message1);
										}
										else if (targetDistance1 > targetDistance2) //send elevator 2
										{
											//Elevator_2_Status.floors[IOcmd.outsidefloor] = IOcmd.direction;
											cout << "CASE 3.2" << endl;
											Elevator_2_Status.direction = IOcmd.direction;
											Elevator_2_Status.outsideFloorReq = IOcmd.outsidefloor;
											PackMessage2();
											printf("Message2: %d\n", Message2);
											child2.Post(Message2);
										}
										else if (targetDistance1 == 0 && targetDistance2 == 0) { //send elevator 1 as default elevator
											//Elevator_1_Status.floors[IOcmd.outsidefloor] = IOcmd.direction;
											cout << "CASE 3.3" << endl;
											Elevator_1_Status.direction = IOcmd.direction;
											Elevator_1_Status.outsideFloorReq = IOcmd.outsidefloor;
											PackMessage1();
											printf("Message1: %d\n", Message1);
											child1.Post(Message1);
										}
									}
								}

							}
							else if (Elevator_1_Status.direction == 0 && Elevator_2_Status.direction != 0) { //only one elevator is idling, send the idler
								cout << "CASE 4.1" << endl;
								Elevator_1_Status.direction = IOcmd.direction;
								Elevator_1_Status.outsideFloorReq = IOcmd.outsidefloor;
								PackMessage1();
								printf("Message1: %d\n", Message1);
								child1.Post(Message1);
							}
							else if (Elevator_2_Status.direction == 0 && Elevator_1_Status.direction != 0) { //only one elevator is idling, send the idler
								cout << "CASE 4.2" << endl;
								Elevator_2_Status.direction = IOcmd.direction;
								Elevator_2_Status.outsideFloorReq = IOcmd.outsidefloor;
								PackMessage2();
								printf("Message2: %d\n", Message2);
								child2.Post(Message2);
							}
							else if (Elevator_1_Status.direction * Elevator_2_Status.direction == 2) {// one is going up (1) the other going down (2)

								if (IOcmd.direction == Elevator_1_Status.direction)
								{
									//Elevator_1_Status.floors[IOcmd.outsidefloor] = IOcmd.direction;
									cout << "CASE 5.1" << endl;
									Elevator_1_Status.direction = IOcmd.direction;
									Elevator_1_Status.outsideFloorReq = IOcmd.outsidefloor;
									PackMessage1();
									printf("Message1: %d\n", Message1);
									child1.Post(Message1);
								}
								else if (IOcmd.direction == Elevator_2_Status.direction)
								{
									//Elevator_2_Status.floors[IOcmd.outsidefloor] = IOcmd.direction;
									cout << "CASE 5.2" << endl;
									Elevator_2_Status.direction = IOcmd.direction;
									Elevator_2_Status.outsideFloorReq = IOcmd.outsidefloor;
									PackMessage2();
									printf("Message2: %d\n", Message2);
									child2.Post(Message2);
								}
							}
						}
					}
				}
			}
			IOcmd.faultElevator = 0;
			IOcmd.insideElevatorDesignation = 0;
			cmdFLG = 0;
		}
		CM1.Signal();
		CM2.Signal();
		CIO1.Signal();
	}

	child1.WaitForProcess();
	child2.WaitForProcess();
	child3.WaitForProcess();
	return 0;
}
