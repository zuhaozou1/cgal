// Copyright (c) 1997-2001  ETH Zurich (Switzerland).
// All rights reserved.
//
// This file is part of CGAL (www.cgal.org); you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License as
// published by the Free Software Foundation; version 2.1 of the License.
// See the file LICENSE.LGPL distributed with CGAL.
//
// Licensees holding a valid commercial license may use this file in
// accordance with the commercial license agreement provided with the software.
//
// This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
// WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
//
// $URL: svn+ssh://gaertner@scm.gforge.inria.fr/svn/cgal/trunk/Number_types/include/CGAL/Gmpzf.h $
// $Id: Gmpzf.h 33782 2006-08-25 14:06:31Z gaertner $
// 
//
// Author(s)     : Bernd Gaertner <gaertner@inf.ethz.ch>

#ifndef CGAL_GMPZF_TYPE_H
#define CGAL_GMPZF_TYPE_H

// includes
#include <CGAL/basic.h>
#include <CGAL/Handle_for.h>
#include <gmp.h>
#include <mpfr.h>
#include <CGAL/Quotient.h>
#include <CGAL/Gmpz_type.h>
#include <boost/operators.hpp>
#include <iostream>
#include <cmath>
#include <limits>
#include <string>

CGAL_BEGIN_NAMESPACE

//internal fwd 
class Gmpzf;
bool operator<(const Gmpzf &a, const Gmpzf &b);
bool operator==(const Gmpzf &a, const Gmpzf &b);
bool operator<(const Gmpzf &a, int b);
bool operator==(const Gmpzf &a, int b);
bool operator>(const Gmpzf &a, int b);

struct Gmpzf_rep // as in Gmpz.h
{
// FIXME : bug if ~() is called before an mpz_init*() is called.
// not a problem in practice, but not nice.
// maybe the mpz_init_set* functions should move back to Gmpz_rep.
// But then we should use the Storage_traits::construct/get...

  mpz_t mpZ;

  Gmpzf_rep() {}
  ~Gmpzf_rep() { mpz_clear(mpZ); }

private:
  // Make sure it does not get accidentally copied.
  Gmpzf_rep(const Gmpzf_rep &);
  Gmpzf_rep & operator= (const Gmpzf_rep &);
};

// class declaration
// =================
// This is an exact floating point type; it can represent numbers
// of the form m*2^e, where m is of type mpz_t and e (currently)
// of type long

class Gmpzf :
  Handle_for<Gmpzf_rep>,
    boost::ordered_euclidian_ring_operators1< Gmpzf
  , boost::ordered_euclidian_ring_operators2< Gmpzf, int
    > >
{
  typedef Handle_for<Gmpzf_rep> Base; 

public:
  // tags
  // ----
  typedef Tag_true   Has_gcd;
  typedef Tag_true   Has_division;
  typedef Tag_true   Has_sqrt;
  typedef Tag_true   Has_exact_ring_operations;
  typedef Tag_true   Has_exact_division;
  typedef Tag_false  Has_exact_sqrt;

  // exponent type
  // --------------------
  typedef long Exponent; // may overflow, but if it does, the mantissa is
                         // potentially too large to be useful, anyway;
                         // still, repeated squaring of a power of two
                         // quickly brings this type to its limits...

private:
  // data members (mantissa is from Gmpzf_rep)
  // ----------------------------------------
  // Invariant: 
  // - number is in canonical form, i.e.(m,e) == 0 or m is odd
  Exponent  e; 

public:
  // access
  // ------
  const mpz_t& man() const 
  {
    return Ptr()->mpZ;
  }

  mpz_t& man()
  {
    return ptr()->mpZ;
  }

  const Exponent& exp() const
  {
    return e;
  }
 
  // construction
  // ------------

  Gmpzf( )
    : e(0)
  {
    mpz_init(man());
    CGAL_postcondition(is_canonical());
  }


  Gmpzf(const mpz_t z)
    : e(0)
  { 
    mpz_init_set(man(), z); 
    canonicalize();
  }  


  Gmpzf(const Gmpz& n )
    : e(0)
  { 
    mpz_init_set(man(), n.mpz()); 
    canonicalize();
  }


  Gmpzf( int i)
    : e(0)
  {   
    mpz_init_set_si( man(), i);  
    canonicalize();
  }

  Gmpzf( long l)
    : e(0)
  {   
    mpz_init_set_si( man(), l);
    canonicalize();
  }

  Gmpzf( double d)    
  {
    Protect_FPU_rounding<> P(CGAL_FE_TONEAREST);
    if (d == 0) {
      mpz_init (man());
      e = 0;
      return;
    }
    static int p = std::numeric_limits<double>::digits;
    CGAL_assertion(CGAL_NTS is_finite(d) && is_valid(d));
    int exp;
    double x = std::frexp(d, &exp); // x in [1/2, 1], x*2^exp = d
    mpz_init_set_d (man(), // to the following integer:   
		    std::ldexp( x, p)); 
    e = exp - p;
    canonicalize();
  }

  // arithmetics 
  // -----------
  Gmpzf operator+() const; 
  Gmpzf operator-() const; 
  Gmpzf& operator+=( const Gmpzf& b);
  Gmpzf& operator+=( int i);
  Gmpzf& operator-=( const Gmpzf& b);
  Gmpzf& operator-=( int i);
  Gmpzf& operator*=( const Gmpzf& b);
  Gmpzf& operator*=( int i);
  Gmpzf& operator/= (const Gmpzf& b);
  Gmpzf& operator%= (const Gmpzf& b);
  Gmpzf& operator/= (int i);
  Gmpzf& operator%= (int i); 
  bool is_zero() const;
  Sign sign() const;
  Gmpzf integral_division(const Gmpzf& b) const;
  Gmpzf gcd (const Gmpzf& b) const;
  Gmpzf sqrt() const;
  Comparison_result compare (const Gmpzf &b) const;
  double to_double() const ;
 
private:
  void canonicalize();
  bool is_canonical() const;
  static void align ( const mpz_t*& a_aligned, const mpz_t*& b_aligned, 
		     Exponent& rexp, const Gmpzf& a, const Gmpzf& b);  
};



    

