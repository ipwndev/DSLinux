// Definitions and operators for fixed point math
// 
// (C) 1999 Nicolas Pitre <nico@cam.org>



#define FIXED_POINT_FRACBITS  24


#define FL2FIX( x, fracs )      ((int)((x)*(1<<fracs)))
#define FIX2FL( x, fracs )      ((float)(x)/(1<<fracs))



// The REAL class defines fixed point number objects with all associated
// operators.  The number of fractional bits is defined by the
// FIXED_POINT_FRACBITS value.

class REAL
{
  public:
    int x;
    //REAL( void ) {};
    //inline REAL( const int );
    //inline REAL( const float );
    inline REAL operator=(const REAL);
    inline REAL operator=(const int);
    inline REAL operator=(const float);
    inline REAL operator+(const REAL) const;
    inline REAL operator+(const float) const;
    inline REAL operator-(const REAL) const;
    inline REAL operator-(const float) const;
    inline REAL operator-(void) const;
    inline REAL operator+=(const REAL);
    inline REAL operator-=(const REAL);
    inline REAL operator*(const REAL) const;
    inline REAL operator*(const float) const;
    inline REAL operator*=(const REAL);
    inline REAL operator*=(const float);
    inline int operator==(const int) const;
    inline int operator!=(const int) const;
    inline REAL operator*(const class REAL13) const;
};

//inline REAL::REAL( const int i )
//{
//    x = i << FIXED_POINT_FRACBITS;
//}

//inline REAL::REAL( const float d )
//{
//    x = FL2FIX( d, FIXED_POINT_FRACBITS );
//}

inline REAL
REAL::operator=(const REAL r)
{
    x = r.x;
    return *this;
}

inline REAL
REAL::operator=(const int i)
{
    x = i << FIXED_POINT_FRACBITS;
    return *this;
}

inline REAL
REAL::operator=(const float d)
{
    x = FL2FIX(d, FIXED_POINT_FRACBITS);
    return *this;
}

inline REAL
REAL::operator+(const REAL r) const
{
    REAL s;
    do {
	s.x = x + r.x;
    } while (0);
    return s;
}

inline REAL
REAL::operator+(const float d) const
{
    REAL s;
    do {
	s.x = x + FL2FIX(d, FIXED_POINT_FRACBITS);
    } while (0);
    return s;
}

inline REAL
REAL::operator-(const REAL r) const
{
    REAL s;
    do {
	s.x = x - r.x;
    } while (0);
    return s;
}

inline REAL
REAL::operator-(const float d) const
{
    REAL s;
    do {
	s.x = x - FL2FIX(d, FIXED_POINT_FRACBITS);
    } while (0);
    return s;
}

inline REAL
REAL::operator-(void) const
{
    REAL s;
    do {
	s.x = -x;
    } while (0);
    return s;
}

inline REAL
REAL::operator+=(REAL r)
{
    do {
	x += r.x;
    } while (0);
    return *this;
}

inline REAL
REAL::operator-=(REAL r)
{
    do {
	x -= r.x;
    } while (0);
    return *this;
}

inline REAL
REAL::operator*(const REAL r) const
{
    REAL s;
    do {
	register long long l = (long long) x * (long long) r.x;
	s.x = (int) (l >> FIXED_POINT_FRACBITS);
    } while (0);
    return s;
}

inline REAL
REAL::operator*(const float d) const
{
    REAL s;
    do {
	long long l =
	    (long long) x * (long long) FL2FIX(d, FIXED_POINT_FRACBITS);
	s.x = (int) (l >> FIXED_POINT_FRACBITS);
    } while (0);
    return s;
}

inline REAL
REAL::operator*=(const REAL r)
{
    return (*this = *this * r);
}

inline REAL
REAL::operator*=(const float d)
{
    return (*this = *this * d);
}

inline int
REAL::operator==(const int i) const
{
    return (x == (i << FIXED_POINT_FRACBITS));
}

inline int
REAL::operator!=(const int i) const
{
    return (x != (i << FIXED_POINT_FRACBITS));
}



// The REAL13 class is mostly like the REAL class but with 13 fractional bits
// values.  It is required by the TO_FOUR_THIRDSTABLE array in order to fit
// its value range.

class REAL13
{
  public:
    int x;
    //inline REAL13( void ) {};
    //inline REAL13( const float );
    inline REAL13 operator=(const REAL13);
    inline REAL13 operator=(const float);
    inline REAL13 operator-(void) const;
};

//inline REAL13::REAL13( const float d )
//{
//    x = FL2FIX( d, 13 );
//}

inline REAL13
REAL13::operator=(const REAL13 r)
{
    x = r.x;
    return *this;
}

inline REAL13
REAL13::operator=(const float d)
{
    x = FL2FIX(d, 13);
    return *this;
}

inline REAL13
REAL13::operator-(void) const
{
    REAL13 s;
    do {
	s.x = -x;
    } while (0);
    return s;
}

inline REAL
REAL::operator*(const REAL13 r) const
{
    REAL s;
    do {
	long long l = (long long) x * (long long) r.x;
	s.x = (int) (l >> 13);
    } while (0);
    return s;
}



// The REAL19 class is mostly like the REAL class but with 19 fractional bits
// values.  It is required by the POW2 array in order to fit
// its value range.

class REAL19
{
  public:
    int x;
    //inline REAL19( void ) {};
    //inline REAL19( const float );
    inline REAL19 operator=(const float);
    inline REAL operator*(const REAL) const;
    inline REAL operator*(const REAL13) const;
};

//inline REAL19::REAL19( const float d )
//{
//    x = FL2FIX( d, 19 );
//}

inline REAL19
REAL19::operator=(const float d)
{
    x = FL2FIX(d, 19);
    return *this;
}

inline REAL
REAL19::operator*(const REAL r) const
{
    REAL s;
    do {
	long long l = (long long) x * (long long) r.x;
	s.x = (int) (l >> 19);
    } while (0);
    return s;
}

inline REAL
REAL19::operator*(const REAL13 r) const
{
    REAL s;
    do {
	long long l = (long long) x * (long long) r.x;
	s.x = (int) (l >> (19 + 13 - FIXED_POINT_FRACBITS));
    } while (0);
    return s;
}



// Misc global operators

inline REAL
operator*(const int i, const REAL r)
{
    REAL s;
    s.x = i * r.x;
    return s;
}
