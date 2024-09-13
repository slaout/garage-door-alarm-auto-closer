#ifndef STATE_MACHINE_H
#define STATE_MACHINE_H

class State {
  private:
    State()
    {
    }

  public:
    static const State ANY;

    void (*enter)();

    State(void (*enter)())
      : enter(enter)
    {
    }
};

const State State::ANY = State();

struct Event
{
};

class Transition {
  public:
    const State *from;
    const Event *event;
    const State *to;

    Transition(const State *from, const Event *event, const State *to)
      : from(from)
      , event(event)
      , to(to)
    {
    }
};

struct Transitions {
  const unsigned int length;
  const Transition **values;
};

class StateMachine {
  private:
    const Transitions *transitions;

    const State *currentState = nullptr;

    const State *beforeLastState = nullptr;
    const State *lastState = nullptr;

    const State *getNewStateFor(const Event *event)
    {
      for (unsigned int i = 0; i < transitions->length; i++) {
        const Transition *transition = transitions->values[i];
        if ((transition->from == currentState || transition->from == &State::ANY) && transition->event == event) {
          return transition->to;
        }
      }

      return nullptr;
    }

    void enter(const State *state)
    {
      beforeLastState = lastState;
      lastState = currentState;

      currentState = state;
      state->enter();
    }

  public:
    StateMachine(const Transitions *transitions)
      : transitions(transitions)
    {
    }

    void start(const State *state)
    {
      if (currentState == nullptr) {
        enter(state);
      }
    }

    void handleEvent(const Event *event)
    {
      const State *newState = getNewStateFor(event);
      if (newState != nullptr && newState != currentState) {
        enter(newState);
      }
    }

    const State *getBeforeLastState() const
    {
      return beforeLastState;
    };

    const State *getLastState() const
    {
      return lastState;
    }

    const State *getCurrentState() const
    {
      return currentState;
    }
};

#endif
