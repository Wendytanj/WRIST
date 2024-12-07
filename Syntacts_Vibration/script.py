from syntacts import *
from time import sleep

channels = [1,0,5,2,4]

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
sequence << signal1

for channel in channels:
    s.play(channel, sequence)

sleep(sequence.length + 0.1)
s.close()
