#ifndef TIMER_H
#define TIMER_H

class Timer {
  private:
    const unsigned long duration;
    void (*runTask)();

    bool started;
    bool infinite;
    unsigned long runAtTimestamp; // Irrelevant when started is false

    void start(bool infinite);

  public:
    Timer(unsigned long duration, void (*runTask)());
    void loop();
    void startOnce();
    void startInfinite();
    void stop();
};

#endif