// implementation
// ==============

// arithmetics 
// -----------

inline
Gmpzf Gmpzf::operator+() const 
{
    return *this;
}

inline
Gmpzf Gmpzf::operator-() const 
{
  Gmpzf result;
  mpz_neg (result.man(), man());
  result.e = exp();
  CGAL_postcondition(is_canonical());
  return result;
}

inline
Gmpzf& Gmpzf::operator+=( const Gmpzf& b)
{  
  Gmpzf result;
  if (b.is_zero()) return *this; // important in sparse contexts
  const mpz_t *a_aligned, *b_aligned;
  align (a_aligned, b_aligned, e, *this, b);
  mpz_add(result.man(), *a_aligned, *b_aligned);
  swap(result);
  canonicalize();
  return(*this);
}

inline
Gmpzf& Gmpzf::operator+=( int i)
{
  return operator+=(Gmpzf (i));   // could be optimized, but why?
}

inline
Gmpzf& Gmpzf::operator-=( const Gmpzf& b)
{    
  Gmpzf result;
  if (b.is_zero()) return *this; // important in sparse contexts
  const mpz_t *a_aligned, *b_aligned;
  align (a_aligned, b_aligned, e, *this, b);
  mpz_sub(result.man(), *a_aligned, *b_aligned);
  swap(result);
  canonicalize();
  return(*this);   
}

inline
Gmpzf& Gmpzf::operator-=( int i)
{
  return operator-=(Gmpzf (i));   // could be optimized, but why?
}

inline
Gmpzf& Gmpzf::operator*=( const Gmpzf& b)
{ 
  Gmpzf result;
  mpz_mul(result.man(), man(), b.man());
  e += b.exp();
  swap (result);
  canonicalize();
  return *this; 
}  

inline
Gmpzf& Gmpzf::operator*=( int i)
{
  Gmpzf result;
  mpz_mul_si(result.man(), man(), i);   
  swap (result);
  canonicalize();
  return *this; 
}

// *this = m1 * 2 ^ e1 = a_aligned * 2 ^ rexp
//     b = m2 * 2 ^ e2 = b_aligned * 2 ^ rexp,   where rexp = min (e1, e2)
// 
// => a / b = a div b = (a_aligned div b_aligned)
//            a mod b = (a_aligned mod b_aligned) * 2 ^ rexp
inline
Gmpzf& Gmpzf::operator/= (const Gmpzf& b)
{
  CGAL_precondition(!b.is_zero());
  Gmpzf result;
  const mpz_t *a_aligned, *b_aligned;
  align (a_aligned, b_aligned, e, *this, b);
  mpz_tdiv_q (result.man(), *a_aligned, *b_aligned); // round towards zero
  e = 0; 
  swap(result);
  canonicalize();
  return(*this);   
}

inline
Gmpzf& Gmpzf::operator%= (const Gmpzf& b)
{
  CGAL_precondition(!b.is_zero());
  Gmpzf result;
  const mpz_t *a_aligned, *b_aligned;
  align (a_aligned, b_aligned, e, *this, b);
  mpz_tdiv_r (result.man(), *a_aligned, *b_aligned);
  swap(result);
  canonicalize();
  return(*this);   
}

inline
Gmpzf& Gmpzf::operator/= (int i)
{
  return operator/= (Gmpzf(i));
}

inline
Gmpzf& Gmpzf::operator%= (int i) 
{
  return operator%= (Gmpzf(i));
}

inline
bool Gmpzf::is_zero() const
{
  return mpz_sgn( man()) == 0;
}

inline
Sign Gmpzf::sign() const 
{
  return static_cast<Sign>(mpz_sgn( man()));
}

