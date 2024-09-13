#ifndef DEMO_MODE_H
#define DEMO_MODE_H

#include "src/libs/action-chain/action-chain.h"

bool isDemoMode = false;

class DemoModeAwareWaitAction : public WaitAction
{
  private:
    const unsigned long normalWaitDuration;
    const unsigned long demoWaitDuration;

  public:
    DemoModeAwareWaitAction(
      const unsigned long normalWaitDuration,
      const unsigned long demoWaitDuration
    )
      : WaitAction(0)
      , normalWaitDuration(normalWaitDuration)
      , demoWaitDuration(demoWaitDuration)
    {
    }

    unsigned long duration() const {
      return isDemoMode ? demoWaitDuration : normalWaitDuration;
    }
};

#endif
