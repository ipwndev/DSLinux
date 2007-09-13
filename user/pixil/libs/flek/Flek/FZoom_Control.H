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

#ifndef _FZOOM_CONTROL_H_
#define _FZOOM_CONTROL_H_

#include <Flek/FVector3.H>
#include <Flek/FMatrix4x4.H>
#include <Flek/math.H>

/**
 * @package libflek_gl
 * 
 * Class for a zoom controller.
 * Implements zooming by scaling in the current XY plane.
 */
class FZoom_Control {
  
protected:
  
  // Current transformation matrix:
  FMatrix4x4 mNow;

  // Drag state
  bool Dragging;

  // Mouse x coordinate
  float vNowx, vDownx;

  FVector3 zNow, zDown;

public:

  /**
   * Default constructor.
   */
  FZoom_Control()
    : mNow(), Dragging(false), vNowx(0.0), vDownx(0.0), zNow(1,1,1), zDown(1,1,1)
  {}

  /**
   * Copy constructor.
   */
  FZoom_Control(const FZoom_Control& tc)
    : mNow(tc.mNow), Dragging(tc.Dragging), vNowx(tc.vNowx), vDownx(tc.vDownx),
      zNow(tc.zNow), zDown(tc.zDown)
  {}

  /**
   * Destructor.
   */
  ~FZoom_Control() {}

  /**
   * Assignment operator.
   */
  FZoom_Control& operator = (const FZoom_Control& tc) {
    mNow = tc.mNow; Dragging = tc.Dragging; vNowx = tc.vNowx; vDownx = tc.vDownx;
    zNow = tc.zNow; zDown = tc.zDown;
    return (*this);
  }
  
  /**
   * Reset the zoom controller to default values.
   */
  void reset (void) {
    zNow.set(1,1,1); zDown.set(1,1,1); mNow.reset();
  }
  
  /**
   * Specify mouse position.
   */
  void mouse (const FVector3& pos) {
    vNowx = pos[0];
  }

  /**
   * Specify mouse position.
   */
  void mouse (float x, float y=0.0, float z=0.0) {
    vNowx = x;
  }
  
  /**
   * Get the translation matrix.
   */
  FMatrix4x4 value (void) const {
    return mNow;
  }

  /**
   * Get the scale vector.
   */
  FVector3 zoom_value (void) const {
    return zNow;
  }
  
  /**
   * Begin a drag.
   */
  void begin_drag (void) {
    Dragging = true; 
    vDownx = vNowx;
  }
     
  /** 
   * End a drag.
   */
  void end_drag (void) {
    Dragging = false; 
    zDown = zNow;
  }
  
  /**
   * Check dragging status.
   */
  bool dragging (void) const {
    return Dragging;
  }
  
  /** 
   * Update the FVectors and FMatrices
   */
  void update (void) {
    if (Dragging) {
      zNow = zDown;
      
      // Mapping between mouse movement and scale change is as follows
      // For a mouse movement of 1.0, scale changes by a factor of 2
      // Note: This is in the transformed mouse coordinates (-1 to 1)
      // Also only the x mouse coordinate is used
      float diff = vNowx - vDownx;
      float fact = 1 + absolute (diff);
      if ( diff < 0.0 ) zNow /= fact;
      else zNow *= fact;
      
      // All 3 coordinates of the zoom FVector are changed to get proper
      // view scaling (rotate after zoom, etc.)
      
      // Fill in transposed order for GL
      mNow[0][0] = zNow[0];
      mNow[1][1] = zNow[1];
      mNow[2][2] = zNow[2];
    }
  }
};

#endif
