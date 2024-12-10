from syntacts import *
from time import sleep

channels = [0]

s = Session()
s.open()
num_channels = s.channel_count



signal1 = Sine(20) * Envelope(0.75)
signal2 = Sine(40) * Envelope(0.75)
signal3 = Sine(60) * Envelope(0.75)
signal4 = Sine(80)* Envelope(0.75)
signal5 = Sine(100) * Envelope(0.75)
signal6 = Sine(120) * Envelope(0.75)
signal7 = Sine(140) * Envelope(0.75)
signal8 = Sine(160) * Envelope(0.75)
signal9 = Sine(180)* Envelope(0.75)
signal10 = Sine(200) * Envelope(0.75)

sequence = Sequence()
sequence << signal3 << signal4 << signal5 << signal6 << signal7 << signal8 << signal9 << signal10

for channel in channels:
    s.play(channel, sequence)
    
sleep(sequence.length + 0.1)
s.close()
