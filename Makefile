# Raspberry Pi C言語プログラム用Makefile

CC = gcc
CFLAGS = -Wall -Wextra -O2 -lwiringPi -li2c
TARGETS = led_control button_input temperature_sensor

all: $(TARGETS)

led_control: led_control.c
	$(CC) $(CFLAGS) -o $@ $<

button_input: button_input.c
	$(CC) $(CFLAGS) -o $@ $<

temperature_sensor: temperature_sensor.c
	$(CC) $(CFLAGS) -li2c -o $@ $<

clean:
	rm -f $(TARGETS) *.o

.PHONY: all clean
