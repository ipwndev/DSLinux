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

#ifndef _FDOLLY_CONTROL_H_
#define _FDOLLY_CONTROL_H_

#include <Flek/FVector3.H>
#include <Flek/FMatrix4x4.H>

/**
 * @package libflek_gl
 * 
 * Class for a dolly controller.
 */
class FDolly_Control {

protected:
  
  // Current transformation matrix
  FMatrix4x4 mNow;

  bool Dragging;

  // Mouse translations/points
  float vNow, vDown;

  // Current dolly
  float dNow, dDown;

  // For scaling given coordinate value
  float scale_factor;
  
public:
  
  /**
   * Default constructor
   */
  FDolly_Control (float sf=1.0)
    : mNow(), Dragging(false), vNow(0), vDown(0), dNow(0), dDown(0),
      scale_factor(sf)
  {}
  
  /**
   * Copy constructor
   */
  FDolly_Control (const FDolly_Control& dc)
    : mNow(dc.mNow), Dragging(dc.Dragging), vNow(dc.vNow), vDown(dc.vDown),
      dNow(dc.dNow), dDown(dc.dDown), scale_factor(dc.scale_factor)
  {}
  
  /**
   * Destructor
   */
  ~FDolly_Control()
  {}
  
  FDolly_Control& operator = (const FDolly_Control& dc) // Assignment operator
  {
    mNow = dc.mNow; Dragging = dc.Dragging; vNow = dc.vNow; vDown = dc.vDown;
    dNow = dc.dNow; dDown = dc.dDown; scale_factor = dc.scale_factor;
    return (*this);
  }
  
  void reset (void) {
    dNow = 0.0; dDown = 0.0 ; mNow.reset();
  }
  
  void set_scale(float sf) {
    scale_factor = sf;
  }
  
  void reset_scale(void) {
    scale_factor = 1.0;
  }
  
  /**
   * Specify mouse position.
   */
  void mouse (const FVector3& pos) {
    // only z coordinate is used
    vNow = scale_factor*pos[2];
  }
  
  /**
   * Specify mouse position.
   */
  void mouse (float z) {
    vNow = scale_factor*z;
  }
  
  /**
   * Get the translation matrix.
   */
  FMatrix4x4 value (void) const {
    return mNow;
  }
  
  /**
   * Get the dolly value.
   */
  float dolly_value (void) const {
    return dNow;
  }
  
  /**
   * Get dolly value as FVector.
   */
  FVector3 dolly_vector (void) const {
    // dolly value is in z
    return FVector3(0,0,dNow);
  }
  
  /**
   * Begin a drag.
   */
  void begin_drag (void) {
    Dragging = true; vDown = vNow;
  }
  
  /**
   * End a drag.
   */
  void end_drag (void) {
    Dragging = false; dDown = dNow;
  }

  /**
   * Check dragging status
   */
  bool dragging (void) const {
    return Dragging;
  }
  
  /**
   * Update the FVectors and matrices.
   */
  void update (void) {
    if (Dragging) {
      dNow = dDown; dNow += vNow; dNow -= vDown;
      
      // Fill in transposed order for GL
      mNow[3][0] = 0.0;
      mNow[3][1] = 0.0;
      mNow[3][2] = dNow;
    }
  }
};

#endif
