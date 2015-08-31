#include "event_queue.hpp"

EventQueue::EventQueue()
{ }

EventQueue::~EventQueue()
{ }

void EventQueue::Push(Event::Base *evt)
{
    _queue.push(evt);
}

Event::Base * EventQueue::Pull()
{
    if (_queue.empty())
        return nullptr;

    Event::Base *evt = _queue.front();
    _queue.pop();
    return evt;
}

void EventQueue::Clear()
{
    while (!_queue.empty())
        _queue.pop();
}

bool EventQueue::Empty() const
{
    return _queue.empty();
}