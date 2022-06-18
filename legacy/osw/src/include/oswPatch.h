
/* 
Copyright (c) 2001 Amar Chaudhary. All rights reserved.
Copyright (c) 1998-2001 Regents of the University of California.
All rights reserved.

     ``The contents of this file are subject to the Open Sound World Public
     License Version 1.0 (the "License"); you may not use this file except in
     compliance with the License. A copy of the License should be included
     in a file named "License" or "License.txt" in the distribution from 
     which you obtained this file. 

     Software distributed under the License is distributed on an "AS IS"
     basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the
     License for the specific language governing rights and limitations
     under the License.

     The Original Code is Open Sound World (OSW) software.

     The Initial Developer of the Original Code is Amar Chaudhary.
     Portions created by Amar Chaudhary are Copyright (C) 1998-2001 
     Regents of the University of California and Amar Chaudhary. 
     All Rights Reserved.

     Contributor(s):

  
From the UC Regents:

Permission to use, copy, modify, and distribute this software and its
documentation, without fee and without a signed licensing agreement, is hereby
granted, provided that the above copyright notice, this paragraph and the
following two paragraphs appear in all copies, modifications, and
distributions.  Contact The Office of Technology Licensing, UC Berkeley, 2150
Shattuck Avenue, Suite 510, Berkeley, CA 94720-1620, (510) 643-7201, for
commercial licensing opportunities.

Written by Amar Chaudhary, The Center for New Music and Audio Technologies, 
University of California, Berkeley.

     IN NO EVENT SHALL REGENTS BE LIABLE TO ANY PARTY FOR DIRECT, INDIRECT,
     SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING LOST
     PROFITS, ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS
     DOCUMENTATION, EVEN IF REGENTS HAS BEEN ADVISED OF THE POSSIBILITY OF
     SUCH DAMAGE.

     REGENTS SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT
     LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
     FOR A PARTICULAR PURPOSE. THE SOFTWARE AND ACCOMPANYING
     DOCUMENTATION, IF ANY, PROVIDED HEREUNDER IS PROVIDED "AS IS".
     REGENTS HAS NO OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT, UPDATES,
     ENHANCEMENTS, OR MODIFICATIONS.
     
*/
/* oswPatch.h
   OSW Patch class
*/

#ifndef _OSW_PATCH_H
#define _OSW_PATCH_H

namespace osw {

  //! The OSW Patch class
  /** 
   * A patch is just a transform which contains other transforms
   * and connections between them.
   * The stuff for "containing" other transforms is inherited
   * from Container
   * @sa Container, Transform, TimeDomainTransform
   */

  class _oswexport Patch : public TimeDomainTransform  {
    
  public:

    //! name of UI object (window) associated with patch
    State<string> window;

    //! unit value set when patch is fully loaded
    State<unit>   load;

    //! Constructor without argc, argv (used internally)
    /**
     * @param aname string name of patch
     * @param acontainer pointer to Container for patch
     * @sa TimeDomainTransform, Transform, Container
     */
    Patch (const string &aname, Container *acontainer) : 
      TimeDomainTransform (aname,acontainer),
      load("load",this,"Indicates that patch has finished loading") {
      xactivity = 0;
    }
    
    //! Patch Constructor
    /**
     * @param aname string name of patch
     * @param acontainer pointer to Container for patch
     * @param argc number of additional arguments to patch constuctor
     * @param argv array of strings for additional arguments to patch constructor
     * @sa TimeDomainTransform, Transform, Container
     */
    Patch (const string &aname, Container *acontainer, int argc, char *argv[]) :
      TimeDomainTransform (aname,acontainer,argc,argv),
      window("window",this,"",ScanArguments<string>("-window","",
						    acontainer,argc,argv)),
      load("load",this,"Indicates that patch has finished loading") {
      xactivity = 0;
    }
     
    //! Patch destructor
    ~Patch();

    //! Returns a script representing action to perform on UI double-click
    const char *Dialog () {
      return ("oswShowPatch " + window).c_str();
    }

    //! Add an argument (free initializable state variable) to patch
    void AddArgument (const string &aname, 
		      const string &atype, 
		      const string &adesc);

    //! Remove an argument (free initializable state variable) from patch
    void RemoveArgument (const string &aname);

    //! Increment the Activity count (number of activations currently executed within this patch)
    void IncrActivity () {
      ++xactivity;
    }