inline
Gmpzf Gmpzf::integral_division(const Gmpzf& b) const
{
  Gmpzf result;
  mpz_divexact(result.man(), man(), b.man());
  result.e = exp()-b.exp(); 
  result.canonicalize();
  CGAL_postcondition (*this == b * result);
  return result;
}

inline
Gmpzf Gmpzf::gcd (const Gmpzf& b) const
{
  Gmpzf result;
  mpz_gcd (result.man(), man(), b.man()); // exponent is 0
  result.canonicalize();
  return result;  
}

inline
Gmpzf Gmpzf::sqrt() const
{
  // is there a well-defined sqrt at all?? Here we do the
  // following: write *this as m * 2 ^ e with e even, and 
  // then return sqrt(m) * 2 ^ (e/2)
  Gmpzf result;
  // make exponent even
  if (exp() % 2 == 0) {
    mpz_set (result.man(), man());
  } else {
    mpz_mul_2exp (result.man(), man(), 1); 
  }
  mpz_sqrt(result.man(), result.man());
  result.e = exp() / 2;
  result.canonicalize();
  return result;  
}

inline
Comparison_result Gmpzf::compare (const Gmpzf &b) const
{
  const mpz_t *a_aligned, *b_aligned;
  Exponent rexp; // ignored
  align (a_aligned, b_aligned, rexp, *this, b);
  int c = mpz_cmp(*a_aligned, *b_aligned);
  if (c < 0) return SMALLER;
  if (c > 0) return LARGER;
  return EQUAL;
}

inline
double Gmpzf::to_double() const
{
    return std::ldexp( mpz_get_d(man()),exp());
}

inline  
void Gmpzf::canonicalize()
{
  if (!is_zero()) {
    // chop off trailing zeros in m
    unsigned long zeros = mpz_scan1(man(), 0);
    mpz_tdiv_q_2exp( man(), man(), zeros);  // bit-wise right-shift
    e += zeros;
  } else {
    e = 0;
  }
  CGAL_postcondition(is_canonical());
}

inline
bool Gmpzf::is_canonical() const
{
  return (is_zero() && e==0) || mpz_odd_p (man());
}

// align a and b such that they have the same exponent:
// a = m1 * 2 ^ e1 -> a_aligned * 2 ^ rexp,
// b = m2 * 2 ^ e2 -> b_aligned * 2 ^ rexp,   where rexp = min (e1, e2)
// 
// function sets (pointers to) a_aligned and b_aligned and rexp;
// it uses the static s to store the shifted number
inline
void Gmpzf::align ( const mpz_t*& a_aligned, 
			   const mpz_t*& b_aligned, 
			   Exponent& rexp, 
			   const Gmpzf& a, const Gmpzf& b) {
  static Gmpz s;
  switch (CGAL_NTS compare (b.exp(), a.exp())) {
  case SMALLER:
    // leftshift of a to reach b.exp()
    mpz_mul_2exp (s.mpz(), a.man(), a.exp() - b.exp()); 
    a_aligned = &s.mpz();  // leftshifted a
    b_aligned = &b.man();  // b
    rexp = b.exp();
    break;
  case LARGER:
    // leftshift of b to reach a.exp()
    mpz_mul_2exp (s.mpz(), b.man(), b.exp() - a.exp());
    a_aligned = &a.man(); // a
    b_aligned = &s.mpz(); // leftshifted b
    rexp = a.exp();
    break;
  case EQUAL:
    a_aligned = &a.man();
    b_aligned = &b.man();
    rexp = a.exp();
  }
}

// input/output
// ------------
inline
std::ostream& operator<< (std::ostream& os, const Gmpzf& a) 
{
    return os << a.to_double();
}

inline
std::ostream& print (std::ostream& os, const Gmpzf& a) 
{
  return os << a.man() << "*2^" << a.exp();
}

inline
std::istream&  operator>> ( std::istream& is, Gmpzf& a) 
{
  // simply read from double
  double d;
  is >> d;
  if (is.good()) 
    a = Gmpzf(d);
  return is;
}

// comparisons
// -----------

inline
bool operator<(const Gmpzf &a, const Gmpzf &b)
{ 
  return a.compare(b) == SMALLER;
}

inline
bool operator==(const Gmpzf &a, const Gmpzf &b)
{ 
  return ( (mpz_cmp(a.man(), b.man()) == 0) && a.exp() == b.exp() );
}

// mixed operators
inline
bool operator<(const Gmpzf &a, int b)
{
  return operator<(a, Gmpzf(b));
}

inline
bool operator==(const Gmpzf &a, int b)
{
  return operator==(a, Gmpzf(b));
}

inline
bool operator>(const Gmpzf &a, int b)
{
  return operator>(a, Gmpzf(b));
}

CGAL_END_NAMESPACE

#endif // CGAL_GMPZF_TYPE_H

// ===== EOF ==================================================================
