import gdb

class FkLoadAndRun(gdb.Command):
  "Load and run"
  def __init__ (self):
    super(FkLoadAndRun, self).__init__("fklr", gdb.COMMAND_SUPPORT, gdb.COMPLETE_NONE, True)

  def invoke(self, arg, from_tty):
    gdb.execute("load")
    gdb.execute("monitor reset")
    gdb.execute("continue")

FkLoadAndRun()
