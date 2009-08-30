// Nucleus/Make.h
// --------------
//
// Maintained by Joshua Juran

// Part of the Nitrogen project.
//
// Written 2004-2007 by Lisa Lippincott and Joshua Juran.
//
// This code was written entirely by the above contributors, who place it
// in the public domain.


#ifndef NUCLEUS_MAKE_H
#define NUCLEUS_MAKE_H

/*
   Make is the generic Nitrogen function for making structures.  It
   acts as a constructor for types that don't have declared constructors.
   
   The Make template functions forward calls to the Maker functor,
   so that specializations can use overloading and templates.
   
   Here's a sample extension of Make:
   
      template <>
      struct Maker< Point >
        {
         Point operator()( short v, short h ) const
           {
            Point result;
            result.v = v;
            result.h = h;
            return result;
           }
         
         Point operator()() const
           {
            return operator()( 0, 0 );
           }
        };
   
   The Nitrogen convention is that the basic parameter list for Maker<T>::operator()
   should be the fields of T, in the order they are declared, but skipping members that
   are solely for alignment.  Other parameter lists may be provided that assign useful
   default values.
*/

namespace Nucleus
  {
   template < class R >
   struct Maker;
   
   template < class R >
   inline R Make()
     {
      return Maker<R>()();
     }

   template < class R, class P0 >
   inline R Make( P0 p0 )
     {
      return Maker<R>()( p0 );
     }

   template < class R, class P0, class P1 >
   inline R Make( P0 p0, P1 p1 )
     {
      return Maker<R>()( p0, p1 );
     }

   template < class R, class P0, class P1, class P2 >
   inline R Make( P0 p0, P1 p1, P2 p2 )
     {
      return Maker<R>()( p0, p1, p2 );
     }

   template < class R, class P0, class P1, class P2, class P3 >
   inline R Make( P0 p0, P1 p1, P2 p2, P3 p3 )
     {
      return Maker<R>()( p0, p1, p2, p3 );
     }

   template < class R, class P0, class P1, class P2, class P3, class P4 >
   inline R Make( P0 p0, P1 p1, P2 p2, P3 p3, P4 p4 )
     {
      return Maker<R>()( p0, p1, p2, p3, p4 );
     }

   template < class R, class P0, class P1, class P2, class P3, class P4, class P5 >
   inline R Make( P0 p0, P1 p1, P2 p2, P3 p3, P4 p4, P5 p5 )
     {
      return Maker<R>()( p0, p1, p2, p3, p4, p5 );
     }

   template < class R, class P0, class P1, class P2, class P3, class P4, class P5, class P6 >
   inline R Make( P0 p0, P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6 )
     {
      return Maker<R>()( p0, p1, p2, p3, p4, p5, p6 );
     }

   template < class R, class P0, class P1, class P2, class P3, class P4, class P5, class P6, class P7 >
   inline R Make( P0 p0, P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6, P7 p7 )
     {
      return Maker<R>()( p0, p1, p2, p3, p4, p5, p6, p7 );
     }
  }

#endif
