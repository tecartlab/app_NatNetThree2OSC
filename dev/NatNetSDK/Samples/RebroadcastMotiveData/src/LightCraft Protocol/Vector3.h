/*
Copyright © 2016 NaturalPoint Inc.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License. */
#pragma once

// System includes
#include <string.h> //== for memcpy
#include <math.h>
#include <float.h>
#include <iostream>
#include <algorithm>

namespace Core
{
    template<typename T>
    class cVector3
    {
    public:
        cVector3() { } // No initialization

        template <typename U>
        cVector3( const cVector3<U>& other )
        {
            mVals[0] = T( other.X() );
            mVals[1] = T( other.Y() );
            mVals[2] = T( other.Z() );
        }
        cVector3( T x, T y, T z )
        {
            mVals[0] = x;
            mVals[1] = y;
            mVals[2] = z;
        }
        template <typename U>
        cVector3( U x, U y, U z )
        {
            mVals[0] = T( x );
            mVals[1] = T( y );
            mVals[2] = T( z );
        }
        cVector3( const T vals[3] )
        {
            ::memcpy( mVals, vals, 3 * sizeof( T ) );
        }

        /// <summary>Set the vector values.</summary>
        void            SetValues( T x, T y, T z ) { mVals[0] = x; mVals[1] = y; mVals[2] = z; }
        template <typename U>
        void            SetValues( U x, U y, U z ) { mVals[0] = T( x ); mVals[1] = T( y ); mVals[2] = T( z ); }
        void            SetValues( const T vals[3] ) { memcpy( mVals, vals, 3 * sizeof( T ) ); }

        T               X() const { return mVals[0]; }
        T&              X() { return mVals[0]; }
        T               Y() const { return mVals[1]; }
        T&              Y() { return mVals[1]; }
        T               Z() const { return mVals[2]; }
        T&              Z() { return mVals[2]; }

        T               operator[]( int idx ) const { return mVals[idx]; }
        T&              operator[]( int idx ) { return mVals[idx]; }

        /// <summary>Access to the data array.</summary>
        const float*    Data() const { return mVals; }

        /// <summary>Scale the vector by the given scalar.</summary>
        void            Scale( T scale )
        {
            mVals[0] *= scale;
            mVals[1] *= scale;
            mVals[2] *= scale;
        }

        /// <summary>Returns a scaled version of the vector.</summary>
        cVector3        Scaled( T scale ) const
        {
            cVector3   returnVal( *this );
            
            returnVal.Scale( scale );

            return returnVal;
        }

        /// <summary>Normalize the vector (in place) to unit length.</summary>
        void            Normalize()
        {
            T   len = Length();

            if( len > 0 )
            {
                Scale( 1 / len );
            }
        }

        /// <summary>Return a normalized version of the vector.</summary>
        cVector3        Normalized() const
        {
            cVector3   returnVal( *this );

            returnVal.Normalize();

            return returnVal;
        }

        /// <summary>Calculate the squared length of the vector.</summary>
        T               LengthSquared() const
        {
            return Dot( *this );
        }

        /// <summary>Calculate the length of the vector.</summary>
        T               Length() const
        {
            return sqrt( LengthSquared() );
        }

        /// <summary>Calculate the squared distance to the given point.</summary>
        T               DistanceSquared( const cVector3 &pnt ) const
        {
            T       x = pnt.mVals[0] - mVals[0];
            T       y = pnt.mVals[1] - mVals[1];
            T       z = pnt.mVals[2] - mVals[2];

            return ( x * x + y * y + z * z );
        }

        /// <summary>Calculate the distance to the given point.</summary>
        T               Distance( const cVector3 &pnt ) const
        {
            return sqrt( DistanceSquared( pnt ) );
        }

        /// <summary>Dot product of this with given vector.</summary>
        T               Dot( const cVector3 &other ) const
        {
            return ( mVals[0] * other.mVals[0] + mVals[1] * other.mVals[1] + mVals[2] * other.mVals[2] );
        }

