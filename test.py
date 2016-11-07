from noise import *

default_context.chunk_size = 128

melody = string_to_freq("f'8 ees' r c g4 aes8 r f bes c bes f4 r " +
                        "f8 ees' r c g4 aes8 f c' ees r c f4 r")

cmel = string_to_freq("f'8 f r f ees4 ees8 r ees bes bes bes f'4 r " +
                        "f8 f r f ees4 ees8 ees ees aes r aes des4 r")

s = saw(slide(melody, "0.02b"))
s2 = saw(slide(cmel, "0.02b"))

(s * 0.7 + s2 * 0.3).repeat(2).play()
