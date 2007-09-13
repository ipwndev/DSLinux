/* -*-C++-*- 

   "$Id$"
   
   Copyright 1999-2000 by the Flek development team.
   
   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
   
   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.
   
   You should have received a copy of the GNU Library General Public
   License along with this library; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
   USA.
   
   Please report all bugs and problems to "flek-devel@sourceforge.net".

*/

#ifndef _FTRANS_CONTROL_H_
#define _FTRANS_CONTROL_H_

#include <Flek/FVector3.H>
#include <Flek/FMatrix4x4.H>

/**
 * @package libflek_gl
 * 
 * Class for a translation controller
 */
class FTrans_Control {

protected:

  // Current transformation matrix
  FMatrix4x4 mNow;
  bool Dragging;

  // Mouse translations/points
  FVector3 vNow, vDown;

  // Current translation
  FVector3 tNow, tDown;

public:
  
  /**
   * Default constructor.
   */
  FTrans_Control ()
    : mNow(), Dragging(false), vNow(), vDown(), tNow(), tDown()
  {}
  
  /**
   * Copy constructor.
   */
  FTrans_Control (const FTrans_Control& tc)
    : mNow(tc.mNow), Dragging(tc.Dragging), vNow(tc.vNow), vDown(tc.vDown),
      tNow(tc.tNow), tDown(tc.tDown)
  {}
  
  /**
   * Destructor
   */
  ~FTrans_Control ()
  {}
  
  FTrans_Control& operator = (const FTrans_Control& tc) {
    mNow = tc.mNow; Dragging = tc.Dragging; vNow = tc.vNow; vDown = tc.vDown;
    tNow = tc.tNow; tDown = tc.tDown;
    return (*this);
  }
  
  void reset (void) {
    tNow.reset(); tDown.reset(); mNow.reset();
  }
  
  /**
   * Specify mouse position.
   */
  void mouse (const FVector3& pos) {
    vNow = pos;
  }
  
  /**
   * Specify mouse position.
   */
  void mouse (float x, float y, float z=0.0) {
    vNow.set(x,y,z);
  }
     
  /**
   * Get the translation matrix.
   */
  FMatrix4x4 value (void) const {
    return mNow;
  }

  /**
   * Get the translation FVector
   */
  FVector3 trans_value (void) const {
    return tNow;
  }
  
  /**
   * Begin a drag.
   */
  void begin_drag (void) {
    Dragging = true; vDown = vNow;
  }
  
  /** 
   * End a drag
   */
  void end_drag (void) {
    Dragging = false; tDown = tNow;
  }

  /**
   * Check dragging status.
   */
  bool dragging (void) const {
    return Dragging;
  }

  /**
   * Update the FVectors and matrices.
   */
  void update (void) {
    if (Dragging) {
      tNow = tDown; tNow += vNow; tNow -= vDown;
      
      // Fill in transposed order for GL
      mNow[3][0] = tNow[0];
      mNow[3][1] = tNow[1];
      mNow[3][2] = tNow[2];
    }
  }
};

#endif