        /// <summary>Returns this cross other.</summary>
        cVector3        Cross( const cVector3 &other ) const
        {
            T x = mVals[1] * other.mVals[2] - mVals[2] * other.mVals[1];
            T y = mVals[2] * other.mVals[0] - mVals[0] * other.mVals[2];
            T z = mVals[0] * other.mVals[1] - mVals[1] * other.mVals[0];

            return cVector3( x, y, z );
        }

        /// <summary>Returns the angle (in radians) between this and another vector.</summary>
        T               Angle( const cVector3 &other) const
        {
            //== angle between two vectors is defined by ==--

            //== dotProduct = a.x * b.x + a.y * b.y + a.z * b.z = a.len() * b.len * cos(angle)
            //== 
            //== thus: cos(angle) = dotProduct / (a.len * b.len) 
            //==
            //== therefore: angle = acos( dotProduct / (a.len * b*len ) )

            //== find (a.len * b.len)
            T len = Length() * other.Length();

            //== bound result ==--
            //== len = std::max<T>( std::numeric_limits<T>::min(), len);  Windows 'min' macro causes issues here

            //len = std::max(FLT_MIN, len);  //== ideally this is bounded by type's minimum as shown in the line above
            //ugh compilation issues with std::max

            if(len<FLT_MIN)
            {
                len = FLT_MIN;
            }
            
            //== find dotProduct / (a.len * b.len)
            T val = Dot(other) / len;

            //== bound result ==--
            val = std::min<T>(1, std::max<T>(-1, val));

            //== calculate angle (in radians) ==--

            return acos(val);
        }

        /// <summary>
        /// Returns a linear interpolated vector some percentage 't' between two vectors.
        /// The parameter t is usually in the range [0,1], but this method can also be used to extrapolate
        /// vectors beyond that range.
        /// </summary>
        inline static cVector3 Lerp( const cVector3 &v1, const cVector3 &v2, T t )
        {
            T k1 = 1 - t;
            T k2 = t;

            T x = k1 * v1.mVals[0] + k2 * v2.mVals[0];
            T y = k1 * v1.mVals[1] + k2 * v2.mVals[1];
            T z = k1 * v1.mVals[2] + k2 * v2.mVals[2];

            cVector3 result( x, y, z );
            
            return result;
        }

        void            SwitchHandedness()
        {
#if RIGHT_HANDED_INVERTED_X
            mVals[0] = -mVals[0];
#else
            mVals[2] = -mVals[2];
#endif
        }

        void            SwitchHandedness( const cVector3 &v )
        {
#if RIGHT_HANDED_INVERTED_X
            SetValues( -v.X(), v.Y(), v.Z() );
#else
            SetValues( v.X(), v.Y(), -v.Z() );
#endif
        }

        //====================================================================================
        // Mathematical and assignment operators
        //====================================================================================

        cVector3        operator+( const cVector3 &rhs ) const
        {
            return cVector3( mVals[0] + rhs.mVals[0], mVals[1] + rhs.mVals[1], mVals[2] + rhs.mVals[2] );
        }
        cVector3        operator+( T rhs ) const
        {
            return cVector3( mVals[0] + rhs, mVals[1] + rhs, mVals[2] + rhs );
        }
        cVector3&       operator+=( const cVector3 &rhs )
        {
            mVals[0] += rhs.mVals[0];
            mVals[1] += rhs.mVals[1];
            mVals[2] += rhs.mVals[2];

            return *this;
        }
        cVector3&       operator+=( T rhs )
        {
            mVals[0] += rhs;
            mVals[1] += rhs;
            mVals[2] += rhs;

            return *this;
        }

