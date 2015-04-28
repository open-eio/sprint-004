import os, shutil
import serial


class SerialPipe:
    def __init__(self,
                in_pipename = None,
                out_pipename = None,
                devpath = None,
                ):
        assert(not( (in_pipename is None) and (out_pipename is None) and (out_pipename is None)))
        if not os.path.exists(in_pipename):
            os.mkfifo(in_pipename)
        self.in_pipename = in_pipename
        if not os.path.exists(out_pipename):
            os.mkfifo(out_pipename)
        self.out_pipename = out_pipename
        self.devpath = devpath
        self._default_mode = 'rw'
        self._in_pipe  = None
        self._out_pipe = None
        
    def swap(self):
        self.in_pipename, self.out_pipename = (self.out_pipename, self.in_pipename)
        self._in_pipe,     self._out_pipe   = (self._out_pipe,    self._in_pipe)
        self._default_mode = self._default_mode[::-1]
        
    def open(self, mode = None, blocking = True):
        if mode is None:
            mode = self._default_mode
        if mode == 'rw':
            self._in_pipe  = open(self.in_pipename,'r')
            self._out_pipe = open(self.out_pipename,'w')
        elif mode == 'r':
            self._in_pipe  = open(self.in_pipename,'r')
        elif mode == 'w':
            self._out_pipe = open(self.out_pipename,'w')
        else:
            raise ValueError("mode string must begin with one of 'r', 'w', not '%s'" % mode)
            
################################################################################
# TEST CODE
################################################################################
if __name__ == "__main__":
    sp = SerialPipe(in_pipename = "/tmp/INPIPE",
                    out_pipename = "/tmp/OUTPIPE"
                    )
            
