import gi
gi.require_version('Gtk', '3.0')
from gi.repository import Gtk
from gi.repository import Gdk
import cairo
import nz
import elem

class Graph(Gtk.DrawingArea):
    ZOOM_SPEED = 0.1
    LINE_WIDTH = 5
    BG_COLOR = (200, 200, 200)

    def __init__(self):
        super().__init__()

        self.add_events(Gdk.EventMask.SCROLL_MASK
                     | Gdk.EventMask.POINTER_MOTION_MASK
                     | Gdk.EventMask.BUTTON_RELEASE_MASK
                     | Gdk.EventMask.BUTTON_PRESS_MASK
                     | Gdk.EventMask.SMOOTH_SCROLL_MASK
                     | Gdk.EventMask.KEY_RELEASE_MASK
                     | Gdk.EventMask.KEY_PRESS_MASK)
        self.set_can_focus(True)

        self.connect('draw', self.draw_event)
        self.connect('scroll-event', self.scroll_event)
        self.connect('button-press-event', self.button_down_event)
        self.connect('button-release-event', self.button_up_event)
        self.connect('motion-notify-event', self.motion_event)
        self.connect('key-press-event', self.key_down_event)
        self.connect('key-release-event', self.key_up_event)

        self.cx = 0
        self.cy = 0
        self.scale = 1
        self.children = []
        self.selected = []
        self.held_buttons = set()
        self.panning = False
        self.dragging = False
        self.mouse_owner = None

        self.context = nz.Context()

    def __enter__(self):
        self.context.__enter__()
        self.context.load_lib("../nzlib/nzstd.so", "nzlib.std")

    def __exit__(self, type, value, tb):
        self.context.__exit__(type, value, tb)

    ## Event Handlers

    def draw_event(self, da, ctx):
        self.stored_ctx = ctx
        rect = self.get_allocation()
        width = rect.width
        height = rect.height

        #ctx.rectangle(0.0, 0.0, width, height)
        #ctx.set_source_rgb(*self.BG_COLOR)
        #ctx.fill()

        ctx.translate(self.cx, self.cy)
        ctx.scale(self.scale, self.scale)

        for child in self.children:
            ctx.save()
            ctx.translate(child.x, child.y)
            child.draw(ctx, selected = child in self.selected)
            ctx.restore()

    def scroll_event(self, da, event):
        factor = 1 - event.delta_y * self.ZOOM_SPEED
        self.scale *= factor
        self.cx = (self.cx - event.x) * factor + event.x
        self.cy = (self.cy - event.y) * factor + event.y
        self.queue_draw()

    def get_child(self, x, y):
        for child in self.children:
            if child.hit_test(x - child.x, y - child.y):
                return child

    def button_down_event(self, da, event):
        # Debounce (why?)
        if event.button not in self.held_buttons:
            self.held_buttons.add(event.button)
        else:
            return

        if event.button == 1:
            self.selected = []
            (ctx_x, ctx_y) = self.conv_screen_pt(event.x, event.y)
            hit_child = self.get_child(ctx_x, ctx_y)
            if hit_child is not None:
                self.selected = [hit_child]
                if hit_child.button_down(ctx_x - hit_child.x,
                                         ctx_y - hit_child.y):
                    self.mouse_owner = hit_child
                else:
                    self.dragging = True
            self.queue_draw()
        elif event.button == 3:
            self.panning = True

        self.lx = event.x
        self.ly = event.y

    def motion_event(self, da, event):
        (ctx_x, ctx_y) = self.conv_screen_pt(event.x, event.y)
        if self.mouse_owner is not None:
            self.mouse_owner.mouse_move(self, ctx_x - mouse_owner.x,
                                        ctx_y - mouse_owner.y)
        elif self.panning:
            self.cx += event.x - self.lx
            self.cy += event.y - self.ly
            self.queue_draw()
        elif self.dragging:
            (ctx_dx, ctx_dy) = self.conv_screen_vec(event.x - self.lx,
                                                    event.y - self.ly)
            for child in self.selected:
                child.x += ctx_dx
                child.y += ctx_dy
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
            self.dragging = False
            self.mouse_owner = None
        elif event.button == 3:
            self.panning = False
            redraw = True

    def key_down_event(self, da, event):
        if event.keyval == Gdk.KEY_Delete:
            for child in self.selected:
                child.delete()
                self.children.remove(child)
            self.selected = []
            self.queue_draw()
        elif event.keyval == Gdk.KEY_a:
            self.add_block()
        else:
            for child in self.selected:
                child.key_down(event.keyval)

    def key_up_event(self, da, event):
        pass

    ## Useful things

    def add_block(self):
        constructor = self.dialog("Block:")
        if constructor is None:
            return
        try:
            b = self.context.create_block(constructor)
            self.children.append(elem.Block(b, self))
            self.queue_draw()
        except nz.NoiseError as e:
            self.msg(str(e))

    ## UTIL

    def conv_screen_pt(self, x, y):
        return ((x - self.cx) / self.scale, (y - self.cy) / self.scale)

    def conv_screen_vec(self, dx, dy):
        return (dx / self.scale, dy / self.scale)

    def dialog(self, prompt):
        dialogWindow = Gtk.MessageDialog(None,
                              Gtk.DialogFlags.MODAL | Gtk.DialogFlags.DESTROY_WITH_PARENT,
                              Gtk.MessageType.QUESTION,
                              Gtk.ButtonsType.OK_CANCEL,
                              prompt)

        dialogBox = dialogWindow.get_content_area()
        userEntry = Gtk.Entry()
        userEntry.set_size_request(250,0)
        dialogBox.pack_end(userEntry, False, False, 0)

        userEntry.set_activates_default(True)
        okButton = dialogWindow.get_widget_for_response(response_id=Gtk.ResponseType.OK)
        okButton.set_can_default(True)
        okButton.grab_default()

        dialogWindow.show_all()
        response = dialogWindow.run()
        text = userEntry.get_text()
        dialogWindow.destroy()
        if (response == Gtk.ResponseType.OK) and (text != ''):
            return text

    def msg(self, prompt):
        dialogWindow = Gtk.MessageDialog(None,
                              Gtk.DialogFlags.MODAL | Gtk.DialogFlags.DESTROY_WITH_PARENT,
                              Gtk.MessageType.QUESTION,
                              Gtk.ButtonsType.OK,
                              prompt)

        dialogWindow.show_all()
        response = dialogWindow.run()
        dialogWindow.destroy()