        cVector3        operator-( const cVector3 &rhs ) const
        {
            return cVector3( mVals[0] - rhs.mVals[0], mVals[1] - rhs.mVals[1], mVals[2] - rhs.mVals[2] );
        }
        cVector3        operator-( T rhs ) const
        {
            return cVector3( mVals[0] - rhs, mVals[1] - rhs, mVals[2] - rhs );
        }
        cVector3&       operator-=( const cVector3 &rhs )
        {
            mVals[0] -= rhs.mVals[0];
            mVals[1] -= rhs.mVals[1];
            mVals[2] -= rhs.mVals[2];

            return *this;
        }
        cVector3&       operator-=( T rhs )
        {
            mVals[0] -= rhs;
            mVals[1] -= rhs;
            mVals[2] -= rhs;

            return *this;
        }

        cVector3        operator*( T rhs ) const
        {
            return cVector3( mVals[0] * rhs, mVals[1] * rhs, mVals[2] * rhs );
        }
        cVector3&       operator*=( T rhs )
        {
            mVals[0] *= rhs;
            mVals[1] *= rhs;
            mVals[2] *= rhs;

            return *this;
        }

        cVector3        operator/( T rhs ) const
        {
            return cVector3( mVals[0] / rhs, mVals[1] / rhs, mVals[2] / rhs );
        }
        cVector3&       operator/=( T rhs )
        {
            mVals[0] /= rhs;
            mVals[1] /= rhs;
            mVals[2] /= rhs;

            return *this;
        }

        cVector3        operator-() const { return cVector3( -mVals[0], -mVals[1], -mVals[2] ); }

        template <typename U>
        cVector3&        operator=( const cVector3<U>& other )
        {
            mVals[0] = T( other.X() );
            mVals[1] = T( other.Y() );
            mVals[2] = T( other.Z() );

            return *this;
        }

        //====================================================================================
        // Type conversion helpers
        //====================================================================================

        inline Core::cVector3<float> ToFloat() const
        {
            return Core::cVector3<float>( (float) X(), (float) Y(), (float) Z() );
        }

        inline Core::cVector3<double> ToDouble() const
        {
            return Core::cVector3<double>( (double) X(), (double) Y(), (double) Z() );
        }

        //====================================================================================
        // Comparison operators
        //====================================================================================

        /// <summary>Useful for basic sorting operations. A "strictly less-than" comparison.</summary>
        bool            operator<( const cVector3 &rhs ) const
        {
            return ( mVals[0] < rhs.mVals[0] && mVals[1] < rhs.mVals[1] && mVals[2] < rhs.mVals[2] );
        }

        bool            operator==( const cVector3 &rhs ) const
        {
            return ( mVals[0] == rhs.mVals[0] && mVals[1] == rhs.mVals[1]
                && mVals[2] == rhs.mVals[2] );
        }

        bool            operator!=( const cVector3 &rhs ) const { return !( *this == rhs ); }

        /// <summary>Compare two vectors to within a tolerance.</summary>
        bool            Equals( const cVector3& rhs, T tolerance ) const
        {
            return ( fabs( mVals[0] - rhs.mVals[0] ) < tolerance && fabs( mVals[1] - rhs.mVals[1] ) < tolerance
                && fabs( mVals[2] - rhs.mVals[2] ) < tolerance );
        }

        //====================================================================================
        // Helper constants
        //====================================================================================
        
        static const cVector3 kZero;

    private:
        T               mVals[3];
    };

    //=========================================================================
    // Stream I/O operators
    //=========================================================================
    template< typename T>
    std::wostream& operator <<( std::wostream& os, const cVector3<T>& vec )
    {
        os << L"(" << vec[0] << L"," << vec[1] << L"," << vec[2] << L")";
        return os;
    }

    template< typename T>
    std::ostream& operator <<( std::ostream& os, const cVector3<T>& vec )
    {
        os << L"(" << vec[0] << L"," << vec[1] << L"," << vec[2] << L")";
        return os;
    }

    template <typename T>
    const cVector3<T> cVector3<T>::kZero( 0, 0, 0 );

    typedef cVector3<float> cVector3f;
    typedef cVector3<double> cVector3d;
}


