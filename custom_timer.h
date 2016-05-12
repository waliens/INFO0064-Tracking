#ifndef CUSTOM_TIMER_HPP_DEFINED
#define CUSTOM_TIMER_HPP_DEFINED

/**
 * Init Timer0 so that it triggers an interrupt every 1ms
 */
void initTMR0For1kHz();

/**
 * Reset the Timer0 preload so that it triggers an interrupt after 1ms
 */
inline void startTMR0For1kHz();

/**
 * Disable Timer0
 */
inline void stopTMR0();

#endif // CUSTOM_TIMER_HPP_DEFINED