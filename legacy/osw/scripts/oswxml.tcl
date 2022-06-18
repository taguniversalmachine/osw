
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


proc xmlfix {stringToFix} {
    regsub -all -- "&" $stringToFix "\\&amp;" result
    regsub -all -- "\\\"" $result "\\&quot;" result
    regsub -all -- "<" $result "\\&lt;" result
    regsub -all -- ">" $result "\\&gt;" result
    regsub -all -- "{" $result "\\&#123;" result
    regsub -all -- "}" $result "\\&#125;" result
    regsub -all -- "\\$" $result "\\&#36;" result
    regsub -all -- "\\\[" $result "\\&#91;" result
    regsub -all -- "\\\]" $result "\\&#93;" result
    regsub -all -- "\\\\" $result "\\&#92;" result
    set result [string trimright $result]
    return $result
}

proc xmlvalfix {stringToFix} {
    set result $stringToFix
    regsub -all -- "<" $result ";lt;" result
    regsub -all -- ">" $result ";gt;" result
    return $result
}

proc xmlvalunfix {stringToFix} {
    set result $stringToFix
    regsub -all -- ";lt;" $result "<" result
    regsub -all -- ";gt;" $result ">" result
    return $result
}

set xmlstate {}

proc CreateXMLParser {} {
    set xmlparser [::xml::parser]
    $xmlparser configure -elementstartcommand ElementStart \
	-elementendcommand ElementEnd \
	-characterdatacommand CharacterData \
	-reportempty 1
    return $xmlparser
}

proc GetXMLStateName {} {
    global xmlstate
    return [lindex [lindex $xmlstate end] 0]
}

proc GetXMLStateData {} {
    global xmlstate
    return [lindex [lindex $xmlstate end] 1]
}

proc PushXMLState {name data} {
    global xmlstate
    lappend xmlstate [list $name $data]
}

proc PopXMLState {} {
    global xmlstate
    set xmlstate [lrange $xmlstate 0 [expr [llength $xmlstate] - 2]]
}

proc ClearXMLState {} {
    global xmlstate
    set xmlstate {}
}

proc GetXMLBackStateData {name} {
    global xmlstate
    for {set i [expr [llength $xmlstate] - 1]} {$i >= 0} {incr i -1} {
	if {[lindex [lindex $xmlstate $i] 0] == $name} {
	    return [lindex [lindex $xmlstate $i] 1]
	}
    }
}

set _osw_xml_data {}
set _osw_xml_data_from_src 0

proc ResetData {} {
    set ::_osw_xml_data {}
}

proc AppendData {toAppend} {
    append ::_osw_xml_data $toAppend
}

proc GetData {} {
    return $::_osw_xml_data
}

proc SetDataFromSource {} {
    set ::_osw_xml_data_from_src 1
}

proc ResetDataFromSource {} {
    set ::_osw_xml_data_from_src 0
}

proc IsDataFromSource {} {
    return $::_osw_xml_data_from_src
}

