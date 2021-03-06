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
    std::unique_lock<std::mutex> ulck(_mutex);
    _condition.wait(ulck, [this]{return !_queue.empty();});

    T msg = std::move(_queue.front());
    _queue.pop_front();

    return msg;
}

template <typename T>
void MessageQueue<T>::send(T &&msg)
{
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
    threads.emplace_back(std::thread(&TrafficLight::cycleThroughPhases, this));
}

// virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases()
{
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
