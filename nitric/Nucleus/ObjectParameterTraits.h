// Nucleus/ObjectParameterTraits.h
// -------------------------------

// Part of the Nitrogen project.
//
// Written 2002 by Lisa Lippincott.
//
// This code was written entirely by the above contributor, who places it
// in the public domain.


#ifndef NUCLEUS_OBJECTPARAMETERTRAITS_H
#define NUCLEUS_OBJECTPARAMETERTRAITS_H

namespace Nucleus
  {
   template < class Parameter > struct ObjectParameterTraits;
   
   template < class Object >
   struct ObjectParameterTraits< Object * >
     {
      typedef Object *Type;

      static const void *ConvertToPointer( Type object )     { return object; }
      static Type ConvertFromPointer( const void *p )        { return const_cast<Object *>( static_cast<const Object *>( p ) ); }
     };

   template < class Object >
   struct ObjectParameterTraits< const Object * >
     {
      typedef const Object *Type;

      static const void *ConvertToPointer( Type object )     { return object; }
      static Type ConvertFromPointer( const void *p )        { return static_cast<const Object *>( p ); }
     };

   template < class Object >
   struct ObjectParameterTraits< Object& >
     {
      typedef Object& Type;

      static const void *ConvertToPointer( Type object )     { return &object; }
      static Type ConvertFromPointer( const void *p )        { return *const_cast<Object *>( static_cast<const Object *>( p ) ); }
     };

   template < class Object >
   struct ObjectParameterTraits< const Object& >
     {
      typedef const Object& Type;

      static const void *ConvertToPointer( Type object )     { return &object; }
      static Type ConvertFromPointer( const void *p )        { return *static_cast<const Object *>( p ); }
     };

   template <>
   struct ObjectParameterTraits< void >
     {
      // When void is used as an object type, we specify a parameter of
      // type Null so that templated functions don't have to be specialized
      // to take one less parameter when void is chosen.  This parameter
      // should always be given a default value ObjectParameterTraits< void >::Type().
      class Null {};
      typedef Null Type;
      static const void *ConvertToPointer( Type )           { return 0; }
     };
  }

#endif