proc ElementStart {name attList args} {

    set clipboard 0
    array set options {-empty 0}
    array set options $args
    switch $name PATCH {
	set Background SystemButtonFace
	set instanceName ""
	set parent ""
	set patchArgs {}
	set sourceFile ""
	set x 0
	set y 0
	set width 96
	set height 24
	set color darkgrey
	set subpatch false
	if {[GetXMLStateName] == "PATCHINSTANCE"} {
	    set data [GetXMLStateData]
	    set parent [lindex $data 0]
	    set instanceName [lindex $data 1]
	    set patchArgs [lindex $data 2]
	    set sourceFile [lindex $data 3]
	} elseif {[GetXMLStateName] == "PATCH"} {
	    set parentPatchObj [GetXMLStateData]
	    set parent [$parentPatchObj cget -parent]/[$parentPatchObj cget -patchInstance]	    
	    set subpatch true
	}

	foreach {attname value} $attList {
	    set $attname $value
	}
	
	if {$instanceName == ""} {
	    set instanceName $CLASS
	}
	if {$subpatch} {
	    # must be a subpatch
	    set transform [[GetXMLStateData] MakeTransform  $x $y \
		    $width $height [list Patch -name $instanceName]]
	    $transform WidgetColor $color
	    catch {delete object patchObj-$parent/$instanceName}
	}
	global defaultCount
	
	if {[llength [info commands patchObj-$parent/$instanceName]] == 1} {
	    # there is already a toplevel patch $patchName
	    append instanceName $defaultCount
	    incr defaultCount
	}

	set newpatch [Patch patchObj-$parent/$instanceName -patchClass $CLASS \
		-patchInstance $instanceName  -bg $Background -parent $parent \
		-patchArgs $patchArgs -sourceFile $sourceFile]
	
	cd [$newpatch cget -dir]
	#focus .$instanceName
	PushXMLState PATCH $newpatch
    } TRANSFORM {
	if {[GetXMLStateName] != "PATCH"} {
	    error "TRANSFORM tag can only appear within a PATCH element.  Current element is [GetXMLStateName]"
	}
	set x 0
	set y 0
	set width 96
	set height 24
	set color darkgrey
	set name ""
	set command ""
	set fontface ""
	set fontsize ""
	set patch [GetXMLStateData]
	foreach {attname value} $attList {
	    set $attname [xmlvalunfix $value]
	}
	set transform ""
        if {$command == ""} {
            #command in separate element, defer transform creation
            PushXMLState TRANSFORM [list $name $patch $x $y $width $height $color $fontface $fontsize]
        } else {
            if {$name == ""} {
                set transform [$patch MakeTransform  $x $y $width $height $command]
            } else {
                set transform [$patch MakeTransform  $x $y $width $height $command $name]
            }
            PushXMLState TRANSFORM $transform
            $transform WidgetColor $color
            if {$fontface != ""} {
                $transform WidgetFontFace $fontface
            }
            if {$fontsize != ""} {
                $transform WidgetFontSize $fontsize
            }
        }

    } STICKY {
	if {[GetXMLStateName] != "PATCH"} {
	    error "STICKY tag can only appear within a PATCH element.  Current element is [GetXMLStateName]"
	}	
	set x 0
	set y 0
	set width 75
	set height 50
	set color #FFFF77
	set text ""
	set fontface ""
	set fontsize ""
	foreach {attname value} $attList {
	    set $attname $value
	}
	set newsticky [[GetXMLStateData] MakeSticky $x $y $width $height $color $text]
	if {$fontface != ""} {
	    $newsticky WidgetFontFace $fontface
	}
	if {$fontsize != ""} {
	    $newsticky WidgetFontSize $fontsize
	}
	PushXMLState STICKY $newsticky
    } IMG {
	if {[GetXMLStateName] != "PATCH"} {
	    error "IMG tag can only appear within a PATCH element.  Current element is [GetXMLStateName]"
	}	
	set x 0
	set y 0
	set src 0
	foreach {attname value} $attList {
	    set $attname $value
	}
	set newimage [[GetXMLStateData] MakeImage $x $y $src]
	PushXMLState IMG $newimage
    } TITLE {
	if {[GetXMLStateName] != "PATCH"} {
	    error "TITLE tag can only appear within a PATCH element.  Current element is [GetXMLStateName]"
	}	
	set x 0
	set y 0
	set width 75
	set height 50
	set color #FFFF77
	set text ""
	set fontface ""
	set fontsize ""
	foreach {attname value} $attList {
	    set $attname $value
	}
	set newtitle [[GetXMLStateData] MakeTitle $x $y $color $text]
	if {$fontface != ""} {
	    $newtitle WidgetFontFace $fontface
	}
	if {$fontsize != ""} {
	    $newtitle WidgetFontSize $fontsize
	}
	PushXMLState TITLE $newtitle
    } HYPERLINK {
	if {[GetXMLStateName] != "PATCH"} {
	    error "HYPERLINK tag can only appear within a PATCH element.  Current element is [GetXMLStateName]"
	}	
	set x 0
	set y 0
	set width 75
	set height 50
	set color #FFFF77
	set link ""
	set fontface ""
	set fontsize ""
	foreach {attname value} $attList {
	    set $attname $value
	}
	set newtitle [[GetXMLStateData] MakeLink $x $y $color $link]
	if {$fontface != ""} {
	    $newtitle WidgetFontFace $fontface
	}
	if {$fontsize != ""} {
	    $newtitle WidgetFontSize $fontsize
	}
	PushXMLState HYPERLINK $newtitle
    
    } STATE {
	if {[GetXMLStateName] != "TRANSFORM"} {
	    error "STATE tag can only appear within a TRANSFORM element.  Current element is [set ::xmlstate]"
	}
	set name ""
	set value ""
	set font ""
	foreach {attname attvalue} $attList {
	    set $attname $attvalue
	}   
	set transformObj [GetXMLStateData]
	set transformPath [$transformObj cget -transformPath]
        if {$value != ""} {
            oswSet [file dirname $transformPath]/$name $value
            PushXMLState STATE NOSET
        } else {
            ResetData
            PushXMLState STATE [file dirname $transformPath]/$name
        }

    } CONNECTION {
	if {[GetXMLStateName] != "PATCH"} {
	    error "CONNECTION tag can only appear within a PATCH element.  Current element is [GetXMLStateName]"
	}	
	set inlet ""
	set outlet ""
	set vertices {}
	set patch [GetXMLStateData]
	foreach {attname value} $attList {
	    set $attname $value
	}   
	if {$inlet == "" || $outlet == ""} {
	    error "Must specify inlet and outlet in CONNECTION tag."
	}
	$patch MakeConnectionRelative $outlet $inlet $vertices
	PushXMLState CONNECTION [list $outlet $inlet]
    } DATA {
	if {[GetXMLStateName] != "TRANSFORM"} {
	    error "DATA can only appear within a TRANSFORM element.  Current element is [GetXMLStateName]"
	}
	set src ""
	foreach {attname value} $attList {
	    set $attname $value
	}
	set transformObj [GetXMLStateData]
	if {$src != ""} {
	    if {[file pathtype $src] == "relative"} {
		set src [[$transformObj cget -patchWindow] cget -dir]/$src
	    }
	    oswSet [$transformObj cget -transformPath]/src $src
            SetDataFromSource
	} else {
            ResetData
        }
	PushXMLState DATA $transformObj
    } OSW {
	#do nothing
    } SCRIPT {
	set language "tcl"
	set src ""
	foreach {attname value} $attList {
	    set $attname $value
	}
	if {[string compare -nocase $language "tcl"]} {
	    error "Script language must be Tcl (at least for now)"
	}
	global scriptbody 
	set scriptbody ""
	set patch [GetXMLStateData]
	switch [$patch info class] "::TransformWidget" {
	    set patch [$patch cget -patchWindow]
	}
	PushXMLState SCRIPT $patch
	if {$src != ""} {
	    if {[file pathtype $src] == "relative"} {
		set src [$patch cget -dir]/$src
	    }
	    oswLoadScript $src
	    $patch AddScriptSrc $src
	}
    } COPYRIGHT {
	PushXMLState COPYRIGHT [GetXMLStateData]
    } COMMAND {
        ResetData
        PushXMLState COMMAND [GetXMLStateData]
    } default {
	PushXMLState unused ""
    }        
}

