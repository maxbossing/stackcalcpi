#include <stdio.h>
#include <string.h>
#include <sys/unistd.h>

#include "pico/stdlib.h"

#define STACK_SIZE 32 /* Size of the Stack */
#define BUTTON_DELAY_MS 250 /* How long to delay between button presses */

#define BUTTON_MOD 0 /* Mod Button */

#define BUTTON_STACK_CELL_CLR    1 /* Clear Current Cell             (mod: Clear stack) */
#define BUTTON_STACK_POINTER_INC 2 /* Increment Stack Pointer by one (mod: Jump to first cell) */
#define BUTTON_STACK_POINTER_DEC 3 /* Decrement Stack Pointer by one (mod: Jump to last cell) */
#define BUTTON_STACK_CELL_INC    4 /* Increment Current Cell by one  (mod: Set cell to 0xff) */
#define BUTTON_STACK_CELL_DEC    5 /* Decrement Current Cell by one  (mod: Set cell to 0x00) */

#define BUTTON_ARITH_ADD 6 /* Arithmatic: + */
#define BUTTON_ARITH_SUB 7 /* Arithmatic: - */
#define BUTTON_ARITH_MUL 8 /* Arithmatic: * */

#define LED_DISPLAY_MOD  15 /* Lit if mod = true */
#define LED_DISPLAY_ERR  16 /* Lit when (0x00 > result || result > 0xff) */
#define LED_DISPLAY_1    17 /* First bit of result */
#define LED_DISPLAY_2    18 /* Second bit of result */
#define LED_DISPLAY_4    19 /* Third bit of result */
#define LED_DISPLAY_8    20 /* Fourth bit of result */
#define LED_DISPLAY_16   21 /* Fifth bit of result */
#define LED_DISPLAY_32   22 /* Sixth bit of result */
#define LED_DISPLAY_64   26 /* Seventh bit of result */
#define LED_DISPLAY_128  27 /* Eigth bit of result */
#define LED_DISPLAY_LIFE 25 /* Board LED, for seeing if system is alive */

/* Represents the Stack used for calculations */
typedef struct Stack {
  unsigned char data[STACK_SIZE];
  char pointer;
} Stack;

/* Init a Pin to be used as a LED */
#define PIN_INIT_LED(p) \
  gpio_init(p); \
  gpio_set_dir(p, GPIO_OUT) \

/* Init a Pin to be used as a button */
#define PIN_INIT_BUTTON(p) \
  gpio_init(p); \
  gpio_set_dir(p, GPIO_IN); \
  gpio_pull_up(p);

/* Write an unsigned char and the error value to the ouput LEDs */
#define WRITE_DISPLAY(num, err, mod) do {\
    gpio_put(LED_DISPLAY_ERR, err); \
    gpio_put(LED_DISPLAY_MOD, mod); \
    gpio_put(LED_DISPLAY_1, num >> 7 & 0x01); \
    gpio_put(LED_DISPLAY_2, num >> 6 & 0x01); \
    gpio_put(LED_DISPLAY_4, num >> 5 & 0x01); \
    gpio_put(LED_DISPLAY_8, num >> 4 & 0x01); \
    gpio_put(LED_DISPLAY_16, num >> 3 & 0x01); \
    gpio_put(LED_DISPLAY_32, num >> 2 & 0x01); \
    gpio_put(LED_DISPLAY_64, num >> 1 & 0x01); \
    gpio_put(LED_DISPLAY_128, num >> 0 & 0x01); \
  } while (false)

/* Increase the Stack Pointer by one, wrapping around if limit is reached */
#define STACK_ADVANCE(sp, ss) \
  if (sp < ss - 1) sp++; \
  else sp = 0;

/* Decrease the Stack Pointer by one, wrapping around if limit is reached */
#define STACK_BACK(sp, ss) \
  if (sp <= 0) sp = ss - 1; \
  else sp--;

/* Toggle a GPIO Pin */
#define GPIO_TOGGLE(p) gpio_put(p, !gpio_get(p))

