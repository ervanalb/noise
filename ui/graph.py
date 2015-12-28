import gi
gi.require_version('Gtk', '3.0')
from gi.repository import Gtk
from gi.repository import Gdk
#import cairo

class Graph(Gtk.DrawingArea):
    ZOOM_SPEED = 0.1
    LINE_WIDTH = 5

    def __init__(self):
        super().__init__()

        self.add_events(Gdk.EventMask.SCROLL_MASK
                     | Gdk.EventMask.POINTER_MOTION_MASK
                     | Gdk.EventMask.BUTTON_RELEASE_MASK
                     | Gdk.EventMask.BUTTON_PRESS_MASK
                     | Gdk.EventMask.SMOOTH_SCROLL_MASK)

        self.connect('draw', self.draw_event)
        self.connect('scroll-event', self.scroll_event)
        self.connect('button-press-event', self.button_down_event)
        self.connect('button-release-event', self.button_up_event)
        self.connect('motion-notify-event', self.motion_event)
        self.cx = 0
        self.cy = 0
        self.scale = 1
        self.children = []
        self.held_buttons = set()
        self.panning = False

    def draw_event(self, da, ctx):
        rect = self.get_allocation()
        width = rect.width
        height = rect.height

        ctx.rectangle(0.0, 0.0, width, height)
        ctx.set_source_rgb(255, 255, 255)
        ctx.fill()

        ctx.translate(self.cx, self.cy)
        ctx.scale(self.scale, self.scale)

        ctx.rectangle(100, 100, 200, 200)
        ctx.set_source_rgb(255, 0, 0)
        ctx.fill()

    def scroll_event(self, da, event):
        factor = 1 - event.delta_y * self.ZOOM_SPEED
        self.scale *= factor
        self.cx = (self.cx - event.x) * factor + event.x
        self.cy = (self.cy - event.y) * factor + event.y
        self.queue_draw()

    def get_child(self, da, event):
        for child in self.children:
            if child.mouse_hit(event):
                return child

    def button_down_event(self, da, event):
        # Debounce (why?)
        if event.button not in self.held_buttons:
            self.held_buttons.add(event.button)
        else:
            return

        if event.button == 1:
            pass
        elif event.button == 3:
            self.panning = True

        self.lx = event.x
        self.ly = event.y

    def motion_event(self, da, event):
        if self.panning:
            self.cx += event.x - self.lx
            self.cy += event.y - self.ly
            self.queue_draw()

        self.lx = event.x
        self.ly = event.y

    def button_up_event(self, da, event):
        # Debounce (why?)
        if event.button in self.held_buttons:
            self.held_buttons.remove(event.button)
        else:
            return

        if event.button == 1:
            pass
        elif event.button == 3:
            self.panning = False
            redraw = True

