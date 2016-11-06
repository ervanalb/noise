import numpy as np
import itertools
import operator
import math
import pyaudio

class Context:
    def __init__(self):
        self.chunk_size = 1024
        self.frame_rate = 44100
        self.bpm = None

    def chunks(self, t):
        if t.endswith("c"):
            return int(t[:-1])
        elif t.endswith("s"):
            return float(t[:-1]) * self.frame_rate / self.chunk_size
        elif t.endswith("b"):
            return float(t[:-1]) * 60 * self.frame_rate / self.chunk_size / self.bpm
        raise ValueError('Could not parse time value: "{}"'.format(t))

def pull(x):
    return next(x) if isinstance(x, Stream) else x

def reset(x):
    if isinstance(x, Stream):
        x.reset()

def stream_map(op):
    class StreamMap(Stream):
        def __init__(self, *args, **kwargs):
            super().__init__(**kwargs)
            self.args = args

        def reset(self):
            for a in self.args:
                reset(a)

        def __next__(self):
            return op(*[pull(a) for a in self.args])

    return StreamMap

class Stream:
    def __init__(self, ctx=None):
        self.ctx = ctx if ctx is not None else default_context

    def __iter__(self):
        return self

    def reset(self):
        pass

    def cut(self, t):
        return CutStream(self, t, ctx=self.ctx)

    def render(self):
        return data(list(self))

    def repeat(self, times=None):
        return RepeaterStream(self, times, ctx=self.ctx)

    def play(self):
        p = pyaudio.PyAudio()

        s = p.open(format=pyaudio.paFloat32,
                   channels=1,
                   rate=self.ctx.frame_rate,
                   output=True)

        try:
            for chunk in self:
                s.write(chunk.astype(np.float32).tostring())
        finally:
            s.stop_stream()
            s.close()
            p.terminate()

MAP_OPERATORS = ["__add__", "__sub__", "__mul__", "__truediv__"]

def make_operator(op):
    return lambda self, other: stream_map(op)(self, other, ctx=self.ctx)

for op in MAP_OPERATORS:
    setattr(Stream, op, make_operator(getattr(operator, op)))

class CutStream(Stream):
    def __init__(self, stream, t, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.stream = stream
        self.chunks = self.ctx.chunks(t)
        self.i = 0

    def reset(self):
        self.i = 0
        self.stream.reset()

    def __next__(self):
        if self.i >= self.chunks:
            raise StopIteration()
        self.i += 1
        return pull(self.stream)

class RepeaterStream(Stream):
    def __init__(self, stream, times=None, *args, **kwargs):
        self.stream = stream
        self.times = times
        self.i = 0

    def reset(self):
        reset(stream)
        self.i = 0

    def __next__(self):
        while True:
            try:
                return pull(self.stream)
            except StopIteration:
                self.i += 1
                if self.times is not None and self.i >= self.times:
                    raise
                else:
                    reset(self.stream)

class constant(Stream):
    def __init__(self, x, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.x = x

    def __next__(self):
        return self.x

class data(Stream):
    def __init__(self, x, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.x = x
        self.i = 0

    def reset(self):
        self.i = 0

    def __next__(self):
        if self.i >= len(self.x):
            raise StopIteration()
        result = self.x[self.i]
        self.i += 1
        return result

def waveform(fn):
    class Waveform(Stream):
        def __init__(self, freq, *args, **kwargs):
            super().__init__(*args, **kwargs)
            self.freq = freq
            self.t = 0

        def reset(self):
            self.t = 0
            reset(self.freq)

        def __next__(self):
            freq = pull(self.freq)
            if freq is None:
                return np.zeros(self.ctx.chunk_size, dtype=np.float)
            result = fn(freq * (self.t + np.arange(self.ctx.chunk_size) / self.ctx.frame_rate))
            self.t = (self.t + self.ctx.chunk_size / self.ctx.frame_rate) % (1 / freq)
            return result
    return Waveform

class cat(Stream):
    def __init__(self, *args, **kwargs):
        super().__init__(**kwargs)
        self.args = args
        self.i = 0

    def reset(self):
        for a in self.args:
            reset(a)
        self.i = 0

    def __next__(self):
        while True:
            try:
                return pull(self.args[self.i])
            except StopIteration:
                self.i += 1
                if self.i >= len(self.args):
                    raise

sine = waveform(lambda x: np.sin(2 * math.pi * x))
saw = waveform(lambda x: np.remainder(x, 1))

default_context = Context()
