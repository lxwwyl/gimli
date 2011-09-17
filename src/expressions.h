/***************************************************************************
 *   Copyright (C) 2008-2011 by the resistivity.net development team       *
 *   Carsten R�cker carsten@resistivity.net                                *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#ifndef GIMLI_EXPRESSIONS__H
#define GIMLI_EXPRESSIONS__H

//** Idea taken from
// @Article{Veldhuizen95b,
//   author =       "Todd L. Veldhuizen",
//   title =        "Expression templates",
//   journal =      "C++ Report",
//   volume =       "7",
//   number =       "5",
//   pages =        "26--31",
//   month =        jun,
//   year =         "1995",
//   note =         "Reprinted in C++ Gems, ed. Stanley Lippman"
// }

#include <iostream>
#include <cmath>

namespace GIMLI{

class ExprIdentity;
template< class A > class Expr;
template< class A > class ExprLiteral;

struct XAxis__ {};
struct YAxis__ {};
struct ZAxis__ {};

template <class T> struct Variable {
};

template<> struct Variable< XAxis__ > {
    double operator()( const double x ) const { return x; }
    double operator()( const double x, const double y ) const { return x; }
    double operator()( const double x, const double y, const double z ) const { return x; }
};

template<> struct Variable< YAxis__ > {
    double operator()( const double x, const double y ) const { return y; }
    double operator()( const double x, const double y, const double z ) const { return y; }
};

template<> struct Variable< ZAxis__ > {
    double operator()( const double x, const double y, const double z ) const { return z; }
};

typedef Expr< ExprIdentity > DPlaceholder;
typedef Expr< ExprIdentity > IntPlaceholder;
typedef Expr< ExprIdentity > Placeholder;
typedef Expr< Variable<XAxis__> > VariableX;
typedef Expr< Variable<YAxis__> > VariableY;
typedef Expr< Variable<ZAxis__> > VariableZ;


inline double fabs( double a ) { return std::fabs( a ); }

//**! Simple show the results of the expression
template< class Ex > void show( Expr< Ex > expr, double start, double end, double step = 1.0 ){
    for ( double i = start; i < end; i += step ) std::cout << expr( i ) << std::endl;
}

template< class Ex > double sum( Expr< Ex > expr, double start, double end, double step ){
    double result = 0.0;
    for ( double i = start; i < end; i += step ) result += expr( i );
    return result;
}

template< class Ex > double sum( Expr< Ex > expr, double start, double end ){
    double result = 0.0;
    for ( double i = start; i < end; i += 1.0 ) result += expr( i );
    return result;
}
#ifndef PI
//** I redefine PI here, because vector.h should independ from alternative non std::headers, Ca
#define PI 3.141592653589793238462643383279502884197169399375105820974944592308
#define PI2 6.2831853071795864769252867665590057683942387987502116419498891846
#define PI_2 1.5707963267948967384626433682795028841971193993751058209749445923
#endif

#define DEFINE_BINARY_OPERATOR__( OP, FUNCT ) \
struct OP { template < class T > inline T operator()( const T & a, const T & b ) const { return a FUNCT b; } };\

DEFINE_BINARY_OPERATOR__( PLUS, + )
DEFINE_BINARY_OPERATOR__( MINUS, - )
DEFINE_BINARY_OPERATOR__( MULT, * )
DEFINE_BINARY_OPERATOR__( DIVID, / )

#undef DEFINE_BINARY_OPERATOR__

inline bool isEqual( const double & a, const double & b ) { return ( ::fabs( a - b ) < TOLERANCE ); }
inline bool isNonEqual( const double & a, const double & b ) { return !isEqual( a, b ); }
inline bool isEqual( const Complex & a, const Complex & b ) { return ( a == b ); }
inline bool isNonEqual( const Complex & a, const Complex & b ) { return ( a != b ); }

template < class T > bool isLesser( const T & a, const T & b ) { return a < b; }
template < class T > bool isGreater( const T & a, const T & b ) { return a > b; }
template < class T > bool isLesserEqual( const T & a, const T & b ) { return a < b || isEqual( a, b ); }
template < class T > bool isGreaterEqual( const T & a, const T & b ) { return a > b || isEqual( a, b ); }

template < class T > bool isInfNaN( const T & a ){ return ( std::isinf( a ) || std::isnan( a ) ); }

inline bool operator < ( const Complex & a, const Complex & b ) { return false; }
inline bool operator > ( const Complex & a, const Complex & b ) { return false; }

inline double square( const double & a ) { return a * a; }
inline double cot( const double & a ) { return 1.0 / std::tan( a ); }
inline double acot( const double & a ) { return PI / 2.0 * std::atan( a ); }
inline double sign( const double & a ) { return a > 0.0 ? 1.0 : (a < 0.0 ? -1.0 : 0.0); }
inline double exp10( const double & a ) { return std::pow( 10.0, a ); }

// template < class T > inline T square( const T & a ) { return a * a; }
// template < class T > inline T cot( const T & a ) { return 1.0 / std::tan( a ); }
// template < class T > inline T acot( const T & a ) { return PI / 2.0 * std::atan( a ); }
#define DEFINE_UNARY_OPERATOR__( OP, FUNCT ) \
struct OP { template < class T > T operator()( const T & a ) const { return FUNCT( a ); } };\

DEFINE_UNARY_OPERATOR__( ACOT, acot )
DEFINE_UNARY_OPERATOR__( ABS_, std::fabs )
DEFINE_UNARY_OPERATOR__( SIN , std::sin )
DEFINE_UNARY_OPERATOR__( COS , std::cos )
DEFINE_UNARY_OPERATOR__( COT , cot )
DEFINE_UNARY_OPERATOR__( TAN , std::tan )
DEFINE_UNARY_OPERATOR__( ATAN, std::atan )
DEFINE_UNARY_OPERATOR__( TANH, std::tanh )
DEFINE_UNARY_OPERATOR__( LOG , std::log )
DEFINE_UNARY_OPERATOR__( LOG10, std::log10 )
DEFINE_UNARY_OPERATOR__( EXP , std::exp )
DEFINE_UNARY_OPERATOR__( EXP10, exp10 )
DEFINE_UNARY_OPERATOR__( SQRT, std::sqrt )
DEFINE_UNARY_OPERATOR__( SIGN, sign )
DEFINE_UNARY_OPERATOR__( SQR,  square )

#undef DEFINE_UNARY_OPERATOR__

#define DEFINE_UNARY_IF_FUNCTION__( OP, FUNCT ) \
struct OP { template < class T > bool operator()( const T & a ) const { return FUNCT( a ); } }; \

DEFINE_UNARY_IF_FUNCTION__( ISINF, std::isinf )
DEFINE_UNARY_IF_FUNCTION__( ISNAN, std::isnan )
DEFINE_UNARY_IF_FUNCTION__( ISINFNAN, isInfNaN )

#undef DEFINE_UNARY_IF_FUNCTION__

/*! Expr is a wrapper class which contains a more interesting expression type,
such as ExprIdentity, ExprLiteral, unary (UnaryExprOp) or  binaray expression operator (BinaryExprOp). */
template< class A > class Expr {
public:
    Expr( ) : a_( A() ){ }

    Expr( const A & a ) : a_( a ){ }

    template < class ValueType > inline ValueType operator()( const ValueType & x ) const { return a_( x ); }
    template < class ValueType > inline ValueType operator()( const ValueType & x, const ValueType & y ) const { return a_( x, y ); }
    template < class ValueType > inline ValueType operator()( const ValueType & x, const ValueType & y, const ValueType & z ) const { return a_( x, y, z ); }
private:
    A a_;
};

