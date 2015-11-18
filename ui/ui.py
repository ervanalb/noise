import gi
gi.require_version('Gtk', '3.0')
from gi.repository import Gtk
from gi.repository import Gdk
import cairo

class Graph(Gtk.DrawingArea):
    ZOOM_SPEED = 0.1

    def __init__(self, win, *args, **kwargs):
        super(Graph, self).__init__(*args, **kwargs)
        win.add_events(Gdk.EventMask.SCROLL_MASK | Gdk.EventMask.POINTER_MOTION_MASK)
        win.connect('button-press-event', self.button_down_event)
        win.connect('button-release-event', self.button_up_event)
        win.connect('scroll-event', self.scroll_event)
        win.connect('motion-notify-event', self.motion_event)
        self.connect('draw', self.draw_event)
        win.add(self)

        self.cx = 0
        self.cy = 0
        self.scale = 1

        self.panning = False
        self.dragging = False
        self.mouse_handler = None
        self.selected = ()

        self.blocks = []

    def conv_screen_coords(self, x, y):
        return ((x - self.cx) / self.scale, (y - self.cy) / self.scale)

    def draw_event(self, da, ctx):
        ctx.translate(self.cx, self.cy)
        ctx.scale(self.scale, self.scale)

        for block in self.blocks:
            block.draw(ctx, selected = block in self.selected)

    def scroll_event(self, win, event):
        factor = 1 - event.delta_y * self.ZOOM_SPEED
        self.scale *= factor
        self.cx = (self.cx - event.x) * factor + event.x
        self.cy = (self.cy - event.y) * factor + event.y
        self.queue_draw()

    def button_down_event(self, win, event):
        if event.button == 1:
            handled = False
            selected = ()
            for block in self.blocks:
                if block.mouse_hit(event):
                    selected = (block,)
                    if block.handle_button_down(event):
                        self.mouse_handler = block
                    else:
                        self.lx = event.x
                        self.ly = event.y
                        self.dragging = True
                    break
            if selected != self.selected:
                self.selected = selected
                self.queue_draw()
        elif event.button == 3:
            self.lx = event.x
            self.ly = event.y
            self.panning = True

    def button_up_event(self, win, event):
        if event.button == 1:
            if self.mouse_handler is not None:
                self.mouse_handler.handle_button_up(event)
                self.mouse_handler = None
            elif self.dragging:
                self.dragging = False
        if event.button == 3:
            self.panning = False

    def motion_event(self, win, event):
        if self.panning:
            self.cx += event.x - self.lx
            self.cy += event.y - self.ly
            self.lx = event.x
            self.ly = event.y
            self.queue_draw()
        elif self.mouse_handler is not None:
            self.mouse_handler.handle_motion(event)
        elif self.dragging:
            dx = (event.x - self.lx) / self.scale
            dy = (event.y - self.ly) / self.scale
            self.lx = event.x
            self.ly = event.y
            for obj in self.selected:
                if isinstance(obj, Block):
                    obj.x += dx
                    obj.y += dy
            self.queue_draw()

class Block(object):
    def __init__(self, parent, blocktype, x, y):
        self.parent = parent
        self.blocktype = blocktype
        self.x = x
        self.y = y

        self.setup = False

    def draw(self, ctx, selected = False):
        if not self.setup:
            ctx.select_font_face("Helvetica", cairo.FONT_SLANT_NORMAL, cairo.FONT_WEIGHT_NORMAL)
            ctx.set_font_size(60)

            (self.text_x, self.text_y, self.text_width, self.text_height, dx, dy) = ctx.text_extents(self.blocktype)

            self.width = self.text_width + 60
            self.height = self.text_height + 60
            self.setup = True

        ctx.set_line_width(20)
        ctx.set_line_join(cairo.LINE_JOIN_ROUND)

        if selected:
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

    def handle_motion(self, event):
        pass

    def handle_button_down(self, event):
        pass

    def handle_button_up(self, event):
        pass

    def mouse_hit(self, event):
        if not self.setup:
            return False

        (x, y) = self.parent.conv_screen_coords(event.x, event.y)
        return abs(x - self.x) < self.width / 2 and abs(y - self.y) < self.height / 2

def main():
    win = Gtk.Window()
    win.set_default_size(450, 550)
    win.connect('destroy', lambda w: Gtk.main_quit())

    nzgraph = Graph(win)

    b1 = Block(nzgraph, "synth", 0, 0)
    b2 = Block(nzgraph, "speaker", 400, 250)

    nzgraph.blocks = [b1, b2]

    win.show_all()
    Gtk.main()

if __name__ == '__main__':
    main()
