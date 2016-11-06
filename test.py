from noise import *

default_context.bpm = 120

pulse = cat(constant(440).cut("1b"), constant(None).cut("1b")).repeat()

s = (saw(pulse) * 0.5)

s.play()
