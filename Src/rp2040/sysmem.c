
#ifdef RP2040_FEATHER
/* Includes */
#include <errno.h>
#include <stdint.h>

/**
 * current heap boundary
 */
static uint8_t *__sbrk_heap_end = NULL;


void *_sbrk(ptrdiff_t incr)
{
  extern uint8_t __end__; /* end of the predefined ram section (data, bss, ram_exec), defined in the linker script */
  extern uint8_t __StackBottom; /* minimum protected stack size, defined in the linker script */
  const uint8_t *max_heap = (uint8_t *)&__StackBottom;
  uint8_t *prev_heap_end;

  if (NULL == __sbrk_heap_end)
  {
    __sbrk_heap_end = &__end__;
  }

  /* Protect heap from growing into the reserved MSP stack */
  if (__sbrk_heap_end + incr > max_heap)
  {
    errno = ENOMEM;
    return (void *)-1;
  }

  prev_heap_end = __sbrk_heap_end;
  __sbrk_heap_end += incr;

  return (void *)prev_heap_end;
}
#endif
