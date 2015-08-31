#include "event_queue.hpp"

EventQueue::EventQueue()
{ }

EventQueue::~EventQueue()
{ }

void EventQueue::Push(Event::Base evt)
{
    _queue.push(evt);
}

bool EventQueue::Pull(Event::Base *evt)
{
    if (_queue.empty())
        return false;

    (*evt) = _queue.front();
    _queue.pop();
    return true;
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