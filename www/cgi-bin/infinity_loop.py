#!/usr/bin/ python3

import time
import sys

print("Starting infinite loop...")

while True:
    print("Looping...")
    sys.stdout.flush()
    time.sleep(1)  # Sleep for 1 second to avoid busy-waiting