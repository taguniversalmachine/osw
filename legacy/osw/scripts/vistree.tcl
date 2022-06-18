
#Copyright (c) 2001 Amar Chaudhary. All rights reserved.
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

set TreeHorizontal 10
set TreeVertical   15

image create bitmap treePlus -data {
    #define plus_width 9
    #define plus_height 9
    static unsigned char plus_bits[] = {
	0x10, 0x00, 0x28, 0x00, 0x54, 0x00, 0x92, 0x00, 0x7d, 0x01, 0x92, 0x00,
	0x54, 0x00, 0x28, 0x00, 0x10, 0x00};
    }

image create bitmap treeMinus -data {
    #define minus_width 9
    #define minus_height 9
    static unsigned char minus_bits[] = {
	0x10, 0x00, 0x28, 0x00, 0x44, 0x00, 0x82, 0x00, 0x7d, 0x01, 0x82, 0x00,
	0x44, 0x00, 0x28, 0x00, 0x10, 0x00};	
    }



proc VisualTree {tree c} {
    $c delete all
    DrawTree $tree $tree $c 0 5
    bind $c <Button-1> "FindNode $c %x %y click"
    bind $c <Double-Button-1> "FindNode $c %x %y dblclick"
}

proc DrawTree {tree root c horiz vert} {
    global TreeHorizontal TreeVertical
    
    $c create line $horiz $vert [expr $horiz + $TreeHorizontal] $vert

    if {[$tree HasChildren]} {
	set dvert [DrawNode $tree $c [expr $horiz + $TreeHorizontal + 5 + 10] $vert]

	if {[$tree cget -open]} {
	    $c create image [expr $horiz + $TreeHorizontal] $vert -image treePlus -tag toggle_$tree
	    incr horiz $TreeHorizontal
	    
	    set oldvert $vert
	    incr vert $dvert
	    foreach child [$tree cget -children] {
		
		$c create line $horiz $oldvert $horiz $vert
		set oldvert $vert		
		set vert [DrawTree $child $root $c $horiz $vert]
	    }
	} else {
	    $c create image [expr $horiz + $TreeHorizontal] $vert -image treeMinus -tag toggle_$tree
	    incr vert $dvert
	}
	$c bind toggle_$tree <Button-1> [list OpenOrCloseNode $tree $root $c]
    } else {
	set dvert [DrawNode $tree $c [expr $horiz + $TreeHorizontal + 5] $vert]
	incr vert $dvert
    }
	
    #$c bind $tree <Button-1> [list OpenOrCloseNode $tree $root $c]

    return $vert
}

proc OpenOrCloseNode {node root c} {
    
    $node Toggle
    VisualTree $root $c
}


proc DrawNode {tree c x y} {

    if {[llength [$tree cget -nodeWidget]] == 0} {
	$c create text $x $y -text [$tree cget -nodeVal] -tag $tree -anchor w
	global TreeVertical
	return $TreeVertical
    } else {
	$c create window $x $y -window [$tree cget -nodeWidget] -tag $tree -anchor w
	return [[$tree cget -nodeWidget] cget -height]
    } 
}

proc FindNode {c x y func} {

    set x [$c canvasx $x]
    set y [$c canvasy $y]

    foreach overlap [$c find overlapping $x $y $x $y] {
	foreach tag [$c itemcget $overlap -tag] {
	    catch {$tag Interact $func}
	}
    }
}





