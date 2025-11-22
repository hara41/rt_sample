# Raspberry Pi C言語プログラム用Makefile (PREEMPT_RT対応)

CC = gcc
CFLAGS = -Wall -Wextra -O2 -D_GNU_SOURCE
LDFLAGS_GPIO = -llgpio -lrt -lpthread
LDFLAGS_I2C = -li2c
TARGETS = led_control button_input temperature_sensor

all: $(TARGETS)

led_control: led_control.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS_GPIO)

button_input: button_input.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS_GPIO)

temperature_sensor: temperature_sensor.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS_I2C)

clean:
	rm -f $(TARGETS) *.o

.PHONY: all clean
