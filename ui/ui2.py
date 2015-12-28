import gi
gi.require_version('Gtk', '3.0')
from gi.repository import Gtk
from gi.repository import Gdk
import graph

def main():
    win = Gtk.Window()
    win.set_default_size(450, 550)
    win.connect('destroy', lambda w: Gtk.main_quit())
    g = graph.Graph()
    win.add(g)
    win.show_all()
    Gtk.main()

if __name__ == '__main__':
    main()
