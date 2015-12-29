import cairo

class Element():
    def __init__(self, parent, x = 0, y = 0):
        self.parent = parent
        self.x = 0
        self.y = 0

    def button_down(self, x, y):
        return False

    def mouse_move(self, x, y):
        pass

    def key_down(self, key):
        return False

class Block(Element):
    TERM_WIDTH = 20
    TERM_HEIGHT = 20
    TERM_SPACING = 60

    def __init__(self, block, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.block = block
        self.text = self.block.id
        self.n_inputs = len(self.block.inputs)
        self.n_outputs = len(self.block.outputs)
        self.init_graphics()

        input_slots = float(self.n_inputs - 1)
        self.input_locations = [(-self.width / 2, -(input_index - input_slots / 2) * self.TERM_SPACING) for input_index in range(self.n_inputs)]
        output_slots = float(self.n_outputs - 1)
        self.output_locations = [(self.width / 2, -(output_index - output_slots / 2) * self.TERM_SPACING) for output_index in range(self.n_outputs)]

    def hit_test(self, x, y):
        return abs(x) < self.width / 2 and abs(y) < self.height / 2

    def rect_test(self, x, y, w, h):
        # test if center is in rect
        return x < 0 and y < 0 and x + w > 0 and y + h > 0

    def init_graphics(self):
        ctx = self.parent.stored_ctx
        ctx.select_font_face("Helvetica", cairo.FONT_SLANT_NORMAL, cairo.FONT_WEIGHT_NORMAL)
        ctx.set_font_size(60)
        (self.text_x, self.text_y, self.text_width, self.text_height, dx, dy) = ctx.text_extents(self.text)
        self.width = self.text_width + 60
        text_height = self.text_height + 60
        input_height = self.TERM_SPACING * self.n_inputs
        output_height = self.TERM_SPACING * self.n_outputs
        self.height = max(text_height, input_height, output_height)

    def draw(self, ctx, selected = False):
        ctx.set_line_width(self.parent.LINE_WIDTH)
        ctx.set_line_join(cairo.LINE_JOIN_ROUND)

        ctx.rectangle(-self.width / 2, -self.height / 2,
                      self.width, self.height)

        ctx.set_source_rgb(200, 200, 200)
        ctx.fill_preserve()

        if selected:
            ctx.set_source_rgb(100, 0, 0)
        else:
            ctx.set_source_rgb(0, 0, 0)
        ctx.stroke()

        ctx.select_font_face("Helvetica", cairo.FONT_SLANT_NORMAL, cairo.FONT_WEIGHT_NORMAL)
        ctx.set_font_size(60)
        ctx.move_to(-self.text_width / 2 - self.text_x, -self.text_height / 2 - self.text_y)
        ctx.set_source_rgb(0, 0, 0)
        ctx.show_text(self.text)

        for pos in self.input_locations + self.output_locations:
            ctx.save()
            ctx.translate(*pos)
            self.draw_term(ctx)
            ctx.restore()

    def draw_term(self, ctx):
        ctx.rectangle(-self.TERM_WIDTH / 2, -self.TERM_HEIGHT / 2,
                      self.TERM_WIDTH, self.TERM_HEIGHT)
        ctx.set_source_rgb(200, 200, 200)
        ctx.fill_preserve()
        ctx.set_source_rgb(0, 0, 0)
        ctx.stroke()