proc ElementEnd {name args} {
    
#    puts "$name [GetXMLStateName] [GetXMLStateData]"

    switch $name PATCH {
	set patch [GetXMLStateData]
	$patch FinishSetup
	PopXMLState
    } SCRIPT {
	global scriptbody
	if {$scriptbody != ""} {
	    set patchObj [GetXMLStateData]
	    set patch "[$patchObj cget -parent]/[$patchObj cget -patchInstance]"
	    eval $scriptbody
	    $patchObj AddScript $scriptbody
	}
	PopXMLState	
    } OSW {
	#do nothing
    } DATA {
        if {[IsDataFromSource]} {
            ResetDataFromSource
        } else {
            oswSetData [[GetXMLStateData] cget -transformPath] [GetData]
        }
        ResetData
        PopXMLState
    } STICKY {
        set stickyObj [GetXMLStateData]
        $stickyObj SetText [GetData]
        ResetData
        PopXMLState
    } COMMAND {
        set transformInfo [GetXMLStateData]
        set name [lindex $transformInfo 0]
        set patch [lindex $transformInfo 1]
        set x [lindex $transformInfo 2]
        set y [lindex $transformInfo 3]
        set width [lindex $transformInfo 4]
        set height [lindex $transformInfo 5]
        set color [lindex $transformInfo 6]
        set fontface [lindex $transformInfo 7]
        set fontsize [lindex $transformInfo 8]
        set command [GetData]
        if {$name == ""} {
            set transform [$patch MakeTransform  $x $y $width $height $command]
        } else {
            set transform [$patch MakeTransform  $x $y $width $height $command $name]
        }
        $transform WidgetColor $color
        if {$fontface != ""} {
            $transform WidgetFontFace $fontface
        }
        if {$fontsize != ""} {
            $transform WidgetFontSize $fontsize
        }            
        ResetData
        PopXMLState
        #fix parent TRANSFORM state
        PopXMLState
        PushXMLState TRANSFORM $transform
    } STATE {
        set stateVar [GetXMLStateData]
        set value [GetData]
        if {$stateVar != "NOSET"} {
            oswSet $stateVar $value
        }
        PopXMLState
    } default {
	PopXMLState
    }

}

proc CharacterData {data} {
    if {$data != "" } {
	switch [GetXMLStateName] DATA {
	    set transformObj [GetXMLStateData]
            AppendData $data
	} STICKY {
	    set stickyObj [GetXMLStateData]
            AppendData $data
	} SCRIPT {
	    global scriptbody
	    append scriptbody $data
	} COPYRIGHT {
	    set patch [GetXMLStateData]
	    $patch SetCopyright $data
	} TITLE {
            set titleObj [GetXMLStateData]
            $titleObj SetText $data
        } COMMAND {
            AppendData $data
        } STATE {
            AppendData $data
        }
    }    
}



