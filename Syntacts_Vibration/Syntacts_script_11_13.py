from syntacts import *
from time import sleep

<<<<<<<< HEAD:Syntacts_Vibration/Syntacts_script_11_13.py
"""
Custom pattern script for inter-team presentatoins on 11/13/2024.
- Patterns include:
    - individual sequential tactor triggering for different durations of time
    - All triggered vibration (sine wave at 170Hz)
"""
channels = [1,0,4,5,3]
========
channels = [0,1,2,3,4]
>>>>>>>> 07b8b42b20c108887f160c5a23444d5005877c3b:Syntacts_Vibration/script.py


s = Session()
s.open()
num_channels = s.channel_count

def create_tactor_signal(frequency, duration):
    amplitude = 1.0
    decay_rate = 6.9
    return Sine(frequency) * ExponentialDecay(amplitude, decay_rate) * Envelope(duration)

def play_individual(duration):
    for channel in channels:
        signal = create_tactor_signal(170, duration)
        s.play(channel, signal)
        sleep(signal.length)
        s.stop(channel)

# Part 1: Play a pulse with exponential decay on each tactor individually
play_individual(1.0)
play_individual(0.5)
play_individual(0.2)
play_individual(0.1)

print("Playing custom pattern")
max_duration = 1.0
signal1 = Sine(170) * Envelope(0.75)

sequence = Sequence()
sequence << signal1 << signal1

for channel in channels:
    s.play(channel, sequence)

sleep(sequence.length + 0.1)
s.close()
