// #include <sys/time.h>

// bool isTimeToExecute(time_t now, tm *startTime)
// {
// 	time_t now;
// 	time(&now);

// 	bool itsTime = now >= mktime(&startCopy) - 2.5 && now <= mktime(&startCopy) + 2.5;

// 	// If _isInterval is true, set new _start time
// 	// if (itsTime && _isInterval)
// 	// {

// 	//     startCopy.tm_sec += _interval;
// 	//     _start = startCopy;
// 	//     // log new start
// 	//     char buffer[80];
// 	//     strftime(buffer, 80, "%FT%TZ", &startCopy);
// 	//     logI("Farming %d new start: %s", _id, buffer);
// 	// }

// 	return itsTime;
// }

// virtual void execute()
// {
// 	logI("Farming %d execute", _id);
// }