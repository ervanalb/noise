import gi
gi.require_version('Gtk', '3.0')
from gi.repository import Gtk
from gi.repository import Gdk
import cairo
import math
import nz

class Graph(Gtk.DrawingArea):
    ZOOM_SPEED = 0.1
    LINE_WIDTH = 5

    def __init__(self, win, nz_context, *args, **kwargs):
        super(Graph, self).__init__(*args, **kwargs)
        self.win = win
        win.add_events(Gdk.EventMask.SCROLL_MASK 
                     | Gdk.EventMask.POINTER_MOTION_MASK 
                     | Gdk.EventMask.BUTTON_RELEASE_MASK 
                     | Gdk.EventMask.BUTTON_PRESS_MASK)
        win.connect('button-press-event', self.button_down_event)
        win.connect('button-release-event', self.button_up_event)
        win.connect('scroll-event', self.scroll_event)
        win.connect('motion-notify-event', self.motion_event)
        win.connect('key-press-event', self.key_down_event)
        self.connect('draw', self.draw_event)
        win.add(self)

        self.cx = 0
        self.cy = 0
        self.scale = 1

        self.held_buttons = set()
        self.panning = False
        self.dragging = False
        self.mouse_handler = None
        self.selected = ()
        self.output_term = None
        self.input_term = None

        self.blocks = []
        self.connections = []

        self.nz_graph = nz.Graph(nz_context)
        self.block_counter = 1

    def conv_screen_coords(self, x, y):
        return ((x - self.cx) / self.scale, (y - self.cy) / self.scale)

    def draw_event(self, da, ctx):
        ctx.translate(self.cx, self.cy)
        ctx.scale(self.scale, self.scale)

        for block in self.blocks:
            block.draw(ctx, selected = block in self.selected)

        for conn in self.connections:
            conn.draw(ctx, selected = conn in self.selected)

        if self.input_term is not None:
            startp = self.conv_screen_coords(self.lx, self.ly)
            (block, index) = self.input_term
            endp = block.input_location(index)
            self.draw_pending_connection(ctx, startp, endp)
        elif self.output_term is not None:
            (block, index) = self.output_term
            startp = block.output_location(index)
            endp = self.conv_screen_coords(self.lx, self.ly)
            self.draw_pending_connection(ctx, startp, endp)

    def draw_pending_connection(self, ctx, startp, endp):
        ctx.set_line_width(self.LINE_WIDTH)
        ctx.set_line_cap(cairo.LINE_CAP_ROUND);
        ctx.set_source_rgb(0, 0, 0)
        ctx.new_path()
        ctx.move_to(*startp)
        ctx.line_to(*endp)
        ctx.stroke()

    def scroll_event(self, win, event):
        factor = 1 - event.delta_y * self.ZOOM_SPEED
        self.scale *= factor
        self.cx = (self.cx - event.x) * factor + event.x
        self.cy = (self.cy - event.y) * factor + event.y
        self.queue_draw()

    def button_down_event(self, win, event):
        # Debounce (why?)
        if event.button not in self.held_buttons:
            self.held_buttons.add(event.button)
        else:
            return

        redraw = False
        if event.button == 1:
            handled = False
            selected = ()
            input_term = self.input_term
            output_term = self.output_term
            hit_term = False
            for block in self.blocks:
                input_index = block.mouse_hit_input(event)
                output_index = block.mouse_hit_output(event)
                if input_index is not None:
                    input_term = (block, input_index)
                    hit_term = True
                elif output_index is not None:
                    output_term = (block, output_index)
                    hit_term = True
                elif block.mouse_hit(event):
                    selected = (block,)
                    if block.handle_button_down(event):
                        self.mouse_handler = block
                    else:
                        self.dragging = True

            for connection in self.connections:
                if connection.mouse_hit(event):
                    selected = (connection,)

            if not hit_term:
                input_term = None
                output_term = None

            if selected != self.selected:
                self.selected = selected
                redraw = True
            if input_term != self.input_term or output_term != self.output_term:
                self.input_term = input_term
                self.output_term = output_term
                redraw = True
            if self.input_term is not None and self.output_term is not None:
                self.add_connection(*(self.output_term + self.input_term))
                self.input_term = None
                self.output_term = None
                redraw = True
        elif event.button == 3:
            self.panning = True
            redraw = True

        self.lx = event.x
        self.ly = event.y

        if redraw:
            self.queue_draw()

    def button_up_event(self, win, event):
        # Debounce (why?)
        if event.button in self.held_buttons:
            self.held_buttons.remove(event.button)
        else:
            return

        redraw = False
        if event.button == 1:
            if self.mouse_handler is not None:
                self.mouse_handler.handle_button_up(event)
                self.mouse_handler = None
            elif self.dragging:
                self.dragging = False
                redraw = True
            elif self.input_term is not None or self.output_term is not None:
                input_term = self.input_term
                output_term = self.output_term
                hit_term = False
                for block in self.blocks:
                    input_index = block.mouse_hit_input(event)
                    output_index = block.mouse_hit_output(event)
                    if input_index is not None:
                        input_term = (block, input_index)
                        hit_term = True
                    elif output_index is not None:
                        output_term = (block, output_index)
                        hit_term = True
                if not hit_term:
                    input_term = None
                    output_term = None
                if input_term != self.input_term or output_term != self.output_term:
                    self.input_term = input_term
                    self.output_term = output_term
                    redraw = True
                if self.input_term is not None and self.output_term is not None:
                    self.add_connection(*(self.output_term + self.input_term))
                    self.input_term = None
                    self.output_term = None
        elif event.button == 3:
            self.panning = False
            redraw = True

        self.lx = event.x
        self.ly = event.y

        if redraw:
            self.queue_draw()

    def motion_event(self, win, event):
        redraw = False
        if self.panning:
            self.cx += event.x - self.lx
            self.cy += event.y - self.ly
            redraw = True
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
            redraw = True
        elif self.input_term is not None or self.output_term is not None:
            redraw = True

        if redraw:
            self.queue_draw()

        self.lx = event.x
        self.ly = event.y

    def key_down_event(self, win, event):
        if event.keyval == Gdk.KEY_Delete:
            for item in self.selected:
                item.delete()
            self.queue_draw()
        elif event.keyval == Gdk.KEY_a:
            self.add_block()
        elif event.keyval == Gdk.KEY_s:
            for obj in self.selected:
                if isinstance(obj, Block) and obj.blocktype == 'pa':
                    if not hasattr(obj, "playing") or not obj.playing:
                        nz.pa.start(self.nz_graph.block_handle(obj.name))
                        obj.playing = True
                    else:
                        nz.pa.stop(self.nz_graph.block_handle(obj.name))
                        obj.playing = False

    def add_connection(self, output_block, output_index, input_block, input_index):
        try:
            self.nz_graph.connect(output_block.name, output_block.outputs[output_index], input_block.name, input_block.inputs[input_index])
            self.connections.append(Connection(self, output_block, output_index, input_block, input_index))
        except nz.NoiseError as e:
            self.msg(str(e))

    def add_block(self):
        constructor = self.dialog("Block:")
        if constructor is None:
            return
        try:
            self.blocks.append(Block(self, "Block{0}".format(self.block_counter, constructor), constructor))
            self.block_counter += 1
            self.queue_draw()
        except nz.NoiseError as e:
            self.msg(str(e))

    def dialog(self, prompt):
        dialogWindow = Gtk.MessageDialog(self.win,
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
        dialogWindow = Gtk.MessageDialog(self.win,
                              Gtk.DialogFlags.MODAL | Gtk.DialogFlags.DESTROY_WITH_PARENT,
                              Gtk.MessageType.QUESTION,
                              Gtk.ButtonsType.OK,
                              prompt)

        dialogWindow.show_all()
        response = dialogWindow.run()
        dialogWindow.destroy()

class Connection(object):
    def __init__(self, parent, output_block, output_index, input_block, input_index):
        self.parent = parent
        self.output_block = output_block
        self.output_index = output_index
        self.input_block = input_block
        self.input_index = input_index
        self.setup = False

    def delete(self):
        self.parent.nz_graph.disconnect(self.output_block.name, self.output_block.outputs[self.output_index], self.input_block.name, self.input_block.inputs[self.input_index])
        self.parent.connections.remove(self)

    def draw(self, ctx, selected = False):
        if not self.setup:
            self.setup = True

        startp = self.output_block.output_location(self.output_index)
        endp = self.input_block.input_location(self.input_index)

        ctx.new_path()
        ctx.move_to(*startp)
        ctx.line_to(*endp)

        self.hitpath = ctx.copy_path_flat()
        self.ctx = ctx

        if selected:
            ctx.set_line_width(self.parent.LINE_WIDTH * 3)
            ctx.set_line_cap(cairo.LINE_CAP_ROUND)
            ctx.set_source_rgb(200, 0, 0)
            ctx.stroke_preserve()

        ctx.set_line_width(self.parent.LINE_WIDTH)
        ctx.set_line_cap(cairo.LINE_CAP_ROUND)
        ctx.set_source_rgb(0, 0, 0)
        ctx.stroke()

    def mouse_hit(self, event):
        (x, y) = self.parent.conv_screen_coords(event.x, event.y)
        self.ctx.new_path()
        self.ctx.set_line_width(self.parent.LINE_WIDTH)
        self.ctx.set_line_cap(cairo.LINE_CAP_ROUND)
        self.ctx.append_path(self.hitpath)
        return self.ctx.in_stroke(x, y)

class Block(object):
    TERM_WIDTH = 20
    TERM_HEIGHT = 20
    TERM_SPACING = 60

    def __init__(self, parent, name, constructor, x = 0, y = 0):
        parent.nz_graph.add_block(name, constructor)

        info = parent.nz_graph.block_info(name)

        self.parent = parent
        self.name = name
        self.blocktype = constructor
        self.inputs = info.inputs
        self.outputs = info.outputs
        self.x = x
        self.y = y

        self.setup = False

    def delete(self):
        connections = self.parent.connections
        for connection in connections:
            if connection.input_block == self or connection.output_block == self:
                connection.delete()

        self.parent.blocks.remove(self)
        self.parent.nz_graph.del_block(self.name)

    def draw(self, ctx, selected = False):
        if not self.setup:
            ctx.select_font_face("Helvetica", cairo.FONT_SLANT_NORMAL, cairo.FONT_WEIGHT_NORMAL)
            ctx.set_font_size(60)

            (self.text_x, self.text_y, self.text_width, self.text_height, dx, dy) = ctx.text_extents(self.blocktype)

            self.width = self.text_width + 60
            text_height = self.text_height + 60
            input_height = self.TERM_SPACING * len(self.inputs)
            output_height = self.TERM_SPACING * len(self.outputs)

            self.height = max(text_height, input_height, output_height)

            self.setup = True

        ctx.set_line_width(self.parent.LINE_WIDTH)
        ctx.set_line_join(cairo.LINE_JOIN_ROUND)

        ctx.new_path()
        ctx.move_to(self.x - self.width / 2, self.y - self.height / 2)
        ctx.rel_line_to(self.width, 0)
        ctx.rel_line_to(0, self.height)
        ctx.rel_line_to(-self.width, 0)
        ctx.close_path()

        ctx.set_source_rgb(200, 200, 200)
        ctx.fill_preserve()

        if selected:
            ctx.set_source_rgb(100, 0, 0)
        else:
            ctx.set_source_rgb(0, 0, 0)
        ctx.stroke()

        ctx.select_font_face("Helvetica", cairo.FONT_SLANT_NORMAL, cairo.FONT_WEIGHT_NORMAL)
        ctx.set_font_size(60)
        ctx.move_to(self.x - self.text_width / 2 - self.text_x, self.y - self.text_height / 2 - self.text_y)
        ctx.set_source_rgb(0, 0, 0)
        ctx.show_text(self.blocktype)

        for i in range(len(self.inputs)):
            (x, y) = self.input_location(i)
            ctx.new_path()
            ctx.move_to(x - self.TERM_WIDTH / 2, y - self.TERM_HEIGHT / 2)
            ctx.rel_line_to(self.TERM_WIDTH, 0)
            ctx.rel_line_to(0, self.TERM_HEIGHT)
            ctx.rel_line_to(-self.TERM_WIDTH, 0)
            ctx.close_path()
            ctx.set_source_rgb(200, 200, 200)
            ctx.fill_preserve()
            ctx.set_source_rgb(0, 0, 0)
            ctx.stroke_preserve()

        for i in range(len(self.outputs)):
            (x, y) = self.output_location(i)
            ctx.new_path()
            ctx.move_to(x - self.TERM_WIDTH / 2, y - self.TERM_HEIGHT / 2)
            ctx.rel_line_to(self.TERM_WIDTH, 0)
            ctx.rel_line_to(0, self.TERM_HEIGHT)
            ctx.rel_line_to(-self.TERM_WIDTH, 0)
            ctx.close_path()
            ctx.set_source_rgb(200, 200, 200)
            ctx.fill_preserve()
            ctx.set_source_rgb(0, 0, 0)
            ctx.stroke()

    def input_location(self, input_index):
        input_slots = float(len(self.inputs) - 1)
        return (self.x - self.width / 2, self.y - (input_index - input_slots / 2) * self.TERM_SPACING)

    def output_location(self, output_index):
        output_slots = float(len(self.outputs) - 1)
        return (self.x + self.width / 2, self.y - (output_index - output_slots / 2) * self.TERM_SPACING)

    def mouse_hit_input(self, event):
        (x, y) = self.parent.conv_screen_coords(event.x, event.y)
        for input_index in range(len(self.inputs)):
            (inp_x, inp_y) = self.input_location(input_index)
            if abs(x - inp_x) < self.TERM_WIDTH and abs(y - inp_y) < self.TERM_HEIGHT:
                return input_index

    def mouse_hit_output(self, event):
        (x, y) = self.parent.conv_screen_coords(event.x, event.y)
        for output_index in range(len(self.outputs)):
            (out_x, out_y) = self.output_location(output_index)
            if abs(x - out_x) < self.TERM_WIDTH and abs(y - out_y) < self.TERM_HEIGHT:
                return output_index

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

    with nz.pa:
        nzcontext = nz.Context()
        graph = Graph(win, nzcontext)

        win.show_all()
        Gtk.main()

if __name__ == '__main__':
    main()
