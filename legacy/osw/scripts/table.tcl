
#Copyright (c) 2001-2004 Amar Chaudhary. All rights reserved.
#Copyright (c) 1998-2001 Regents of the University of California.
#All rights reserved.
#
#     ``The contents of this file are subject to the Open Sound World Public
#     License Version 1.0 (the "License"); you may not use this file except in
#     compliance with the License. A copy of the License should be included
#     in a file named "License" or "License.txt" in the distribution from 
#     which you obtained this file. 
#
#     Software distributed under the License is distributed on an "AS IS"
#     basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the
#     License for the specific language governing rights and limitations
#     under the License.
#
#     The Original Code is Open Sound World (OSW) software.
#
#     The Initial Developer of the Original Code is Amar Chaudhary.
#     Portions created by Amar Chaudhary are Copyright (C) 1998-2001 
#     Regents of the University of California and Amar Chaudhary. 
#     All Rights Reserved.
#
#     Contributor(s):
#
#
#
#From the UC Regents:
#
#Permission to use, copy, modify, and distribute this software and its
#documentation, without fee and without a signed licensing agreement, is hereby
#granted, provided that the above copyright notice, this paragraph and the
#following two paragraphs appear in all copies, modifications, and
#distributions.  Contact The Office of Technology Licensing, UC Berkeley, 2150
#Shattuck Avenue, Suite 510, Berkeley, CA 94720-1620, (510) 643-7201, for
#commercial licensing opportunities.
#
#Written by Amar Chaudhary, The Center for New Music and Audio Technologies, 
#University of California, Berkeley.
#
#     IN NO EVENT SHALL REGENTS BE LIABLE TO ANY PARTY FOR DIRECT, INDIRECT,
#     SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING LOST
#     PROFITS, ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS
#     DOCUMENTATION, EVEN IF REGENTS HAS BEEN ADVISED OF THE POSSIBILITY OF
#     SUCH DAMAGE.
#
#     REGENTS SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT
#     LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
#     FOR A PARTICULAR PURPOSE. THE SOFTWARE AND ACCOMPANYING
#     DOCUMENTATION, IF ANY, PROVIDED HEREUNDER IS PROVIDED "AS IS".
#     REGENTS HAS NO OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT, UPDATES,
#     ENHANCEMENTS, OR MODIFICATIONS.
#     
#