    //! Increase the Activity count (number of activations currently executed within this patch)
    void DecrActivity () {
      --xactivity;
    }
    
    //! Return the Activity count (number of activations currently executed within this patch)
    int GetActivity () const {return xactivity;}

    //! Clone a patch (including all of its member transforms and variables
    /**
     * @param aname optional string name of cloned patch,
     * @param acontainer optional container of cloned patch (default is container is patch being cloned)
     */
    Patch *Clone(const string &aname = "", Container *acontainer = NULL);
    
  protected:
    
    friend class OutletTransform;
    friend class InletTransform;
    friend class ThisTransform;

    //! The activity counter for patch
    int xactivity;

    //! maps names of patch outlets to corresponding OutletTransforms
    /**
     * @see OutletTransform
     */
    typedef stl::map<BaseState *,TimeDomainTransform *,stl::less<BaseState *> > 
        OutletTransformMap;
    
    //! the map of patch-outlet names to OutletTransforms
    OutletTransformMap  xoutletTransformMap;

    //! Add an outlet and its corresponding OutletTransform to this Patch
    void RegisterOutletTransform (BaseState *aoutlet, 
				  TimeDomainTransform *atransform) {
      xoutletTransformMap[aoutlet] = atransform;
    }

    //! Remove an oulet from this patch
    void UnregisterOutletTransform (BaseState *aoutlet) {
      xoutletTransformMap.erase(aoutlet);
    }

    //! Find the outlet transform given a pointer to an outlet
    TimeDomainTransform *FindOutletTransform (BaseState *aoutlet) {
      OutletTransformMap::iterator found = xoutletTransformMap.find(aoutlet);
      return (found == xoutletTransformMap.end()) ? NULL : found->second;
    }

    virtual bool PropagateSampleRate(float aSampleRate,
				     int aNumberOfSamples,
				     int alabel,
				     BaseState *startingPoint);
    virtual bool PropagateNumberOfSamples(float aSampleRate,
					  int aNumberOfSamples,
					  int alabel,
					  BaseState *startingPoint);

    //! List of self references to this patch (This transforms)
    /**
     * @see ThisTransform
     */
    typedef stl::list<Transform *> SelfReferenceList;

    //! List of self references to this patch (This transforms)
    /**
     * @see ThisTransform
     */
    SelfReferenceList xself_references;

    //! Add a self referennce (This transform)
    /**
     * @see ThisTransform
     */
    void AddSelfReference (Transform *toAdd) {
      xself_references.push_back(toAdd);
    }

    //! Remove a self reference (This transform)
    /**
     * @see ThisTransform
     */
    void RemoveSelfReference (Transform *toRemove) {
      xself_references.erase(remove(xself_references.begin(),
                                    xself_references.end(),
                                    toRemove),
                             xself_references.end());
    }

    //! Update self-reference transforms (e.g., fix inlets/outlets, etc.)
    /**
     * @see ThisTransform
     */
    void FixSelfReferences();

    //! Add a new outlet to self-reference transforms
    /**
     * @see ThisTransform
     */
    void AddOutletToSelfReferences(BaseState *toAdd);
    
    //! Remove an outlet from self-reference transforms
    /**
     * @see ThisTransform
     */
    void RemoveOutletFromSelfReferences(BaseState *toRemove);

    //! Add an inlet to self-reference transforms
    /**
     * @see ThisTransform
     */
    void AddInletToSelfReferences(BaseState *toAdd);

    //! Remove an inlet from self-references
    /**
     * @see ThisTransform
     */
    void RemoveInletFromSelfReferences(BaseState *toRemove);
       
    
    
  };

  //! definition of "proc" from Activation
  /**
   * @sa Activation Patch Transform
   */
  template<class T, class functor>
  inline void Activation<T,functor>::proc () {
    ProfileOn(xcontainer);
    Transform *transform;
    if ((transform = dynamic_cast<Transform *>(xcontainer))) {
      // If container is a transform, then maintain its Patch activity count
      if (transform->GetPatch()) {
        transform->GetPatch()->IncrActivity();
      }
      xproc(xcontainer); 
      if (transform->GetPatch()) {
        transform->GetPatch()->DecrActivity();
      }
    } else {
      xproc(xcontainer);
    }
    ProfileOff(xcontainer);
  }
}

#endif //  _OSW_PATCH_H
