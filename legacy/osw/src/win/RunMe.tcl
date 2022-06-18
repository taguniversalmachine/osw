
set argv [concat [list [pwd]] $argv]

label .title -text "Open Sound World" -font {{helvetica} 24}
catch {label .logo -image "bitmaps/logoHoriz.gif"}
label .auth -text "by Amar Chaudhary"
label .copy -text "Copyright(c) 1999-2001 Regents of the University of California\nAll rights reserved."
label .install -text "Installing..." -font {{helvetica} 18}
button .done -state disabled -text "Done" -command {exit}
pack .title -side top
catch {pack .logo -side top}
pack .auth .copy .install .done -side top
source reginstall.tcl
after 3000 {.install configure -text "Installation Complete!" ; .done configure -state normal}