proc ShowTable {table} {

    set shell [iwidgets::shell .table#auto -title [file tail $table]]
               set win [$shell childsite]

    # ---- Configure Canvas ----
    iwidgets::scrolledcanvas $win.c \
            -vscrollmode none \
            -hscrollmode static \
            -width 400 \
            -height 240 \
            -autoresize yes
    
    grid $win.c -row 1 -column 2
    
    set tableCanvas [$win.c component canvas]
    
    # TODO: make scrollbar obedient
    set tableScrollbar [$win.c component horizsb]
    
    $tableCanvas configure -background black
    
    #---- Configure Drawing Toolbar ----
    set tbar [iwidgets::toolbar $win.tbar -orient vertical]
    $tbar add button line -balloonstr "Line" -image \
            [oswLoadBitmap photo line.gif true] \
            -command [list SelectTableLine $tableCanvas]
    $tbar add button freehand -balloonstr "Freehand" -image \
            [oswLoadBitmap photo pencil2.gif true] \
            -command [list SelectTableFreeDraw $tableCanvas]
    
    grid $tbar -row 1 -column 1
    
    #---- Configure Buttons ----
    iwidgets::buttonbox $win.bb
    
    $win.bb add OK \
            -text "OK" \
            -command [list StoreTableData $table $tableCanvas $shell]
    
    $win.bb add Cancel \
            -text "Cancel" \
            -command [list destroy $shell]
    
    $win.bb add Apply \
            -text "Apply" \
            -command [list StoreTableData $table $tableCanvas none]
    
    grid $win.bb -row 2 -column 2 -columnspan 2
    
    #---- Plotting ----
    set linecoords {} 
    
    set max [oswGet $table/max]
    set min [oswGet $table/min]
    set x 0
    foreach elem [oswGetData $table] {
        if {[catch {set y [expr int(120 - 120 * $elem)]} ]} {
            puts stderr "Warning: integer overflow drawing table"
            set y 0 
        }
        
        lappend linecoords $x
        lappend linecoords $y
        incr x
    }
    
    $tableCanvas create line $x 0 $x 240 -tag boundary -fill yellow
    $tableCanvas create line 0 120 $x 120 -tag zero -fill yellow
    
    eval $tableCanvas create line $linecoords -tag data -fill green
    $tableCanvas configure -scrollregion [list 0 0 [lindex [$tableCanvas bbox all] 2] 240]
    
    SelectTableLine $tableCanvas
    $shell activate
    focus $shell
}

proc ShowTableInline {table win} {

    set width [llength [oswGetData $table]]
    if {$width > 400} {
        set width 400
    }

    iwidgets::scrolledcanvas $win.c \
            -vscrollmode none \
            -hscrollmode static \
            -width $width \
            -height 240 \
            -autoresize yes
    
    grid $win.c -row 1 -column 1 -sticky ew
    
    set tableCanvas [$win.c component canvas]
    
    # TODO: make scrollbar obedient
    set tableScrollbar [$win.c component horizsb]
    
    $tableCanvas configure -background black

    DrawTableInline $table $tableCanvas
    SelectTableFreeDraw $tableCanvas
    bind $tableCanvas <ButtonRelease-1> [list StoreTableData $table $tableCanvas none]
}

proc DrawTableInline {table tableCanvas} {    
    set linecoords {} 
    
    set max [oswGet $table/max]
    set min [oswGet $table/min]
    set x 0
    foreach elem [oswGetData $table] {
        if {[catch {set y [expr int(120 - 120 * $elem)]} ]} {
            puts stderr "Warning: integer overflow drawing table"
            set y 0 
        }
        
        lappend linecoords $x
        lappend linecoords $y
        incr x
    }
    
    $tableCanvas delete data

    $tableCanvas create line $x 0 $x 240 -tag boundary -fill yellow
    $tableCanvas create line 0 120 $x 120 -tag zero -fill yellow
    eval $tableCanvas create line $linecoords -tag data -fill green
    $tableCanvas configure -scrollregion [list 0 0 [lindex [$tableCanvas bbox all] 2] 240]
}


namespace eval showtable set OldX 0
namespace eval showtable set OldY 0


proc SelectTableFreeDraw {tableCanvas} {
    bind $tableCanvas <B1-Motion> [list SetTableDraw $tableCanvas %x %y]
    bind $tableCanvas <Button-1> [list StartTableDraw %x %y]    
}

proc SelectTableLine {tableCanvas} {
    set showtable::OldX -1
    set showtable::OldY -1
    bind $tableCanvas <Button-1> [list SetTableLine $tableCanvas %x %y]
}

proc StartTableDraw {x y} {
    set showtable::OldX $x
    set showtable::OldY $y
}


proc SetTableDraw {tableCanvas x y} {
    
    
    set linecoords [$tableCanvas coords data]
    
    if {$x != $showtable::OldX} {
        set slope [expr ($y - $showtable::OldY) / ($x - $showtable::OldX)]
    } else {
        set slope 1
    }
    
    set prevx [expr 2 * $showtable::OldX + 1]
    set showtable::OldX $x
    set x [expr 2 * $x + 1]
    set prevy $showtable::OldY
    set showtable::OldY $y
    
    if {$x >= $prevx} {
        set dx 2
    } else {
        set dx -2
    }
    
    if {[catch {
        for {set i $prevx} {$i != $x} {incr i $dx} {
            set interpolate [expr $prevy + $slope * ($i - $prevx - 2)/2]
            set linecoords [lreplace $linecoords $i $i $interpolate]
        }
    } error]} {
        puts $error
        return
    }
    
    if {[catch {set linecoords [lreplace $linecoords $x $x $y]} error]} {
        puts $error
        return
    }
    
    catch {eval $tableCanvas coords data $linecoords}
}

proc DrawStartPoint {tableCanvas x y} {
    $tableCanvas create rectangle $x $y [expr $x + 4] [expr $y + 4] -fill black -tag point
}

proc SetTableLine {tableCanvas x y} {
    
    if {$showtable::OldX == -1} {
        set showtable::OldX $x
        set showtable::OldY $y
        $tableCanvas delete point
        DrawStartPoint $tableCanvas $x $y
        return
    }
    
    if {$x == $showtable::OldX} {
        return
    }
    
    $tableCanvas delete point
    DrawStartPoint $tableCanvas $x $y    
    set linecoords [$tableCanvas coords data]
    set slope [expr double($y - $showtable::OldY) / double($x - $showtable::OldX)]
    set prevx [expr 2 * $showtable::OldX + 1]
    set showtable::OldX $x
    set x [expr 2 * $x + 1]
    set prevy $showtable::OldY
    set showtable::OldY $y
    
    if {$x >= $prevx} {
        set dx 2
    } else {
        set dx -2
    }
    
    if {[catch {
        for {set i $prevx} {$i != $x} {incr i $dx} {
            set interpolate [expr $prevy + $slope * ($i - $prevx - 2)/2]
            set linecoords [lreplace $linecoords $i $i $interpolate]
        }
    }]} {
        return
    }
    
    #set linecoords [lreplace $linecoords $x $x $y]
    catch {eval $tableCanvas coords data $linecoords}
    
}

proc StoreTableData {table tableCanvas shell} {
    set max [oswGet $table/max]
    set min [oswGet $table/min]
    set TableSize [oswGet $table/size]
    
    set linecoords [$tableCanvas coords data]
    set tabledata {}
    
    # This loop will always map indexes one-to-one if their are the same
    # amount of them, or if there are more or less it will round off
    for {set TableIndex 1} {$TableIndex < $TableSize} {incr TableIndex} {
        set CanvasIndex [expr 1 + 2 * \
                int(($TableIndex*1.0/$TableSize) \
                * [llength $linecoords] / 2) ]
        set ratio [expr (120 - [lindex $linecoords $CanvasIndex]) / 120.0]
        
        set val $ratio
        if {$val < $min} {set val $min}
        if {$val > $max} {set val $max}
        
        lappend tabledata [expr $min + $ratio * ($max-$min)]
        #lappend tabledata [expr (100 - [lindex $linecoords $i]) * 0.01 * $max]
    }
    
    oswSetData $table $tabledata
    if {$shell != "none"} {
        destroy $shell
    }
}

