import gi
gi.require_version('Gtk', '3.0')
from gi.repository import Gtk
from gi.repository import Gdk
import cairo

class Block(object):
    def __init__(self, blocktype):
        self.blocktype = blocktype

    def draw(self, ctx):
        ctx.select_font_face("Helvetica", cairo.FONT_SLANT_NORMAL, cairo.FONT_WEIGHT_NORMAL)
        ctx.set_font_size(60)

        (text_x, text_y, text_width, text_height, dx, dy) = ctx.text_extents(self.blocktype)

        rect_width = text_width + 60
        rect_height = text_height + 60

        ctx.set_line_width(20)
        ctx.set_line_join(cairo.LINE_JOIN_ROUND)
        ctx.set_source_rgb(0, 0, 0)

        ctx.move_to(-rect_width / 2, -rect_height / 2)
        ctx.rel_line_to(rect_width, 0)
        ctx.rel_line_to(0, rect_height)
        ctx.rel_line_to(-rect_width, 0)
        ctx.close_path()

        ctx.stroke_preserve()
        ctx.set_source_rgb(200, 200, 200)
        ctx.fill()

        ctx.move_to(-text_width / 2 - text_x, -text_height / 2 - text_y)
        ctx.set_source_rgb(0, 0, 0)
        ctx.show_text(self.blocktype)

b = Block("synth")

cx = 200
cy = 200
scale = 0.5
dragging = False

def draw(da, ctx):
    ctx.translate(cx, cy)
    ctx.scale(scale, scale)
    b.draw(ctx)

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
    if dragging:
        event.x, event.y
        cx += event.x - lx
        cy += event.y - ly
        lx = event.x
        ly = event.y
        drawingarea.queue_draw()

def main():
    global win, drawingarea

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

    win.show_all()
    Gtk.main()

if __name__ == '__main__':
    main()
