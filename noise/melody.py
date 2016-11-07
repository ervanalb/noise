import noise.core

def string_to_freq(string, ctx=None):
    ctx = ctx if ctx is not None else noise.core.default_context

    l = 4
    last_pitch = 48
    letter_to_midi = {
        "c": 0,
        "d": 2,
        "e": 4,
        "f": 5,
        "g": 7,
        "a": 9,
        "b": 11,
        "r": None
    }

    def parse_note(note):
        nonlocal last_pitch
        nonlocal l

        pitch = letter_to_midi[note[0]]
        note = note[1:]
        while len(note) >= 2:
            if note[0:2] == "es":
                pitch -= 1
                note = note[2:]
            elif note[0:2] == "is":
                pitch += 1
                note = note[2:]
            else:
                break
        if pitch is not None:
            last_octave = last_pitch // 12
            octave = min(range(last_octave - 1, last_octave + 3), key=lambda o: abs(12 * o + pitch - last_pitch))
            pitch += 12 * octave
        if not note:
            if pitch is not None:
                last_pitch = pitch
            return (pitch, 4 / l)
        while len(note) >= 1:
            if note[0] == "'":
                pitch += 12
                note = note[1:]
            elif note[0] == ",":
                pitch -= 12
                note = note[1:]
            else:
                break
        if pitch is not None:
            last_pitch = pitch
        if not note:
            return (pitch, 4 / l)
        dots = note.index(".") if "." in note else len(note)
        l = int(note[0:dots])
        note = note[dots:]
        if not note:
            return (pitch, 4 / l)
        dots = 0
        while len(note) >= 1:
            if note[0] == ".":
                dots += 1
        if not note:
            extra = 1 - 2 ** -dots
            return (pitch, 4 / l + extra)
        raise ValueError('Unexpected characters in melody string: "{}"'.format(note))

    parsed = [parse_note(note.strip()) for note in string.split() if note.strip()]

    result = []
    t = 0
    for (n, l) in parsed:
        while t < l:
            freq = None if n is None else 2 ** ((n - 69) / 12) * 440
            result.append(freq)
            t += ctx.bpm * ctx.chunk_size / ctx.frame_rate / 60
        t -= l

    return noise.core.data(result)

class slide(noise.core.Stream):
    def __init__(self, stream, t, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.stream = stream
        self.alpha = 1  / (1 + self.ctx.chunks(t))
        self.last_v = None

    def reset(self):
        self.last_v = None
        noise.core.reset(self.stream)

    def __next__(self):
        v = noise.core.pull(self.stream)
        if self.last_v is None or v is None:
            self.last_v = v
            return self.last_v
        self.last_v = self.alpha * v + (1 - self.alpha) * self.last_v
        return self.last_v