class ExprIdentity {
public:
    ExprIdentity( ){ }

    template < class ValueType > inline ValueType operator( )( const ValueType & x ) const { return x; }
};

template < class ValueType > class ExprLiteral {
public:
    ExprLiteral( const ValueType & value ): value_( value ){ }

    inline ValueType operator()( const ValueType & x ) const { return value_; }
    inline ValueType operator()( const ValueType & x, const ValueType & y ) const { return value_; }
    inline ValueType operator()( const ValueType & x, const ValueType & y, const ValueType & z ) const { return value_; }
private:
    ValueType value_;
};

template< class A, class Op > class UnaryExprOp {
public:
    UnaryExprOp( const A & a ) : a_( a ){ }
    template < class ValueType > ValueType operator( ) ( const ValueType & x ) const {
        return Op()( a_( x ) );
    }
    template < class ValueType > ValueType operator( ) ( const ValueType & x, const ValueType & y ) const {
        return Op()( a_( x, y ) );
    }
    template < class ValueType > ValueType operator( ) ( const ValueType & x, const ValueType & y, const ValueType & z ) const {
        return Op()( a_( x, y, z ) );
    }
private:
    A a_;
};

template< class A, class B, class Op > class BinaryExprOp {
public:
    BinaryExprOp( const A & a, const B & b ) : a_( a ), b_( b ){ }
    template < class ValueType > ValueType operator( ) ( const ValueType & x ) const {
        return Op()( a_( x ), b_( x ) );
    }
    template < class ValueType > ValueType operator( ) ( const ValueType & x, const ValueType & y ) const {
        return Op()( a_( x, y ), b_( x, y ) );
    }
    template < class ValueType > ValueType operator( ) ( const ValueType & x, const ValueType & y, const ValueType & z ) const {
        return Op()( a_( x, y, z ), b_( x, y, z ) );
    }
private:
    A a_;
    B b_;
};

