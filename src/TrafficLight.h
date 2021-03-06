#ifndef TRAFFICLIGHT_H
#define TRAFFICLIGHT_H

#include <mutex>
#include <deque>
#include <condition_variable>
#include "TrafficObject.h"

// forward declarations to avoid include cycle
class Vehicle;


template <class T>
class MessageQueue
{
public:
void send(T&& msg);
T receive(void);

private:
std::deque<T>  _queue;
std::condition_variable _condition;
std::mutex _mutex;
};

enum TrafficLightPhase {red, green};

class TrafficLight : public TrafficObject
{
public:
    // constructor / desctructor
    TrafficLight();

    // getters / setters

    // typical behaviour methods
    void waitForGreen();
    void simulate();
    TrafficLightPhase getCurrentPhase();

private:
    void cycleThroughPhases();
    TrafficLightPhase _currentPhase;
    // typical behaviour methods

    MessageQueue<TrafficLightPhase> _queue;
    std::condition_variable _condition;
    std::mutex _mutex;
};

#endif
