/*
 * Software License Agreement (BSD License)
 *
 *  Copyright (c) 2011-2014, Willow Garage, Inc.
 *  Copyright (c) 2014-2016, Open Source Robotics Foundation
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above
 *     copyright notice, this list of conditions and the following
 *     disclaimer in the documentation and/or other materials provided
 *     with the distribution.
 *   * Neither the name of Open Source Robotics Foundation nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 *  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 *  COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 *  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 *  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 *  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 *  ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 */

/** \author Jia Pan */

#ifndef FCL_BV_FITTER_H
#define FCL_BV_FITTER_H

#include "fcl/math/triangle.h"
#include "fcl/BVH/BVH_internal.h"
#include "fcl/BV/kIOS.h"
#include "fcl/BV/OBBRSS.h"
#include <iostream>

namespace fcl
{

/// @brief Interface for fitting a bv given the triangles or points inside it.
template <typename BV>
class BVFitterBase
{
public:
  /// @brief Set the primitives to be processed by the fitter
  virtual void set(Vector3d* vertices_, Triangle* tri_indices_, BVHModelType type_) = 0;

  /// @brief Set the primitives to be processed by the fitter, for deformable mesh.
  virtual void set(Vector3d* vertices_, Vector3d* prev_vertices_, Triangle* tri_indices_, BVHModelType type_) = 0;

  /// @brief Compute the fitting BV
  virtual BV fit(unsigned int* primitive_indices, int num_primitives) = 0;

  /// @brief clear the temporary data generated.
  virtual void clear() = 0;
};

/// @brief The class for the default algorithm fitting a bounding volume to a set of points
template <typename BV>
class BVFitter : public BVFitterBase<BV>
{
public:
  /// @brief default deconstructor
  virtual ~BVFitter() {}

  /// @brief Prepare the geometry primitive data for fitting
  void set(Vector3d* vertices_, Triangle* tri_indices_, BVHModelType type_)
  {
    vertices = vertices_;
    prev_vertices = NULL;
    tri_indices = tri_indices_;
    type = type_;
  }

  /// @brief Prepare the geometry primitive data for fitting, for deformable mesh
  void set(Vector3d* vertices_, Vector3d* prev_vertices_, Triangle* tri_indices_, BVHModelType type_)
  {
    vertices = vertices_;
    prev_vertices = prev_vertices_;
    tri_indices = tri_indices_;
    type = type_;
  }

  /// @brief Compute a bounding volume that fits a set of primitives (points or triangles).
  /// The primitive data was set by set function and primitive_indices is the primitive index relative to the data
  BV fit(unsigned int* primitive_indices, int num_primitives)
  {
    BV bv;

    if(type == BVH_MODEL_TRIANGLES)             /// The primitive is triangle
    {
      for(int i = 0; i < num_primitives; ++i)
      {
        Triangle t = tri_indices[primitive_indices[i]];
        bv += vertices[t[0]];
        bv += vertices[t[1]];
        bv += vertices[t[2]];

        if(prev_vertices)                      /// can fitting both current and previous frame
        {
          bv += prev_vertices[t[0]];
          bv += prev_vertices[t[1]];
          bv += prev_vertices[t[2]];
        }
      }
    }
    else if(type == BVH_MODEL_POINTCLOUD)       /// The primitive is point
    {
      for(int i = 0; i < num_primitives; ++i)
      {
        bv += vertices[primitive_indices[i]];

        if(prev_vertices)                       /// can fitting both current and previous frame
        {
          bv += prev_vertices[primitive_indices[i]];
        }
      }
    }

    return bv;
  }

  /// @brief Clear the geometry primitive data
  void clear()
  {
    vertices = NULL;
    prev_vertices = NULL;
    tri_indices = NULL;
    type = BVH_MODEL_UNKNOWN;
  }

private:

  Vector3d* vertices;
  Vector3d* prev_vertices;
  Triangle* tri_indices;
  BVHModelType type;
};


/// @brief Specification of BVFitter for OBBd bounding volume
template<>
class BVFitter<OBBd> : public BVFitterBase<OBBd>
{
public:
  /// @brief Prepare the geometry primitive data for fitting
  void set(Vector3d* vertices_, Triangle* tri_indices_, BVHModelType type_)
  {
    vertices = vertices_;
    prev_vertices = NULL;
    tri_indices = tri_indices_;
    type = type_;
  }

  /// @brief Prepare the geometry primitive data for fitting, for deformable mesh
  void set(Vector3d* vertices_, Vector3d* prev_vertices_, Triangle* tri_indices_, BVHModelType type_)
  {
    vertices = vertices_;
    prev_vertices = prev_vertices_;
    tri_indices = tri_indices_;
    type = type_;
  }

