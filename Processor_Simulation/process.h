
class process //TODO create fully functioning process object
{
public:
	process();
	int actualTimeRemaining();
	int estimatedTimeRemaining();
	int startTime();

	int processSomeTime(int timeProcessed);
	int age(int currentTime);
	int waitTime(int currentTime);
	int turnaroundTime(int currentTime);
	bool isFirstResponse();

	//Setters
	void setPID(int PID);
	void setcpuTime1(int cpuTime1);
	void setcpuTime2(int cpuTime2);
	void setiO1(int iO1);
	void setiO2(int iO2);
	void setArrivalTime(int aTime);
	void setTransferTime(int tTime);

	//Getters
	int getPID();
	int getcpuTime1();
	int getcpuTime2();
	int getiO1();
	int getiO2();
	int getTransferTime();
private:
	int PID, arrivalTime, cpuTime1, iO1, cpuTime2, iO2; // initialized data
	int transferTime; //time at which process has waterfalled into new queue
	int timePreviouslyProcessed = 0;
	bool hasResponded = false;
};

process::process()
{
}

//SET ALL OF THE VALUES WE WANT FROM THIS LIST
void process::setPID(int PID)
{
	this->PID = PID;
}
void process::setcpuTime1(int cpuTime1)
{
	this->cpuTime1 = cpuTime1;
}
void process::setcpuTime2(int cpuTime2)
{
	this->cpuTime2 = cpuTime2;
}
void process::setiO1(int iO1)
{
	this->iO1 = iO1;
}
void process::setiO2(int iO2)
{
	this->iO2 = iO2;
}

void process::setArrivalTime(int aTime) {
	this->arrivalTime = aTime;
}

inline void process::setTransferTime(int tTime)
{
	transferTime = tTime;
}

//GETTERS
int process::getcpuTime1()
{
	return cpuTime1;
}
int process::getcpuTime2()
{
	return cpuTime2;
}
int process::getiO1()
{
	return iO1;
}
int process::getiO2()
{
	return iO2;
}

inline int process::getTransferTime()
{
	return transferTime;
}

int process::actualTimeRemaining() {//returns time remaining to be processed
	return cpuTime1 + iO1 + cpuTime2 + iO2 - timePreviouslyProcessed;
}
int process::estimatedTimeRemaining() {
	return (actualTimeRemaining()) + (rand() % 40 - 20) - timePreviouslyProcessed; //actual Time remaining +/- 20
}
int process::startTime() {//returns the time that the process first enters the queue
	return arrivalTime;
}
int process::getPID() {
	return PID;
}
int process::processSomeTime(int timeProcessed)//should decrement the timeRemaining because it was half processed, return 0 if finished
{
	timePreviouslyProcessed += timeProcessed;
	return actualTimeRemaining();
	//returning 0 means the process finished exacly in that amount of time
	//positive number means process still has time to go
	//negative number means it finished that much before the processor expected
}
int process::age(int currentTime)//return total time since arrival time
{
	return currentTime - arrivalTime;
}

int process::waitTime(int currentTime)//return time spent in queues a.k.a. turnaroundTime - processed time
{
	return currentTime - arrivalTime - (cpuTime1 + iO1 + cpuTime2 + iO2);
}

int process::turnaroundTime(int currentTime)//return total time since arrival (same as age())
{
	return currentTime - arrivalTime;
}

bool process::isFirstResponse()//returns true if not yet responded, every time after will return false
{
	if (hasResponded == false) {
		hasResponded = true;
		return true;
	}
	return false;
}