#define DEFINE_UNARY_EXPR_OPERATOR__( OP, FUNCT ) \
template< class A > Expr < UnaryExprOp < Expr< A >, FUNCT > >\
    OP( const Expr< A > & a ) { \
    typedef UnaryExprOp< Expr< A >, FUNCT > ExprT; \
    return Expr< ExprT >( ExprT( a ) );\
}\

DEFINE_UNARY_EXPR_OPERATOR__( abs,   ABS_ )
DEFINE_UNARY_EXPR_OPERATOR__( acot,  ACOT )
DEFINE_UNARY_EXPR_OPERATOR__( atan,  ATAN )
DEFINE_UNARY_EXPR_OPERATOR__( cos,   COS )
DEFINE_UNARY_EXPR_OPERATOR__( cot,   COT )
DEFINE_UNARY_EXPR_OPERATOR__( exp,   EXP )
DEFINE_UNARY_EXPR_OPERATOR__( exp10, EXP10 )
DEFINE_UNARY_EXPR_OPERATOR__( fabs,  ABS_ )
DEFINE_UNARY_EXPR_OPERATOR__( log,   LOG )
DEFINE_UNARY_EXPR_OPERATOR__( log10, LOG10 )
DEFINE_UNARY_EXPR_OPERATOR__( sign, SIGN )
DEFINE_UNARY_EXPR_OPERATOR__( sin,   SIN )
DEFINE_UNARY_EXPR_OPERATOR__( sqrt,  SQRT )
DEFINE_UNARY_EXPR_OPERATOR__( square, SQR )
DEFINE_UNARY_EXPR_OPERATOR__( tan,   TAN )
DEFINE_UNARY_EXPR_OPERATOR__( tanh,  TANH )

#undef DEFINE_UNARY_EXPR_OPERATOR__


#define DEFINE_EXPR_OPERATOR__( OP, FUNCT ) \
template< class ValueType, class A > Expr < BinaryExprOp < ExprLiteral< ValueType >, Expr< A >, FUNCT > >\
    operator OP ( const ValueType & x, const Expr< A > & a ) {\
    typedef BinaryExprOp< ExprLiteral< ValueType >, Expr< A >, FUNCT > ExprT;\
    return Expr< ExprT >( ExprT( ExprLiteral< ValueType >( x ), a ) );\
}\
template< class ValueType, class A > Expr < BinaryExprOp < Expr< A >, ExprLiteral< ValueType >, FUNCT > >\
    operator OP ( const Expr< A > & a, const ValueType & x ) {\
    typedef BinaryExprOp< Expr< A >, ExprLiteral< ValueType >, FUNCT > ExprT;\
    return Expr< ExprT >( ExprT( a, ExprLiteral< ValueType >( x ) ) );\
}\
template < class A, class B > Expr< BinaryExprOp< Expr< A >, Expr< B >, FUNCT > >\
    operator OP ( const Expr< A > & a, const Expr< B > & b ){\
    typedef BinaryExprOp< Expr< A >, Expr< B >, FUNCT > ExprT;\
    return Expr< ExprT >( ExprT( a, b ) );\
}\

DEFINE_EXPR_OPERATOR__( +, PLUS  )
DEFINE_EXPR_OPERATOR__( -, MINUS )
DEFINE_EXPR_OPERATOR__( *, MULT )
DEFINE_EXPR_OPERATOR__( /, DIVID )

#undef DEFINE_EXPR_OPERATOR__

} // namespace GIMLI

#endif