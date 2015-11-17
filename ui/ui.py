import gi
gi.require_version('Gtk', '3.0')
from gi.repository import Gtk
from gi.repository import Gdk
import cairo

class Block(object):
    def __init__(self, blocktype, x, y):
        self.blocktype = blocktype
        self.highlight = False
        self.x = x
        self.y = y

        self.setup = False

    def draw(self, ctx):
        if not self.setup:
            ctx.select_font_face("Helvetica", cairo.FONT_SLANT_NORMAL, cairo.FONT_WEIGHT_NORMAL)
            ctx.set_font_size(60)

            (self.text_x, self.text_y, self.text_width, self.text_height, dx, dy) = ctx.text_extents(self.blocktype)

            self.width = self.text_width + 60
            self.height = self.text_height + 60
            self.setup = True

        ctx.set_line_width(20)
        ctx.set_line_join(cairo.LINE_JOIN_ROUND)

        if self.highlight:
            ctx.set_source_rgb(100, 0, 0)
        else:
            ctx.set_source_rgb(0, 0, 0)

        ctx.move_to(self.x - self.width / 2, self.y - self.height / 2)
        ctx.rel_line_to(self.width, 0)
        ctx.rel_line_to(0, self.height)
        ctx.rel_line_to(-self.width, 0)
        ctx.close_path()

        ctx.stroke_preserve()
        ctx.set_source_rgb(200, 200, 200)
        ctx.fill()

        ctx.select_font_face("Helvetica", cairo.FONT_SLANT_NORMAL, cairo.FONT_WEIGHT_NORMAL)
        ctx.set_font_size(60)
        ctx.move_to(self.x - self.text_width / 2 - self.text_x, self.y - self.text_height / 2 - self.text_y)
        ctx.set_source_rgb(0, 0, 0)
        ctx.show_text(self.blocktype)

    def handle_motion(self, x, y):
        if not self.setup:
            return
        (x, y) = scaled_coords(x, y)
        new_highlight = (abs(x - self.x) < self.width / 2 and abs(y - self.y) < self.height / 2)
        if new_highlight != self.highlight:
            self.highlight = new_highlight
            drawingarea.queue_draw()

cx = 200
cy = 200
scale = 0.5
dragging = False

def scaled_coords(x, y):
    global cx, cy, scale
    return ((x - cx) / scale, (y - cy) / scale)

def draw(da, ctx):
    ctx.translate(cx, cy)
    ctx.scale(scale, scale)
    b1.draw(ctx)
    b2.draw(ctx)

def scroll(w, event):
    ZOOM_SPEED = 0.1
    global scale, drawingarea
    global cx, cy
    factor = 1 - event.delta_y * ZOOM_SPEED
    scale *= factor
    cx = (cx - event.x) * factor + event.x
    cy = (cy - event.y) * factor + event.y
    drawingarea.queue_draw()

def button_down(w, event):
    global dragging
    global lx, ly
    if event.button == 1:
        lx = event.x
        ly = event.y
        dragging = True

def button_up(w, event):
    global dragging
    if event.button == 1:
        dragging = False

def motion(w, event):
    global dragging
    global cx, cy, lx, ly
    global drawingarea

    global b1, b2
    b1.handle_motion(event.x, event.y)
    b2.handle_motion(event.x, event.y)

    if dragging:
        cx += event.x - lx
        cy += event.y - ly
        lx = event.x
        ly = event.y
        drawingarea.queue_draw()

def main():
    global win, drawingarea, b1, b2

    win = Gtk.Window()
    win.add_events(Gdk.EventMask.SCROLL_MASK | Gdk.EventMask.POINTER_MOTION_MASK)
    win.connect('destroy', lambda w: Gtk.main_quit())
    win.connect('button-press-event', button_down)
    win.connect('button-release-event', button_up)
    win.connect('scroll-event', scroll)
    win.connect('motion-notify-event', motion)

    win.set_default_size(450, 550)

    drawingarea = Gtk.DrawingArea()
    win.add(drawingarea)
    drawingarea.connect('draw', draw)

    b1 = Block("synth", 100, 100)
    b2 = Block("speaker", 400, 250)

    win.show_all()
    Gtk.main()

if __name__ == '__main__':
    main()
