'''
simulate a mixer to control ratatouille
'''

import argparse
import errno
import os
import struct
import Tkinter as tk

class MidiScale(tk.Scale):
    def __init__(self, parent, var, name, fifo):
        self.var = var
        self.parent = parent
        self.fifo = fifo
        tk.Scale.__init__(self, parent, variable=var,
                          from_=127, to=0, name=name,
                          command=self.pack_and_write_fifo)

    def get(self):
        return self.var.get()
        
    def pack_and_write_fifo(self, newval):
        val = self.get()
        name = self.winfo_name()
        idx = int(name)
        packed_newval = struct.pack('%sB' % 3, 176, idx, val)
        self.update_fifo(packed_newval)

    def update_fifo(self, value_string):
        # send update to FIFO
        try:
            os.write(self.fifo, value_string)
        except IOError as err:
            if err.errno == errno.EPIPE:
                # TODO
                return
            else:
                raise

class MidiCheckbutton(tk.Checkbutton):
    def __init__(self, parent, var, name, fifo):
        self.var = var
        self.fifo = fifo
        tk.Checkbutton.__init__(self, parent, variable=var,
                                onvalue=1, offvalue=0,
                                name=name, command=self.pack_and_write_fifo)
    def get(self):
        return self.var.get()

    def pack_and_write_fifo(self):
        val = self.get()
        name = self.winfo_name()
        idx = int(name)
        packed_newval = struct.pack('%sB' % 3, 176, idx, val)
        self.update_fifo(packed_newval)

    def update_fifo(self, value_string):
        # send update to FIFO
        try:
            os.write(self.fifo, value_string)
        except IOError as err:
            if err.errno == errno.EPIPE:
                # TODO
                return
            else:
                raise

class Mixer:
    def __init__(self, fifo_fname):
        self.tk_root = tk.Tk()

        self.fifo_fname = fifo_fname
        self.fifo = os.open(self.fifo_fname, os.O_RDWR | os.O_NONBLOCK)

        self.tk_root.wm_title("Ratatouille mixer")
        self.tk_root.protocol("WM_DELETE_WINDOW", self.onclose_handler)
        self.set_up_labels()
        self.tk_root.rowconfigure(6, pad=10)

        self.poll_from_remy()

    def onclose_handler(self):
        os.close(self.fifo)
        #self.f.close()
        self.tk_root.destroy()

    def start_mixer(self):
        self.tk_root.mainloop()

    def poll_from_remy(self):
        # get updated mixer values from Remy
        try:
            midi_input = os.read(self.fifo, 4096)
        except OSError as err:
            if err.errno == errno.EAGAIN or err.errno == errno.EWOULDBLOCK:
                self.tk_root.after(50, self.poll_from_remy)
                return
            if err.errno == errno.EPIPE:
                # TODO
                return
            else:
                raise

        # unpack into uint8_t
        raw_state = struct.unpack(str(len(midi_input)) + 'B', midi_input)
        raw_state = zip(*[raw_state[i::3] for i in range(3)][1:]) 
        new_state = dict(raw_state) # new physical values
        
        self.update_gui(new_state)

        self.tk_root.after(100, self.poll_from_remy)

    def update_gui(self, new_state):
        print new_state
        for idx, value in new_state.iteritems():
            try:
                widget = self.tk_root.nametowidget(str(idx))
                widget.var.set(value)
            except KeyError:
                continue

    def set_up_labels(self):
        tk.Label(self.tk_root, text='RemyCC').grid(row=0,column=0)
        tk.Label(self.tk_root, text='AIMD').grid(row=2,column=0)

        c1 = 1
        c2 = 1
        r = 0
        for idx, (label, otype) in labels.iteritems():
            widget = None
            if otype == 'bool':
                if label not in ['RemyCC', 'AIMD']:
                    tk.Label(self.tk_root, text=label).grid(row=r+1, column=c1, 
                                                 pady=(0, 15))
                    widget = MidiCheckbutton(self.tk_root, 
                                               tk.IntVar(), str(idx),
                                               self.fifo)
                    widget.grid(row=r,column=c1, padx=5, pady=(20, 0))
                else:
                    widget = MidiCheckbutton(self.tk_root, 
                                               tk.IntVar(),
                                               str(idx), self.fifo)
                    widget.grid(row=r,column=c1, padx=5)
                c1 += 1
                if c1 == 5:
                    c1 = 1
                    r += 1
            elif otype == 'float':
                widget = MidiScale(self.tk_root, tk.DoubleVar(),
                                   str(idx), self.fifo)
                widget.grid(row=r+2,column=c2, padx=10)
                tk.Label(self.tk_root, text=label).grid(
                    row=r+3, column=c2, padx=10)
                c2 += 1
            else:
                raise ValueError, otype

'''
physical values mapping:

65 - 80: switch a sender (true/false); half Remy, half AIMD
81: link rate
83: buffer size
87: window horizontal size
88: time increment
89: autoscale (true/false)
91: autoscale_all (true/false)
'''
labels = {65:('RemyCC', 'bool'),
          66:('RemyCC', 'bool'),
          67:('RemyCC', 'bool'),
          68:('RemyCC', 'bool'),
          69:('RemyCC', 'bool'),
          70:('RemyCC', 'bool'),
          71:('RemyCC', 'bool'),
          72:('RemyCC', 'bool'),
          73:('AIMD', 'bool'),
          74:('AIMD', 'bool'),
          75:('AIMD', 'bool'),
          76:('AIMD', 'bool'),
          77:('AIMD', 'bool'),
          78:('AIMD', 'bool'),
          79:('AIMD', 'bool'),
          80:('AIMD', 'bool'),
          81:('Link rate', 'float'),
          83:('Buffer size', 'float'),
          87:('Window scale', 'float'),
          88:('Simulation speed', 'float'),
          89:('Scale to BDP', 'bool'),
          91:('Scale to buffer size', 'bool')
          }

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("midi_file")
    args = parser.parse_args()

    # create the FIFO
    try:
        os.mkfifo(args.midi_file)
    except OSError as e:
        raise OSError, "Failed to create FIFO: %s" % e

    # create and start the GUI
    mixer = Mixer(args.midi_file)
    mixer.start_mixer()

if __name__ == '__main__':
    main()
