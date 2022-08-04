#include "ESPClock.h"

#define SECONDS_PER_DAY 86400
/* 
	ESPClock class - 12 hr clock
	Will return 0 till clock is set setTime('time of day in seconds')
	Returns Hours, Minutes and Seconds as uint8_t
	Returns time '00:00:00' as String
*/

ESPClock::ESPClock()
{
	_baseMillis = 0;
	_baseSeconds = 0;
	_isSet = false;
}

String ESPClock::getTime() {
	char time[9];
	snprintf(time, sizeof(time), "%2u:%02u:%02u", getHours(), getMinutes(), getSeconds());
	return String(time);
}

uint8_t ESPClock::getHours() {
	uint8_t hours = (now()/3600);
	if(hours !=12) hours %= 12;
	return hours;
}

uint8_t ESPClock::getMinutes() {
	return (now()/60) % 60;
}

uint8_t ESPClock::getSeconds() {
	return now() % 60;
}

uint8_t ESPClock::setTime(uint32_t seconds) {
	if(seconds>SECONDS_PER_DAY) return 1;
	_baseSeconds = seconds;
	_baseMillis = millis();
	_isSet = true;
	return 0;
}

bool ESPClock::isSet() {
	return _isSet;
}

uint32_t ESPClock::now() {
	if(!_isSet) return 0; 		//time has not been set
	//current time in seconds
	return (((millis() - _baseMillis)/1000) + _baseSeconds);
}