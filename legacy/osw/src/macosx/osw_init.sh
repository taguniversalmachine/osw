
# OSW Init script

args=""

for arg in "$@"
do
  case "$arg" in
  "--debug")
    debug=1
  ;;
  "--server")
    server=1
  ;;
  "--help" | "-h")
    help=1
    debug=
    server=1
    args="$args --help"
  ;;
  *)
    # append to arg list...
    args="$args $arg"
  ;;
  esac
done

# default command (starts osw in GUI mode)
cmd="${TCL_BIN} ${OSW_DIR}/oswinit.tcl $args"

if [ $debug ]; then
    echo "------------------------------------------------"
    echo "Starting OSW using the GDB debugger."
    echo "Type 'run' to launch program."
    echo "If OSW crashes, type 'bt' to backtrace."
    echo "Use Ctl-C to suspend and return to the debugger."
    echo "Type 'q' to quit."
    echo "------------------------------------------------"
fi

if [ $server ]; then
    cmd="oswbin $args"
fi

if [ $debug ]; then
    if [ $server ]; then
        gdb --args $cmd
    else
        gdb --args "/Library/Frameworks/Tk.framework/Versions/8.4/Resources/Wish Shell.app/Contents/MacOS/Wish Shell" "${OSW_DIR}/oswinit.tcl" "$args"
    fi
else
    $cmd
fi

if [ $help ]; then
    echo "        --server        Run in server mode (No GUI)"
    echo "        --debug         Run using GNU DeBugger"
fi