[[noreturn]] void main(void) {
  stdio_init_all();

  /* Init LEDs */
  PIN_INIT_LED(LED_DISPLAY_1);
  PIN_INIT_LED(LED_DISPLAY_2);
  PIN_INIT_LED(LED_DISPLAY_4);
  PIN_INIT_LED(LED_DISPLAY_8);
  PIN_INIT_LED(LED_DISPLAY_16);
  PIN_INIT_LED(LED_DISPLAY_32);
  PIN_INIT_LED(LED_DISPLAY_64);
  PIN_INIT_LED(LED_DISPLAY_128);
  PIN_INIT_LED(LED_DISPLAY_ERR);
  PIN_INIT_LED(LED_DISPLAY_LIFE);

  /* Button initialization */
  PIN_INIT_BUTTON(BUTTON_MOD);
  PIN_INIT_BUTTON(BUTTON_STACK_CELL_CLR);
  PIN_INIT_BUTTON(BUTTON_STACK_POINTER_INC);
  PIN_INIT_BUTTON(BUTTON_STACK_POINTER_DEC);
  PIN_INIT_BUTTON(BUTTON_STACK_CELL_INC);
  PIN_INIT_BUTTON(BUTTON_STACK_CELL_DEC);
  PIN_INIT_BUTTON(BUTTON_ARITH_ADD);
  PIN_INIT_BUTTON(BUTTON_ARITH_SUB);
  PIN_INIT_BUTTON(BUTTON_ARITH_MUL);

  Stack stack = { .pointer = 0, .data = { 0 } };

  gpio_put(LED_DISPLAY_LIFE, true);

  bool error = false;
  bool mod = false;

  while (true) {

    WRITE_DISPLAY(stack.data[stack.pointer], error, mod);

    /* Mod Key */
    if (!gpio_get(BUTTON_MOD)) {
      GPIO_TOGGLE(LED_DISPLAY_LIFE);
      mod = !mod;
      sleep_ms(BUTTON_DELAY_MS);
      continue;
    }

    /* Clear */
    if (!gpio_get(BUTTON_STACK_CELL_CLR)) {
      GPIO_TOGGLE(LED_DISPLAY_LIFE);
      if (mod) {
        mod = false;
        /* Clear Stack */
        memset(&stack.data, 0x00, 32 * sizeof(unsigned char));
        sleep_ms(BUTTON_DELAY_MS);
        continue;
      }
      stack.data[stack.pointer] = 0x00;
      sleep_ms(BUTTON_DELAY_MS);
      continue;
    }

    /* Increment Stack pointer */
    if (!gpio_get(BUTTON_STACK_POINTER_INC)) {
      GPIO_TOGGLE(LED_DISPLAY_LIFE);
      if (mod) {
        mod = false;
        stack.pointer = 0;
        sleep_ms(BUTTON_DELAY_MS);
        continue;
      }
      STACK_ADVANCE(stack.pointer, STACK_SIZE);
      sleep_ms(BUTTON_DELAY_MS);
      continue;
    }

    /* Decrease Stack pointer */
    if (!gpio_get(BUTTON_STACK_POINTER_DEC)) {
      GPIO_TOGGLE(LED_DISPLAY_LIFE);
      if (mod) {
        mod = false;
        stack.pointer = STACK_SIZE - 1;
        sleep_ms(BUTTON_DELAY_MS);
        continue;
      }
      STACK_BACK(stack.pointer, STACK_SIZE);
      sleep_ms(BUTTON_DELAY_MS);
      continue;
    }

    /* Increase Stack cell */
    if (!gpio_get(BUTTON_STACK_CELL_INC)) {
      GPIO_TOGGLE(LED_DISPLAY_LIFE);
      if (mod) {
        mod = false;
        stack.data[stack.pointer] = 0xff;
        sleep_ms(BUTTON_DELAY_MS);
        continue;
      }
      stack.data[stack.pointer]++;
      sleep_ms(BUTTON_DELAY_MS);
      continue;
    }

    /* Decrease Stack cell */
    if (!gpio_get(BUTTON_STACK_CELL_DEC)) {
      GPIO_TOGGLE(LED_DISPLAY_LIFE);
      if (mod) {
        mod = false;
        stack.data[stack.pointer] = 0x00;
        sleep_ms(BUTTON_DELAY_MS);
        continue;
      }
      stack.data[stack.pointer]--;
      sleep_ms(BUTTON_DELAY_MS);
      continue;
    }

    /* Add two last Cells */
    if (!gpio_get(BUTTON_ARITH_ADD)) {
      GPIO_TOGGLE(LED_DISPLAY_LIFE);
      STACK_BACK(stack.pointer, STACK_SIZE);
      const unsigned char in_1 = stack.data[stack.pointer];
      STACK_ADVANCE(stack.pointer, STACK_SIZE);
      const unsigned char in_2 = stack.data[stack.pointer];
      STACK_ADVANCE(stack.pointer, STACK_SIZE);

      if (in_1 + in_2 > 0xff) {
        error = true;
        stack.data[stack.pointer] = 0xff;
        sleep_ms(BUTTON_DELAY_MS);
        continue;
      }

      stack.data[stack.pointer] = in_1 + in_2;
      sleep_ms(BUTTON_DELAY_MS);
      continue;
    }

    /* Subtract two last Cells */
    if (!gpio_get(BUTTON_ARITH_SUB)) {
      GPIO_TOGGLE(LED_DISPLAY_LIFE);
      STACK_BACK(stack.pointer, STACK_SIZE);
      const unsigned char in_1 = stack.data[stack.pointer];
      STACK_ADVANCE(stack.pointer, STACK_SIZE);
      const unsigned char in_2 = stack.data[stack.pointer];
      STACK_ADVANCE(stack.pointer, STACK_SIZE);

      if (in_1 - in_2 < 0x00) {
        error = true;
        stack.data[stack.pointer] = 0x00;
        sleep_ms(BUTTON_DELAY_MS);
        continue;
      }

      stack.data[stack.pointer] = in_1 - in_2;
      sleep_ms(BUTTON_DELAY_MS);
      continue;
    }

    /* Multiply two last cells */
    if (!gpio_get(BUTTON_ARITH_MUL)) {
      GPIO_TOGGLE(LED_DISPLAY_LIFE);
      STACK_BACK(stack.pointer, STACK_SIZE);
      const unsigned char in_1 = stack.data[stack.pointer];
      STACK_ADVANCE(stack.pointer, STACK_SIZE);
      const unsigned char in_2 = stack.data[stack.pointer];
      STACK_ADVANCE(stack.pointer, STACK_SIZE);

      if (in_1 * in_2 > 0xff) {
        error = true;
        stack.data[stack.pointer] = 0xff;
        sleep_ms(BUTTON_DELAY_MS);
        continue;
      }

      stack.data[stack.pointer] = in_1 * in_2;
      sleep_ms(BUTTON_DELAY_MS);
      continue;
    }
  }
}