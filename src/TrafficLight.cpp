#include <iostream>
#include <random>
#include <future>
#include "TrafficLight.h"

#define RANGE_MIN 4000
#define RANGE_MAX 6000

/* Implementation of class "MessageQueue" */

template <typename T>
T MessageQueue<T>::receive()
{
    // FP.5a : The method receive should use std::unique_lock<std::mutex> and _condition.wait() 
    // to wait for and receive new messages and pull them from the queue using move semantics. 
    // The received object should then be returned by the receive function. 
    std::unique_lock<std::mutex> ulck(_mutex);
    _condition.wait(ulck, [this]{return !_queue.empty();});

    T msg = std::move(_queue.front());
    _queue.pop_front();

    return msg;
}

template <typename T>
void MessageQueue<T>::send(T &&msg)
{
    // FP.4a : The method send should use the mechanisms std::lock_guard<std::mutex> 
    // as well as _condition.notify_one() to add a new message to the queue and afterwards send a notification.
    std::lock_guard<std::mutex> lck(_mutex);
    _queue.clear();
    _queue.emplace_back(std::move(msg));
    _condition.notify_one();
}

/* Implementation of class "TrafficLight" */

TrafficLight::TrafficLight()
{
    _currentPhase = TrafficLightPhase::red;
}

void TrafficLight::waitForGreen()
{
    // FP.5b : add the implementation of the method waitForGreen, in which an infinite while-loop 
    // runs and repeatedly calls the receive function on the message queue. 
    // Once it receives TrafficLightPhase::green, the method returns.
    while (true)
    {
        TrafficLightPhase msg = _queue.receive();
        if (msg == TrafficLightPhase::green) return;
    }
}

TrafficLightPhase TrafficLight::getCurrentPhase()
{
    return _currentPhase;
}

void TrafficLight::simulate()
{
    // FP.2b : Finally, the private method „cycleThroughPhases“ should be started in a thread when the public method „simulate“ is called. 
    // To do this, use the thread queue in the base class. 
    threads.emplace_back(std::thread(&TrafficLight::cycleThroughPhases, this));
}

// virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases()
{
    // FP.2a : Implement the function with an infinite loop that measures the time between two loop cycles 
    // and toggles the current phase of the traffic light between red and green and sends an update method 
    // to the message queue using move semantics. The cycle duration should be a random value between 4 and 6 seconds. 
    // Also, the while-loop should use std::this_thread::sleep_for to wait 1ms between two cycles. 

    std::random_device rd;
    std::default_random_engine generator(rd());
    std::uniform_real_distribution<double> distribution(RANGE_MIN, RANGE_MAX);
    double cycle_duration_ms = distribution(generator);
    //double cycle_duration_ms = static_cast<double>(RANGE_MIN + rand() % (RANGE_MAX - RANGE_MIN + 1));
    auto time_prev = std::chrono::steady_clock::now();
    while (true)
    {
        auto time_now = std::chrono::steady_clock::now();
        int time_difference = std::chrono::duration_cast<std::chrono::milliseconds>(time_now - time_prev).count();
        if (time_difference >= cycle_duration_ms)
        {
            _currentPhase = (_currentPhase == TrafficLightPhase::red) ? TrafficLightPhase::green : TrafficLightPhase::red;
            time_prev = time_now;
            //cycle_duration_ms = static_cast<double>(RANGE_MIN + rand() % (RANGE_MAX - RANGE_MIN + 1));
            cycle_duration_ms = distribution(generator);
            _queue.send(std::move(getCurrentPhase()));
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}