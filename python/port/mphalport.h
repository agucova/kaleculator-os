#ifndef PYTHON_MPHALPORT_H
#define PYTHON_MPHALPORT_H

extern int mp_interrupt_char;
void mp_hal_set_interrupt_char(int c);
void mp_keyboard_interrupt(void);

#endif
