
/* 
Copyright (c) 2001-2003 Amar Chaudhary. All rights reserved.
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
/* oswNameable.h
   The Nameable class, base class for all major components in OSW
*/

#ifndef _OSW_NAMEABLE_H
#define _OSW_NAMEABLE_H

namespace osw {

  //! The base class of all "nameable" objects in OSW
  /** The base class of all "nameable" objects in OSW, including transforms,
      inlets, outlets, state variables, etc.  Each nameable object has a
      local string name and a path name, similar to a full path name of a 
      file or URL.
  */

  class _oswexport Nameable {
    
  public:
    
    friend class _oswexport Container;
    

    //! The constructor for Nameable
    /**
     * @param aname a String containing the local name of the Nameable
     * @param acaontainer a pointer to a Container containing the Nameable
     * The path name of a nameable will be the path name of the container
     * followed by a "/" character and the value of aname.
     * @see Container
     */
    Nameable (const String& aname, Container *acontainer = NULL);
    
    //! The destructor for Nameable
    /**
     * This descructor automatically removes a Nameable from the OSW
     * hierarchical name space
     */
    virtual ~Nameable ();

    //! The local name of a Nameable
    const String & Name () const {return xname;}
    
    //! The full path name (hierarchical name) of a Nameable
    const String & PathName () const {return xpath;}
    
    //! Return a nameable that matches a given string name
    /**
     * @param toFind a String containing the path name to find
     * @param acontainer a pointer to a Container to use as a root for the path name
     * If container is supplied, it is prepended to the value of toFind before
     * searching for the variable
     * @see Container
     */
    static Nameable * Find (const String &toFind, 
			    Container *acontainer = NULL);
    
    //! Return a nameable that matches a given string name
    /**
     * @param toFind a String containing the path name to find
     * @param acontainer a pointer to a Container to use as a root for the path name
     * If container is supplied, it is prepended to the value of toFind before
     * searching for the variable
     * @see Container
     */
    inline friend Nameable * Find (const String &toFind, 
			    Container *acontainer = NULL) {
      return Nameable::Find(toFind,acontainer);
    }

    //! Registers a Nameable with the OSW hierarchical name space
    /**
     * @param toRegister a pointer to the Nameable to Register
     * This function is automatically called by the Nameable constructor
     */
    static void	Register (Nameable *toRegister);
    //! Removes a Nameable from the OSW hierarchical name space
    /**
     * @param toRegister a pointer to the Nameable to remove
     * This function is automatically called by the Nameable destructor
     */
    static void Unregister (const String &nameToUnregister, 
			    Nameable *objToUnregister);
    
    //! Enumerates the path names of all Nameables in the OSW hierarchical name space to a given stream
    /**
     * @param os a valid output stream (ostream)
     */
    static ostream & EnumerateToStream (ostream &os);

    //! Returns the number of Namables in the OSW hierarchical name space
    static int NumberOfNameables ();
    
    //! Renames a Nameable
    /**
     * Rename changes the local name of a Nameable, but does not change its path name (i.e., the name of its Container)
     * @see Container
     */
    virtual void Rename (const String &aname);

    //! Returns the Container of a Nameable
    /**
     * @return a pointer to a Container
     * @see Container
     */
    Container *GetContainer () const {return xcontainer;}

    //! Get the OSW "major" version number (e.g., 1 for version 1.x, etc.)
    int MajorVersion () const {
      return OSW_MAJOR_VERSION;
    }

    //! Get the OSW "minor" version number (e.g., 2 for version x.2, etc.)
    int MinorVersion () const {
      return OSW_MINOR_VERSION;
    }

    //! Get the OSW "sub" version number (e.g., 3 for version x.y.3, etc.)
    int SubVersion () const {
      return OSW_SUB_VERSION;
    }

    //! Gets the total number of Nameables in the OSW name space
    int GetNumberOfNameables ();

    //! Gets the number of instances in the name space of the same class as this nameable
    /**
     * GetNumberOfInstances returns the number of instances in the OSW name space that have the same type (subclass of Nameable) as this object
     */
    int GetNumberOfInstances ();

    //! The table of all Nameables in the OSW hierarchical name space
    /**
     * Instances object should not be accessed/modified directly.
     * Various members of Nameable should be used instead.
     * @sa Find, BeginEnumerate, EndEnumerate, Rename, Register, Unregister
     */
#ifdef OSW_HASH_MAP_TRAITS
    typedef stl::hash_map<String, Nameable * > NameableTable;
#else
    typedef stl::hash_map<String, Nameable *, stl::hash<String>, stl::equal_to<String> > NameableTable;
#endif
    //! An enumerator (iterator) through the OSW hierarchical name space
    typedef NameableTable::iterator  enumerator;

    //! Initialize an OSW name-space enumerator
    /**
     * @return enumerator
     */
    static enumerator BeginEnumerate ();

    //! Check whether an enumerator has reached the end of the OSW name space
    /**
     * @param e a name-space enumerator
     * @return boolean value (true if at the end of the name space)
     */
    static bool EndEnumerate (enumerator e);

    //! Get a pointer to the Nameable referenced by a name-space enumerator
    static Nameable *GetNameableFromEnumerator (enumerator &e) {
      return e->second;
    }

    //! Get the local name of a Nameable referenced by a name-space enumerator
    static const String & GetNameFromEnumerator (enumerator &e) {
      return e->first;
    }

    //! Get the documentation associated with this Nameable
    /**
     * @return String
     * All Nameables have Documentation.
     */
    virtual String &Documentation () { return xdoc; }

    //! Set the documentation associated with this Nameable
    /**
     * @param adoc a String containing the new documentation value for this Nameable
     */
    virtual void SetDocumentation (const String &adoc) {
      xdoc = adoc;
    }
    
    //! Adds a new "top-level" object (Nameable without a Container) to the OSW name space
    /**
     * @see Container
     */
    void AddTopLevelNameable(Nameable *);
    //! Removes a new "top-level" object (Nameable without a Container) from the OSW name space
    /**
     * @see Container
     */
    void RemoveTopLevelNameable(Nameable *);

  protected:

    //! The table containing the full OSW name space 
    static NameableTable *xtable;

    //! The table of top-level objects in the OSW name space
    static NameableTable *xtoplevel_table;

    //! The per-class object counter for the OSW name space

#ifdef OSW_HASH_MAP_TRAITS
    static stl::hash_map<String,int > 
       *xcounter;
#else
    static stl::hash_map<String,int,stl::hash<String>,stl::equal_to<String> > 
       *xcounter;
#endif
 
    //! The local name of this Nameable
    String    xname;

    //! The path name of this Nameable
    String    xpath;

    //! The documentation of this Nameable
    String    xdoc;

    //! The container of this Nameable
    Container *xcontainer;
  };



}

#endif //  _OSW_NAMEABLE_H