  /// @brief Compute a bounding volume that fits a set of primitives (points or triangles).
  /// The primitive data was set by set function and primitive_indices is the primitive index relative to the data.
  OBBd fit(unsigned int* primitive_indices, int num_primitives);

  /// brief Clear the geometry primitive data
  void clear()
  {
    vertices = NULL;
    prev_vertices = NULL;
    tri_indices = NULL;
    type = BVH_MODEL_UNKNOWN;
  }

private:

  Vector3d* vertices;
  Vector3d* prev_vertices;
  Triangle* tri_indices;
  BVHModelType type;
};


/// @brief Specification of BVFitter for RSSd bounding volume
template<>
class BVFitter<RSSd> : public BVFitterBase<RSSd>
{
public:
  /// brief Prepare the geometry primitive data for fitting
  void set(Vector3d* vertices_, Triangle* tri_indices_, BVHModelType type_)
  {
    vertices = vertices_;
    prev_vertices = NULL;
    tri_indices = tri_indices_;
    type = type_;
  }

  /// @brief Prepare the geometry primitive data for fitting, for deformable mesh
  void set(Vector3d* vertices_, Vector3d* prev_vertices_, Triangle* tri_indices_, BVHModelType type_)
  {
    vertices = vertices_;
    prev_vertices = prev_vertices_;
    tri_indices = tri_indices_;
    type = type_;
  }

  /// @brief Compute a bounding volume that fits a set of primitives (points or triangles).
  /// The primitive data was set by set function and primitive_indices is the primitive index relative to the data.
  RSSd fit(unsigned int* primitive_indices, int num_primitives);

  /// @brief Clear the geometry primitive data
  void clear()
  {
    vertices = NULL;
    prev_vertices = NULL;
    tri_indices = NULL;
    type = BVH_MODEL_UNKNOWN;
  }

private:

  Vector3d* vertices;
  Vector3d* prev_vertices;
  Triangle* tri_indices;
  BVHModelType type;
};


/// @brief Specification of BVFitter for kIOSd bounding volume
template<>
class BVFitter<kIOSd> : public BVFitterBase<kIOSd>
{
public:
  /// @brief Prepare the geometry primitive data for fitting
  void set(Vector3d* vertices_, Triangle* tri_indices_, BVHModelType type_)
  {
    vertices = vertices_;
    prev_vertices = NULL;
    tri_indices = tri_indices_;
    type = type_;
  }

  /// @brief Prepare the geometry primitive data for fitting
  void set(Vector3d* vertices_, Vector3d* prev_vertices_, Triangle* tri_indices_, BVHModelType type_)
  {
    vertices = vertices_;
    prev_vertices = prev_vertices_;
    tri_indices = tri_indices_;
    type = type_;
  }

  /// @brief Compute a bounding volume that fits a set of primitives (points or triangles).
  /// The primitive data was set by set function and primitive_indices is the primitive index relative to the data.
  kIOSd fit(unsigned int* primitive_indices, int num_primitives);

  /// @brief Clear the geometry primitive data
  void clear()
  {
    vertices = NULL;
    prev_vertices = NULL;
    tri_indices = NULL;
    type = BVH_MODEL_UNKNOWN;
  }

private:
  Vector3d* vertices;
  Vector3d* prev_vertices;
  Triangle* tri_indices;
  BVHModelType type;
};


/// @brief Specification of BVFitter for OBBRSSd bounding volume
template<>
class BVFitter<OBBRSSd> : public BVFitterBase<OBBRSSd>
{
public:
  /// @brief Prepare the geometry primitive data for fitting
  void set(Vector3d* vertices_, Triangle* tri_indices_, BVHModelType type_)
  {
    vertices = vertices_;
    prev_vertices = NULL;
    tri_indices = tri_indices_;
    type = type_;
  }

  /// @brief Prepare the geometry primitive data for fitting
  void set(Vector3d* vertices_, Vector3d* prev_vertices_, Triangle* tri_indices_, BVHModelType type_)
  {
    vertices = vertices_;
    prev_vertices = prev_vertices_;
    tri_indices = tri_indices_;
    type = type_;
  }

  /// @brief Compute a bounding volume that fits a set of primitives (points or triangles).
  /// The primitive data was set by set function and primitive_indices is the primitive index relative to the data.
  OBBRSSd fit(unsigned int* primitive_indices, int num_primitives);

  /// @brief Clear the geometry primitive data
  void clear()
  {
    vertices = NULL;
    prev_vertices = NULL;
    tri_indices = NULL;
    type = BVH_MODEL_UNKNOWN;
  }

private:

  Vector3d* vertices;
  Vector3d* prev_vertices;
  Triangle* tri_indices;
  BVHModelType type;
};

}

#endif
