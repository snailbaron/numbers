#pragma once

#include <queue>
#include "event.hpp"

class EventQueue
{
public:
    EventQueue();
    ~EventQueue();

    void Push(Event::Base *evt);
    Event::Base * Pull();
    void Clear();
    bool Empty() const;

private:
    std::queue<Event::Base *> _queue;
};