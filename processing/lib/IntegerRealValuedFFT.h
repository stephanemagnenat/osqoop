/*
 *     Program: REALFFT.C
 *      Author: Philip VanBaren
 *        Date: 2 September 1993
 *
 * Description: These routines perform an FFT on real data.
 *              On a 486/33 compiled using Borland C++ 3.1 with full
 *              speed optimization and a small memory model, a 1024 point 
 *              FFT takes about 16ms.
 *              This code is for integer data, but could be converted
 *              to float or double simply by changing the data types
 *              and getting rid of the bit-shifting necessary to prevent
 *              overflow/underflow in fixed-point calculations.
 *
 *  Note: Output is BIT-REVERSED! so you must use the BitReversed to
 *        get legible output, (i.e. Real_i = buffer[ BitReversed[i] ]
 *                                  Imag_i = buffer[ BitReversed[i]+1 ] )
 *        Input is in normal order.
 *
 *  Copyright (C) 1995  Philip VanBaren
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

/*
 *	Modified for C++ by Stephane Magnenat and Lucas Tamarit
 *  Stephane Magnenat <stephane at magnenat dot net>
 *  Lucas Tamarit <lucas dot tamarit at hesge dot ch>
 */

#ifndef __INTEGER_REAL_VALUED_FFT_H
#define __INTEGER_REAL_VALUED_FFT_H

#include <cmath>

//! Class for doing a FFT (Fast Fourrier Transform) using short int with a purely real valued source 
template<unsigned size>
class IntegerRealValuedFFT
{
private:
	short SinTable[size*2]; //!< table of sinus
	unsigned BitReversed[size]; //!< table of bit reverse lookup

public:
	//! Constructor, fill the tables
	IntegerRealValuedFFT()
	{
		// fill bit reversed lookup table
		for(unsigned i=0; i<size; i++)
		{
			unsigned temp = 0;
			for (unsigned mask = size/2; mask>0; mask >>= 1)
				temp = (temp >> 1) + (i&mask ? size : 0);

			BitReversed[i] = temp;
		}

		// fill sinus table
		for(unsigned i=0; i<size; i++)
		{
                        double s, c;
			s = floor(-32768.0*sin((M_PI*i)/(size))+0.5);
			c = floor(-32768.0*cos((M_PI*i)/(size))+0.5);
			if (s>32767.5)
				s=32767;
			if (c>32767.5)
				c=32767;
			SinTable[BitReversed[i]  ] = (short)s;
			SinTable[BitReversed[i]+1] = (short)c;
		}
	}

	//! Return the real component of a given index
	inline short re(const short buffer[size*2], unsigned i) const { return buffer[BitReversed[i]]; }
	//! Return the imaginary component of a given index
	inline short im(const short buffer[size*2], unsigned i) const { return buffer[BitReversed[i]+1]; }
	//! Return the module of a given index
	inline int module2(const short buffer[size*2], unsigned i) const
	{
		int reValue = (int)re(buffer, i);
		int imValue = (int)im(buffer, i);
		return reValue * reValue + imValue * imValue;
	}
	//! Return the size of the FFT
	inline unsigned sampleCount(void) const { return size; }

	/*!
		Do the fft

		Buffer of input containing:
		Re[0]
		0
		Re[1]
		0
		...

		Producing:
		Re*[0]
		Im*[0]
		Re*[1]
		Im*[1]

		To get the real index i, one must call the bitreversed lookup using
	*/
	void fft(short buffer[size*2])
	{
		int ButterfliesPerGroup = size/2;
		short *endptr1 = buffer + size * 2;

		/*
		*  Butterfly:
		*     Ain-----Aout
		*         \ /
		*         / \
		*     Bin-----Bout
		*/

		while (ButterfliesPerGroup>0)
		{
			short *A = buffer;
			short *B = buffer + ButterfliesPerGroup * 2;
			short *sptr = SinTable;

			while (A<endptr1)
			{
                                short sin = *sptr;
                                short cos = *(sptr+1);
				short *endptr2 = B;
				while (A<endptr2)
				{
					long v1 = ((long)*B*cos + (long)*(B+1)*sin) >> 15;
					long v2 = ((long)*B*sin - (long)*(B+1)*cos) >> 15;
					*B = (*A+v1) >> 1;
					*(A++) = *(B++) - v1;
					*B = (*A-v2) >> 1;
					*(A++) = *(B++) + v2;
				}
				A = B;
				B += ButterfliesPerGroup * 2;
				sptr += 2;
			}
			ButterfliesPerGroup >>= 1;
		}
		
		//  Massage output to get the output for a real input sequence.
		unsigned *br1 = BitReversed + 1;
		unsigned *br2 = BitReversed + size - 1;

		while (br1 <= br2)
		{
                        long temp1, temp2;
			short sin = SinTable[*br1];
			short cos = SinTable[*br1+1];
			short *A = buffer + *br1;
			short *B = buffer + *br2;
			long HRminus;
			long HIminus;
			long HRplus = (HRminus = *A     - *B    ) + (*B << 1);
			long HIplus = (HIminus = *(A+1) - *(B+1)) + (*(B+1) << 1);
			temp1  = ((long)sin*HRminus - (long)cos*HIplus) >> 15;
			temp2  = ((long)cos*HRminus + (long)sin*HIplus) >> 15;
			*B     = (*A     = (HRplus  + temp1) >> 1) - temp1;
			*(B+1) = (*(A+1) = (HIminus + temp2) >> 1) - HIminus;

			br1++;
			br2--;
		}

		// Handle DC bin separately
		buffer[0] += buffer[1];
		buffer[1] = 0;
	}
};

#endif
