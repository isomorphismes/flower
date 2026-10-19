//	GeometryGamesFauxSimd.h
//
//	© 2022 by Jeff Weeks
//	See TermsOfUse.txt

#ifndef GeometryGamesFauxSimd_h
#define GeometryGamesFauxSimd_h

//	As of November 2020, there's no such thing as "simd_half3" or "simd_half4".
//	So for now let's create them manually.  Surprisingly enough,
//	this simple definition works!  It even produces 8-byte alignment
//	for "simd_half3", as required for consistency with the 8-byte alignment
//	of the Metal Shading Language's "half3" data type.

typedef __attribute__((__ext_vector_type__(2))) __fp16 faux_simd_half2;
typedef __attribute__((__ext_vector_type__(3))) __fp16 faux_simd_half3;
typedef __attribute__((__ext_vector_type__(4))) __fp16 faux_simd_half4;

#endif /* GeometryGamesFauxSimd_h */
