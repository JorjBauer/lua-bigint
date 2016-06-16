/*
 * BigInt - an arbitrarily large integer math class
 *
 * This class was originally used in DejaVu Software, Inc.'s PockeTTY
 * (an SSH and SSL terminal for Windows CE).
 *
 * This code is provided under the MIT license.
 *
 * Copyright (c) 2002-2007 DejaVu Software, Inc.
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 * 
 */

#include <string.h>
#include "BigInt.h"

#define USESQUAREMOD 0
#define USESQUARE 0

// Constructors & destructors

BigInt::BigInt()
{
	this->value = NULL;
	set_zero();
}

BigInt::BigInt(const BigInt& bi)
{
	this->value = NULL;
	copy_value(bi.value, bi.lsd+1, bi.negative);
}

BigInt::BigInt(unsigned char* value, long count)
{
	this->value = NULL;
	copy_bytes(value, count, false);	// default to positive
}

BigInt::BigInt(unsigned char* value, long count, bool negative)
{
	this->value = NULL;
	copy_bytes(value, count, negative);
}

BigInt::BigInt(long value)
{
	this->value = NULL;
	set_value(value);
}

BigInt::BigInt(unsigned long value)
{
	this->value = NULL;
	set_value(value);
}

BigInt::BigInt(const char* value)
{
	this->value = NULL;
	set_value(value);
}

BigInt::BigInt(const unsigned char* value)
{
	this->value = NULL;
	set_value(value);
}

BigInt::BigInt(long count, DIGIT value)
{
	this->value = new DIGIT[count];
	msd = lsd = count-1;
	memset(this->value, value, count*DIGITBYTES);
	negative = false;
}

#if !TWODIGITS_IS_UL
BigInt::BigInt(TWODIGITS digits)
{
	this->value = new DIGIT[2];
	msd = 0;
	lsd = 1;
	this->value[1] = (DIGIT) (digits) & DIGITMASK;
#if DIGITBITS == 32
	digits = 0;
#else
	digits >>= DIGITBITS;
#endif
	this->value[0] = (DIGIT) digits;
	negative = false;
}
#endif


BigInt::~BigInt()
{
	if (value)
	{
		delete[] value;
		value = NULL;
	}
}



// Assignment

bool BigInt::operator=(const BigInt& bi)
{
	return copy_value(bi.value, bi.lsd+1, bi.negative);
}

bool BigInt::operator=(const long value)
{
	return set_value(value);
}

bool BigInt::operator=(char* value)
{
	return set_value(value);
}

bool BigInt::copy_value(DIGIT* value, long count)
{
	return copy_value(value, count, false);	// default to positive
}

// NOTE: The contents of the value param are copied to this->value, i.e. 
// it is still the caller's responsibility to delete[] value.
//
// c.f. use_value
//
bool BigInt::copy_value(DIGIT* value, long count, bool negative)
{
	if (!value)  return false;
	if (!count)  return set_zero();

	if (this->value)
	{
		if (lsd+1 < count)
		{
			delete[] this->value;
			this->value = NULL;
		}
	}

	if (!this->value)
	{	
		this->value = new DIGIT[count];
		if (!this->value)  return false;
		lsd = count-1;
	}

	if (lsd+1 > count)
		memset(this->value, 0, (lsd+1-count)*DIGITBYTES);
	memcpy(&this->value[lsd+1-count], value, count*DIGITBYTES);
	for (msd=0; msd<lsd && this->value[msd]==0; msd++);
	this->negative = negative;
	return true;
}

bool BigInt::copy_bytes(const unsigned char* value, long count)
{
	return copy_bytes(value, count, false);	// default to positive
}

bool BigInt::copy_bytes(const unsigned char* value, long count, bool negative)
{
	if (!value)  return false;
	if (!count)  return set_zero();

	unsigned char slop = (unsigned char) (count % DIGITBYTES);
	long digits = count / DIGITBYTES + (slop ? 1 : 0);

	if (this->value)
	{
		if (lsd+1 < digits)
		{
			delete[] this->value;
			this->value = NULL;
		}
	}

	if (!this->value)
	{	
		this->value = new DIGIT[digits];
		if (!this->value)  return false;
		lsd = digits-1;
	}

	memset(this->value, 0, (lsd+1)*DIGITBYTES);
	long j = lsd+1-digits;
	unsigned char byte = slop ? slop : DIGITBYTES;
	for (long i = 0; i<count; i++)
	{
		if (!byte)
		{
			++j;
			byte = DIGITBYTES;
		}
		this->value[j] = (this->value[j] << 8) + value[i];
		--byte;
	}

	for (msd=lsd+1-digits; msd<lsd && this->value[msd]==0; msd++);
	this->negative = negative;
	return true;
}

bool BigInt::use_value(DIGIT* value, long count)
{
	return use_value(value, count, false);	// default to positive
}

// NOTE: The value param BECOMES this->value, i.e. it is now this BigInt's 
// responsibility to delete[] value.
//
// c.f. copy_value
//
bool BigInt::use_value(DIGIT* value, long count, bool negative)
{
	if (!value) return false;
	if (this->value)
		delete[] this->value;
	
	this->value = value;
	this->lsd = count-1;
	for (msd=0; msd<lsd && value[msd]==0; msd++);
	this->negative = negative;
	return true;
}

bool BigInt::set_value(long value)
{
	if (this->value)
	{
		delete[] this->value;
		this->value = NULL;
	}

	if (!value)
		return set_zero();
	
	// Get the sign
	//
	if (value < 0)
	{
		negative = true;
		value *= -1;
	}
	else
		negative = false;

	// TODO: We're assuming that DIGITBYTES divides 4 evenly

	lsd = 4 / DIGITBYTES - 1;
	msd = lsd;
	// Allocate and populate the digits
	//
	this->value = new DIGIT[lsd+1];
	if (!this->value)  return false;
	memset(this->value, 0, (lsd+1)*DIGITBYTES);
	for (long i=lsd; value; i--)
	{
		this->value[i] = (DIGIT)(value & DIGITMASK);
#if DIGITBITS == 32
		value = 0;
#else
		value >>= DIGITBITS;
#endif
		if (this->value[i])
			msd = i;
	}
	
	return true;
}

bool BigInt::set_value(unsigned long value)
{
	if (this->value)
	{
		delete[] this->value;
		this->value = NULL;
	}

	if (!value)
		return set_zero();
	
	negative = false;

	// TODO: We're assuming that DIGITBYTES divides 4 evenly

	lsd = 4 / DIGITBYTES - 1;
	this->msd = this->lsd;

	// Allocate and populate the digits
	//
	this->value = new DIGIT[lsd+1];
	if (!this->value)  return false;
	memset(this->value, 0, (lsd+1)*DIGITBYTES);
	for (long i=lsd; value; i--)
	{
		this->value[i] = (DIGIT)(value & DIGITMASK);
#if (DIGITBITS == 32)
		// Can't >> or << by 32 bits; the compiler drops the instruction
		value = 0;
#else
		value >>= DIGITBITS;
#endif
		if (this->value[i]) {
			this->msd = i;
		}
	}
	
	return true;
}

bool BigInt::set_value(const char* value)
{
	if (!strcmp(value, "0"))
		return set_zero();

	if (this->value)
	{
		delete[] this->value;
		this->value = NULL;
	}
	
	// TODO: check the format (what if it's invalid?)
	
	set_zero();

	// Get the sign
	//
	unsigned long i;
	if (value[0] == '-')
	{
		negative = true;
		i = 1;
	}
	else
	{
		negative = false;
		i = 0;
	}

	// Are we base 10 or base 16?
	if (value[i] == '0' && value[i+1] == 'x') {
		// Base 16.
		i += 2;
		for ( ; i < strlen(value); i++)
		{
			*this <<= 4;
			unsigned long v = 
				((value[i] >= '0' && value[i] <= '9') ? (value[i] - '0') :
				 (value[i] >= 'a' && value[i] <= 'f') ? (value[i] - 'a' + 10) :
				 (value[i] >= 'A' && value[i] <= 'F') ? (value[i] - 'A' + 10) :
				 0);
			add_digit(v);
		}

		return true;
	}
	
	// Get the magnitude
	//
	for ( ; i<strlen(value); i++)
	{
		*this *= (unsigned long)10;
		add_digit(value[i]-'0');
	}

	return true;
}

bool BigInt::set_value(const unsigned char* val)
{
	// A zero value is represented as four zero bytes for the length,
	// and nothing else.

	if (val[0]==0 && val[1]==0 && val[2]==0 && val[3]==0)
		return set_zero();

	long bytes = (val[0]<<24) + (val[1]<<16) + (val[2]<<8) + val[3];

	if (val[4] & 0x80)
	{
		unsigned char *bytecopy = new unsigned char[bytes];
		memcpy(bytecopy, &val[4], bytes);
		// TODO: This is ugly, but it should be OK
		complement_bytes(bytecopy, bytes);
		if (!copy_bytes(bytecopy, bytes, true)) {
			delete[] bytecopy;
			return false;
		}
		delete[] bytecopy;
		return true;
	}
	else
		return copy_bytes(&val[4], bytes, false);
}

bool BigInt::set_zero()
{
	if (!value)
	{
		value = new DIGIT[1];
		if (!value)  return false;
		lsd = 0;
	}
	
	memset(value, 0, (lsd+1)*DIGITBYTES);
	msd = lsd;
	negative = false;
	return true;
}

void BigInt::set_high_bit()
{
	value[msd] |= DIGITHIGHBIT;
}



// Addition

// This operator handles expressions of the form:
//
//	++BigInt
//
const BigInt& BigInt::operator++()
{
	add_digit(1);
	// TODO: what if this returns false?
	return *this;
}

// This operator handles expressions of the form:
//
//	BigInt++
//
const BigInt BigInt::operator++(int dummy)
{
	BigInt tmp = *this;
	add_digit(1);
	// TODO: what if this returns false?
	return tmp;
}

// This operator handles expressions of the form:
//
//	BigInt + (anything from which a BigInt can be constructed)
//
BigInt BigInt::operator+(const BigInt& bi) const
{
	BigInt result = *this;
	result += bi;
	return result;
}

// This operator handles expressions of the form:
//
//	long + BigInt
//
// NOTE: This function is not a method of this class, we're just friends
//
BigInt operator+(long value, const BigInt& bi)
{
	BigInt result = value;
	result += bi;
	return result;
}

// This operator handles expressions of the form:
//
//	BigInt += (anything from which a BigInt can be constructed)
//
bool BigInt::operator+=(const BigInt& bi)
{
	// If the signs match just add in the magnitude, since the sign 
	// doesn't change.
	// e.g. 3 + 5, 5 + 3, -3 + -5, -5 + -3
	//
	if (this->negative == bi.negative)
		return add_BigInt(bi);
	
	// The signs are different, so compare the magnitudes
	//
	switch (value_compare(bi))
	{
	case -1:
		// The magnitude is greater than this magnitude.  Set this 
		// magnitude as the given magnitude minus this one, and set
		// the sign as the given sign.
		// e.g. 3 + -5, -3 + 5
		//
		if (!subtract_from_BigInt(bi))
			return false;
		negative = bi.negative;
		return true;
		
	case 0:
		// The magnitudes are equal, so just set to zero.
		// e.g. 3 + -3, -3 + 3
		//
		return set_zero();
		
	case 1:
		// The magnitude is less than this magnitude, so just subtract
		// out the magnitude, since the sign doesn't change.
		// e.g. 5 + -3, -5 + 3
		//
		subtract_BigInt(bi);
		return true;
	}
	
	return false;	// never happens, just quieting the compiler
}

// Add in the given BigInt.
//
bool BigInt::add_BigInt(const BigInt& bi)
{
	// Allocate more space if we think we might need some
	//
	long r_bilsd = bi.lsd-bi.msd;

	if (r_bilsd >= lsd)
	{
		if (!extend(r_bilsd - lsd + 1))
			return false;
	}
	else if (value[0] != 0)
	{
		if (!extend(1))
			return false;
	}

	// Add digit-by-digit, until we run out of digits
	//
	DIGIT carry = 0;
	TWODIGITS tmp;
	long i, j;
	for (i=lsd, j=bi.lsd; j>=bi.msd; i--, j--)
	{
		tmp = (TWODIGITS)value[i] + (TWODIGITS)bi.value[j] + (TWODIGITS)carry;
		value[i] = (DIGIT)(tmp & DIGITMASK);
		carry = (DIGIT)(tmp >> DIGITBITS);
	}

	// Add the carry, if any
	//
	if (carry)
	{
		while (value[i] == DIGITMASK)
			value[i--] = 0;
		++value[i];
		msd = i;
	}
	else if (i < msd)
		msd = i+1;
	
	return true;
}

// Add digit to current value.
//
bool BigInt::add_digit(DIGIT digit)
{
	TWODIGITS sum = (TWODIGITS)value[lsd] + (TWODIGITS)digit;
	value[lsd] = (DIGIT)(sum & (TWODIGITS)DIGITMASK);
	if (sum >> DIGITBITS)
	{
		if (msd==0 && value[msd] == DIGITMASK) {
			if (!extend(1))
				return false;
		}

		long i;
		for (i=lsd-1; i>=0 && value[i]==DIGITMASK; i--)
			value[i] = 0;
		++value[i];
		if (!value[msd])
			--msd;
	}

	return true;
}



// Subtraction

// This operator handles expressions of the form:
//
//	--BigInt
//
const BigInt& BigInt::operator--()
{
	subtract_digit(1);
	// TODO: what if this returns false?
	return *this;
}

// This operator handles expressions of the form:
//
//	BigInt--
//
const BigInt BigInt::operator--(int dummy)
{
	BigInt tmp = *this;
	subtract_digit(1);
	// TODO: what if this returns false?
	return tmp;
}

// This operator handles expressions of the form:
//
//	BigInt - (anything from which a BigInt can be constructed)
//
BigInt BigInt::operator-(const BigInt& bi) const
{
	BigInt result = *this;
	result -= bi;
	return result;
}

// This operator handles expressions of the form:
//
//	long - BigInt
//
// NOTE: This function is not a method of this class, we're just friends
//
BigInt operator-(long value, const BigInt& bi)
{
	BigInt result = value;
	result -= bi;
	return result;
}

// This operator handles expressions of the form:
//
//	BigInt -= (anything from which a BigInt can be constructed)
//
bool BigInt::operator-=(const BigInt& bi)
{
	// If the signs differ just add in the magnitude, since the sign 
	// doesn't change.
	// e.g. 3 - -5, 5 - -3, -3 - 5, -5 - 3
	//
	if (this->negative != bi.negative)
		return add_BigInt(bi);
	
	// The signs are the same, so compare the magnitudes
	//
	switch (value_compare(bi))
	{
	case -1:
		// The magnitude is greater than this magnitude.  Set this 
		// magnitude as the given magnitude minus this one, and set
		// the sign as the negation of the given sign.
		// e.g. 3 - 5, -3 - -5
		//
		if (!subtract_from_BigInt(bi))
			return false;
		negative = !bi.negative;
		return true;
		
	case 0:
		// The magnitudes are equal, so just set to zero.
		// e.g. 3 - 3, -3 - -3
		//
		return set_zero();
		
	case 1:
		// The magnitude is less than this magnitude, so just subtract
		// out the magnitude, since the sign doesn't change.
		// e.g. 5 - 3, -5 - -3
		//
		subtract_BigInt(bi);
		return true;
	}
	
	return false;	// never happens, just quieting the compiler
}

// Subtract from the given BigInt.  We're assuming that its value is at
// least as big as ours.
//
bool BigInt::subtract_from_BigInt(const BigInt& bi)
{
	// Allocate a place to hold the result as we build it, and
	// initialize an index at the end.  Use the size of bi, 
	// since we won't need any more.
	//
	DIGIT* work = new DIGIT[bi.lsd+1];
	if (!work)  return false;
	long w = bi.lsd;
	
	// Subtract digit-by-digit, until we run out of digits.
	//
	SIGNEDDIGIT borrow = 0;
	SIGNEDTWODIGITS tmp;
	long i, j;
	for (i=lsd, j=bi.lsd; i>=msd; i--, j--)
	{
		tmp = (SIGNEDTWODIGITS)bi.value[j] - (SIGNEDTWODIGITS)value[i] + (SIGNEDTWODIGITS)borrow;
		work[w--] = (DIGIT)(tmp & DIGITMASK);
		borrow = (SIGNEDDIGIT)(tmp >> DIGITBITS);
	}

	// Subtract the borrow, if any
	//
	if (borrow)
	{
		for ( ; bi.value[j]==0; j--)
			work[w--] = DIGITMASK;
		work[w--] = bi.value[j] - 1;
	}
	
	// Just copy in any remaining digits
	//
	if (w>=0)
		memcpy(work, bi.value, (w+1)*DIGITBYTES);
	
	return use_value(work, bi.lsd+1, this->negative);
}

// Subtract out the given BigInt.  We're assuming that our value is at
// least as big as the BigInt.
//
void BigInt::subtract_BigInt(const BigInt& bi)
{
	// Subtract digit-by-digit, until we run out of digits.
	//
	SIGNEDDIGIT borrow = 0;
	SIGNEDTWODIGITS tmp;
	long i, j;
	for (i=lsd, j=bi.lsd; j>=bi.msd; i--, j--)
	{
		tmp = (SIGNEDTWODIGITS)value[i] - (SIGNEDTWODIGITS)bi.value[j] + (SIGNEDTWODIGITS)borrow;
		value[i] = (DIGIT)(tmp & DIGITMASK);
		borrow = (SIGNEDDIGIT)(tmp >> DIGITBITS);
	}

	// Subtract the borrow, if any
	//
	if (borrow)
	{
		while (value[i] == 0)
			value[i--] = DIGITMASK;
		--value[i];
		if (value[i] == 0)
			++msd;
	}
	else
		for (; msd<lsd && value[msd]==0; msd++);
}

// Subtract out the given digit.  We're assuming that our value is at
// least as big as the digit.
//
void BigInt::subtract_digit(DIGIT digit)
{
	SIGNEDTWODIGITS diff = value[lsd] - digit;
	value[lsd] = (DIGIT)(diff & (TWODIGITS)DIGITMASK);

	if (diff >> DIGITBITS)
	{
		long i;
		for (i=lsd-1; i>=0 && value[i]==0; i--)
			value[i] = DIGITMASK;
		--value[i];
		if (!value[msd])
			++msd;
	}
}



// Multiplication

// This operator handles expressions of the form:
//
//	BigInt * (anything from which a BigInt can be constructed)
//
BigInt BigInt::operator*(const BigInt& bi) const
{
	BigInt result = *this;
	result *= bi;
	return result;
}

// This operator handles expressions of the form:
//
//	long * BigInt
//
// NOTE: This function is not a method of this class, we're just friends
//
BigInt operator*(long value, const BigInt& bi)
{
	BigInt result = value;
	result *= bi;
	return result;
}

// This operator handles expressions of the form:
//
//	BigInt *= (anything from which a BigInt can be constructed)
//
bool BigInt::operator*=(const BigInt& bi)
{
	// Handle special cases first
	//
	if (msd == lsd)
	{
		if (value[lsd] == 0) // zero()
			return true;
		else if (value[lsd] == 1)
		{
			if (negative) // negative_one()
				return copy_value(bi.value, bi.lsd+1, !bi.negative);
			else // one()
				return copy_value(bi.value, bi.lsd+1, bi.negative);
		}
	}
	if (bi.msd == bi.lsd)
	{
		if (bi.value[bi.lsd] == 0) // bi.zero()
			return set_zero();
		else if (bi.value[bi.lsd] == 1)
		{
			if (bi.negative) // bi.negative_one()
			{
				negative = !negative;
				return true;
			}
			else // bi.one()
				return true;
		}
	}

	// Allocate space to hold the result value as we build it
	//
	long result_lsd = lsd-msd + bi.lsd-bi.msd + 1;
	DIGIT* result = new DIGIT[result_lsd+1];
	if (!result)  return false;
	memset(result, 0, (result_lsd+1)*DIGITBYTES);

	// Do the multiply
	//
	long i, j, k;
	TWODIGITS tmp;
	for (i=bi.lsd; i>=bi.msd; i--)
	{
		if (!bi.value[i])  continue;

		tmp = 0;
		for (j=lsd, k=result_lsd-(bi.lsd-i); j>=msd; j--, k--)
		{
			tmp += (TWODIGITS) ((TWODIGITS)value[j] * (TWODIGITS)bi.value[i]) + (TWODIGITS)result[k];
			result[k] = (DIGIT)(tmp & (TWODIGITS)DIGITMASK);
			tmp >>= DIGITBITS;
		}
		result[k] += (DIGIT)tmp;
	}

	// Use the result value, with the sign determined from this sign 
	// and the sign of bi.
	//
	return use_value(result, result_lsd+1, (this->negative != bi.negative));
}

bool BigInt::square()
{
	// Allocate space to hold the result as we build it
	//
	long result_lsd = 2*(lsd-msd) + 1;
	DIGIT* result = new DIGIT[result_lsd+1];
	if (!result)  return false;
	memset(result, 0, (result_lsd+1)*DIGITBYTES);

	// Do the squaring, starting from the lsd.
	//
	long i, j, k;
	TWODIGITS tmp;
	for (i=lsd; i>=msd; i--)
	{
		if (!value[i])  continue;

		// Square the current digit and add in the result
		k = 2*(i-msd) + 1;
		tmp = (TWODIGITS)value[i] * (TWODIGITS)value[i] + (TWODIGITS)result[k];
		result[k] = (DIGIT)(tmp & DIGITMASK);
		tmp >>= DIGITBITS;

		// Multiply the rest by the current digit, times two, and
		// add in the result
		for (j=i-1, --k; j>=msd; j--, k--)
		{
			TWODIGITS tmp2 = (TWODIGITS)value[j] * (TWODIGITS)value[i];
			TWODIGITS hibit = tmp2 & (TWODIGITS)((TWODIGITS)DIGITHIGHBIT<<DIGITBITS);
			tmp += (tmp2*2) + (TWODIGITS)result[k];
			result[k] = (DIGIT)(tmp & DIGITMASK);
			tmp >>= DIGITBITS;
			tmp |= hibit >> (DIGITBITS-1);
		}

		// Add in any remaining carry
		while (tmp)
		{
			tmp += result[k];
			result[k--] = (DIGIT)(tmp & DIGITMASK);
			tmp >>= DIGITBITS;
		}
	}

	// Use the result value, and the sign must be positive.
	//
	return use_value(result, result_lsd+1, false);
}

bool BigInt::squaremod(const BigInt& modulator)
{
	long mid = (lsd-msd+1)/2;

	BigInt a, b;
	a.copy_value(&value[msd], mid);
	a *= a;

	for (int i = mid; i+msd <= lsd; i++)
	{
		a <<= 2*DIGITBITS;
		a %= modulator;
		b.copy_value(&value[msd], i);
		b <<= DIGITBITS;
#if TWODIGITS_IS_UL
		b *= (unsigned long)(value[i+msd] * 2);
#else
		b *= (TWODIGITS)(value[i+msd] * 2);
#endif

#if TWODIGITS_IS_UL
		a += b + (unsigned long)(value[i+msd]*value[i+msd]);
#else
		a += b + (TWODIGITS)(value[i+msd]*value[i+msd]);
#endif
		a %= modulator;
	}
	return copy_value(a.value, a.lsd+1);
}

bool BigInt::negate()
{
	this->negative = !this->negative;
	return true;
}

// Division

// This operator handles expressions of the form:
//
//	BigInt / (anything from which a BigInt can be constructed)
//
BigInt BigInt::operator/(const BigInt& bi) const
{
	BigInt result = *this;
	result /= bi;
	return result;
}

// This operator handles expressions of the form:
//
//	long / BigInt
//
// NOTE: This function is not a method of this class, we're just friends
//
BigInt operator/(long value, const BigInt& bi)
{
	BigInt result = value;
	result /= bi;
	return result;
}

// This operator handles expressions of the form:
//
//	BigInt /= (anything from which a BigInt can be constructed)
//
bool BigInt::operator/=(const BigInt& bi)
{
	// If dividing by zero, return false
	// TODO: do more?
	//
	if (bi.zero())
		return false;
	
	// If this is zero, do nothing
	//
	if (zero())
		return true;
	
	// If bi is one, do nothing
	//
	if (bi.one())
		return true;
	
	// If bi is negative one, negate and return
	//
	if (bi.negative_one())
	{
		negative = !negative;
		return true;
	}
	
	// If the value is less than the divisor's, set to zero
	// TODO: If this and bi are different signs, should this be 
	// negative zero?
	//
	if (value_compare(bi) == -1)
		return set_zero();

	BigInt quot;
	BigInt mod;
	long i;
	DIGIT j;
	for (i=msd; i<=lsd; i++)
		for (j=DIGITHIGHBIT; j; j>>=1)
		{
			quot.shift_left_one();
			mod.shift_left_one();
			if (value[i] & j)
				mod.value[mod.lsd] |= 1;
			if (mod.value_compare(bi) != -1)
			{
				mod.subtract_BigInt(bi);
				++quot;
			}
		}

	// Whatever we're left with in quot is the quotient.  The sign is 
	// determined from this sign and the sign of bi.
	//
	return copy_value(quot.value, quot.lsd+1, (this->negative != bi.negative));
}



// Modulation

// This operator handles expressions of the form:
//
//	BigInt % (anything from which a BigInt can be constructed)
//
BigInt BigInt::operator%(const BigInt& bi) const
{
	BigInt result = *this;
	result %= bi;
	return result;
}

// This operator handles expressions of the form:
//
//	long % BigInt
//
// NOTE: This function is not a method of this class, we're just friends
//
BigInt operator%(long value, const BigInt& bi)
{
	BigInt result = value;
	result %= bi;
	return result;
}

// This operator handles expressions of the form:
//
//	BigInt %= (anything from which a BigInt can be constructed)
//
// NOTE: When one or both of the operands is negative, there are two equally
// valid ways of doing the modulation, described on these web pages:
//
//	http://www.math.grin.edu/~stone/scheme-web/remainder.html
//	http://www.math.grin.edu/~stone/scheme-web/modulo.html
//
// We're implementing the "modulo" version here, because that's what the 
// thing we're replacing used.
//
bool BigInt::operator%=(const BigInt& bi)
{
	// If dividing by zero, return false
	// TODO: do more?
	//
	if (bi.zero())
		return false;
	
	// If we're equal to the modulator, set to zero.  We don't care 
	// about the signs, zero is zero.  If we're less than the modulator
	// do nothing, otherwise set the value to the modulation.
	//
	switch (value_compare(bi))
	{
	case 0:
		return set_zero();
	case -1:
		break;
	case 1:
		BigInt mod(bi.lsd-bi.msd+2, (DIGIT)0);
		long i;
		DIGIT j;
		for (i=msd; i<=lsd; i++)
		{
			for (j=DIGITHIGHBIT; j; j>>=1)
			{
				mod.shift_left_one();
				if (value[i] & j)
					mod.value[mod.lsd] |= 1;
				if (mod.value_compare(bi) != -1)
					mod.subtract_BigInt(bi);
			}
		}
		if (!copy_value(mod.value, mod.lsd+1, this->negative))
			return false;
		break;
	}
	
	// If the signs are different, the value of the modulation is the 
	// modulator minus the value we just calculated.
	//
	if (this->negative != bi.negative)
	{
		if (!subtract_from_BigInt(bi))
			return false;
	}
	
	// The sign of the result should be the sign of the modulator.
	//
	this->negative = bi.negative;
	
	return true;
}


bool BigInt::multmod(const BigInt& mult, const BigInt& mod)
{
	BigInt x = *this % mod;
	BigInt y = mult % mod;
	set_zero();
	long i;
	DIGIT j;
	for (i=y.msd; i<=y.lsd; i++)
	{
		for (j=DIGITHIGHBIT; j; j>>=1)
		{
			shift_left_one();
			if (y.value[i] & j)
				add_BigInt(x);
			*this %= mod;
		}
	}
	return true;
}



// Exponentiation

// This function returns this BigInt raised to the power of the given 
// BigInt.
//
BigInt BigInt::exp(const BigInt& bi)
{
	// TODO: handle negative exponents?
	if (bi.negative)
	{
		BigInt result;	// zero
		return result;
	}
	
	// If the exponent is zero, return one
	//
	if (bi.zero())
	{
		BigInt result = (unsigned long)1;
		return result;
	}
	
	// If the exponent is one, return this
	//
	if (bi.one())
	{
		BigInt result = *this;
		return result;
	}

	// Make a copy which we can manipulate, and initialize the 
	// result value to one.
	//
	BigInt me = *this;
	BigInt result = (unsigned long)1;

	// Do all of the bits in all of the digits except the msd.
	//
	long i;
	DIGIT j;
	for (i=bi.lsd; i>bi.msd; i--)
	{
		for (j=1; j; j<<=1)
		{
			if (bi.value[i] & j)
				result *= me;

#if USESQUARE
			me.square();
#else
			me *= me;
#endif
		}
	}

	// Now do the remaining bits in the msd.
	//
	j = bi.value[bi.msd];
	while (j != 0)
	{
		while (!(j & 1))
		{
			j >>= 1;
#if USESQUARE
			me.square();
#else
			me *= me;
#endif
		}
		
		--j;
		result *= me;
	}
	
	return result;
}

// This function returns this BigInt raised to the power of exponent, then 
// modulated by modulator.
//
BigInt BigInt::expmod(const BigInt& exponent, const BigInt& modulator) const
{
	// TODO: handle negative exponents?
	if (exponent.negative)
	{
		BigInt result;	// zero
		return result;
	}
	
	// Make a copy which we can manipulate, and initialize the 
	// result value to one.
	//
	BigInt me = *this;
	BigInt result = (unsigned long)1;

	// Do all of the bits in all of the digits except the msd.
	//
	long i;
	DIGIT j;
	for (i=exponent.lsd; i>exponent.msd; i--)
	{
		for (j=1; j; j<<=1)
		{
			if (exponent.value[i] & j)
			{
				result *= me;
				result %= modulator;
			}
#if USESQUAREMOD
			me.squaremod(modulator);
#else
#if USESQUARE
			me.square();
#else
			me *= me;
#endif
			me %= modulator;
#endif
		}
	}

	// Now do the remaining bits in the msd.
	//
	j = exponent.value[exponent.msd];
	while (j != 0)
	{
		while (!(j & 1))
		{
			j >>= 1;
#if USESQUAREMOD
			me.squaremod(modulator);
#else
#if USESQUARE
			me.square();
#else
			me *= me;
#endif
			me %= modulator;
#endif
		}
		
		--j;
		result *= me;
		result %= modulator;
	}
	
	return result;
}


BigInt& BigInt::get_partial(BigInt** partials, long pindex, const BigInt& modulator) const
{
	if (!partials[pindex])
	{
		partials[pindex] = new BigInt(*this);
		*partials[pindex] %= modulator;

		if (pindex > 1)
		{
			// look for a pair of partials which make up this partial

			int k;
			for (k=1; k<=pindex>>1; k += 2)
			{
				if (partials[k] && partials[pindex-1-k])
				{
					*partials[pindex] *= *partials[k];
					*partials[pindex] %= modulator;
					*partials[pindex] *= *partials[pindex-1-k];
					*partials[pindex] %= modulator;
					break;
				}
			}

			if (k > pindex>>1) // never found a precalculated pair
				*partials[pindex] = partials[pindex]->expmod(pindex, modulator);
		}
	}

	return *partials[pindex];
}


/* Find the inverse of "x mod n". Normal inverses are the number by which you
   multiply the original number such that the answer is 1. Modular inverses are
   similar: the modular inverse of "x mod n" is an integer (y) such that
   "(x*y) mod n = 1".

   This is found by using the extended Euclidean algorithm. This code could
   be cleaned up a bit...
  
   The basic procedure for finding the inverse of "x mod n" is to repeatedly
   solve this equation:
	
   n = q (x) + r
	  
   After each iteration, repeat with n = x and x = r, until finally r == 0.
   If the step previous to the solution shows that r == 1, then there is an
   inverse, and its inverse is the next calculation of P.
		
   For each iteration of the n=q(x)+r equation, a new value of P is calculated.
   For the first iteration, P == 0. The second, P == 1. For all other steps,
		  
       P_i = P_(i-2) - P_(i-1) * q_(i-2)
			
   So, in this implementation, we keep track of the last three Ps and Qs.
			  
   -- Jorj Bauer <jorj@dejavusoftware.com> 5/26/00
*/
BigInt BigInt::inv(const BigInt& modulator) const
{
	unsigned long step;
	
	step = 0;
	
	BigInt bi_a, bi_b, bi_ret, bi_remainder, bi_prevremainder, bi_res;
	BigInt bi_p[3];
	BigInt bi_q[3];
	bi_a = modulator;
	bi_b = *this;
	
	do {
		bi_prevremainder = bi_remainder;
		bi_res = bi_a / bi_b;
		bi_remainder = bi_a - (bi_res * bi_b);
		
		if (step == 0) {
			bi_p[0] = (const char *)"0";
			bi_q[0] = bi_res;
		}
		else if (step == 1) {
			bi_p[1] = 1;
			bi_q[1] = bi_res;
		}
		else if (step == 2) {
			bi_q[2] = bi_res;
			bi_p[2] = (bi_p[0] - (bi_p[1] * bi_q[0])) % modulator;
		}
		else {
			bi_p[0] = bi_p[1];
			bi_p[1] = bi_p[2];
			bi_q[0] = bi_q[1];
			bi_q[1] = bi_q[2];
			bi_q[2] = bi_res;
			
			bi_p[2] = (bi_p[0] - (bi_p[1] * bi_q[0])) % modulator;
		}
		bi_a = bi_b;
		bi_b = bi_remainder;
		step++;
	} while (!bi_remainder.zero());	// while (floor(remainder) != 0);
	
	if (!bi_prevremainder.one()) { //  if (floor(prevremainder) != 1) {
		//    printf("Previous remainder is not 1 (%s)!\n", big_string(&b_prevremainder, 10));
		//    exit(-1); // no solution; error.
	}
	
	if (step > 2) {
		bi_ret = (bi_p[1] - (bi_p[2] * bi_q[1])) % modulator;
	} else if (step==2){
		bi_ret = (bi_p[0] - (bi_p[1] * bi_q[0])) % modulator;
	} else {
		//    printf("Error?\n");
		bi_ret = (const char *)"0";
	}
	
	return (bi_ret); // return (floor(ret));
}



// GCD

// This function returns the greatest common divisor of this BigInt and
// the given BigInt.
//
BigInt BigInt::gcd(const BigInt& bi) const
{
	// If the given value is zero, return the absolute value of this
	//
	if (bi.zero())
	{
		BigInt result = *this;
		result.negative = false;
		return result;
	}

	// Make a copy of each which we can manipulate.
	//
	BigInt val1 = *this;
	BigInt val2 = bi;

	// Find GCD by repeated modulation
	//
	while (!val2.zero())
	{
		val1 %= val2;
		if (val1.zero())
			break;
		val2 %= val1;
	}

	if (val1.zero())
		return val2;
	else
		return val1;
}



// Comparison

// This operator handles expressions of the form:
//
//	long == BigInt
//
// NOTE: This function is not a method of this class, we're just friends
//
bool operator==(long value, const BigInt& bi)
{
	return bi == value;
}

// This operator handles expressions of the form:
//
//	BigInt == (anything from which a BigInt can be constructed)
//
bool BigInt::operator==(const BigInt& bi) const
{
	// If different signs, can't be equal
	//
	if (negative != bi.negative)
		return false;
	
	// Compare the values
	//
	return value_compare(bi) == 0;
}

// This operator handles expressions of the form:
//
//	long != BigInt
//
// NOTE: This function is not a method of this class, we're just friends
//
bool operator!=(long value, const BigInt& bi)
{
	return !(value == bi);
}

// This operator handles expressions of the form:
//
//	BigInt != (anything from which a BigInt can be constructed)
//
bool BigInt::operator!=(const BigInt& bi) const
{
	return !(*this == bi);
}

// This operator handles expressions of the form:
//
//	long < BigInt
//
// NOTE: This function is not a method of this class, we're just friends
//
bool operator<(long value, const BigInt& bi)
{
	return bi > value;
}

// This operator handles expressions of the form:
//
//	BigInt < (anything from which a BigInt can be constructed)
//
bool BigInt::operator<(const BigInt& bi) const
{
	// If negative and bi isn't, must be less than
	//
	if (negative && !bi.negative)
		return true;
	
	// If positive and bi isn't, must not be less than
	//
	if (!negative && bi.negative)
		return false;
	
	// They're the same sign, so we're less if our value is less and 
	// we're (both) positive, or if our value is more and we're (both)
	// negative.
	
	switch (value_compare(bi))
	{
	case -1:	return !negative;
	case 0:		return false;	// equal, so not less than
	case 1:		return negative;
	}
	
	return false;	// never happens, just quieting the compiler
}

// This operator handles expressions of the form:
//
//	long <= BigInt
//
// NOTE: This function is not a method of this class, we're just friends
//
bool operator<=(long value, const BigInt& bi)
{
	return bi >= value;
}

// This operator handles expressions of the form:
//
//	BigInt <= (anything from which a BigInt can be constructed)
//
bool BigInt::operator<=(const BigInt& bi) const
{
	// If negative and bi isn't, must be less than (or equal)
	//
	if (negative && !bi.negative)
		return true;
	
	// If positive and bi isn't, must not be less than (or equal)
	//
	if (!negative && bi.negative)
		return false;
	
	// They're the same sign, so we're less if our value is less and 
	// we're (both) positive, or if our value is more and we're (both)
	// negative.
	
	switch (value_compare(bi))
	{
	case -1:	return !negative;
	case 0:		return true;	// equal, so less than or equal
	case 1:		return negative;
	}
	
	return false;	// never happens, just quieting the compiler
}

// This operator handles expressions of the form:
//
//	long > BigInt
//
// NOTE: This function is not a method of this class, we're just friends
//
bool operator>(long value, const BigInt& bi)
{
	return bi < value;
}

// This operator handles expressions of the form:
//
//	BigInt > (anything from which a BigInt can be constructed)
//
bool BigInt::operator>(const BigInt& bi) const
{
	return !(*this <= bi);
}

// This operator handles expressions of the form:
//
//	long >= BigInt
//
// NOTE: This function is not a method of this class, we're just friends
//
bool operator>=(long value, const BigInt& bi)
{
	return bi <= value;
}

// This operator handles expressions of the form:
//
//	BigInt >= (anything from which a BigInt can be constructed)
//
bool BigInt::operator>=(const BigInt& bi) const
{
	return !(*this < bi);
}

// This function returns true if this BigInt is greater than zero.
//
bool BigInt::is_positive() const
{
	return (!zero() && !negative);
}

// This function returns true if this BigInt is less than zero.
//
bool BigInt::is_negative() const
{
	return (!zero() && negative);
}

// This function returns true if this BigInt is equal to zero.
// Note: We're not distinguishing between positive and negative zero.
// There are places in the code that rely on this fact.
//
bool BigInt::zero() const
{
	return (msd==lsd && value[lsd]==0);
}

// This function returns true if this BigInt is equal to one.
//
bool BigInt::one() const
{
	return (msd==lsd && value[lsd]==1 && !negative);
}

// This function returns true if this BigInt is equal to negative one.
//
bool BigInt::negative_one() const
{
	return (msd==lsd && value[lsd]==1 && negative);
}

// This function returns true if this BigInt is even.
//
bool BigInt::even() const
{
	return !(value[lsd] & 1);	// Only need to check the lsd
}

// This function returns true if this BigInt is odd.
//
bool BigInt::odd() const
{
	return (value[lsd] & 1);	// Only need to check the lsd
}

// Return -1 if this value is less than the given value, 0 if equal, and 
// 1 if greater, regardless of sign.
//
int BigInt::value_compare(const BigInt& bi) const
{
	// If shorter, must be less
	//
	if (lsd-msd < bi.lsd-bi.msd)
		return -1;
	
	// If longer, must be greater
	//
	if (lsd-msd > bi.lsd-bi.msd)
		return 1;
	
	// Compare digit-by-digit, starting with msd
	//
	long i, j;
	for (i=msd, j=bi.msd; i<=lsd; i++, j++)
		if (value[i] < bi.value[j])
			return -1;
		else if (value[i] > bi.value[j])
			return 1;
		
	// No differences, so they're equal
	//
	return 0;
}



// Shifting

// This operator handles expressions of the form:
//
//	BigInt <<= long
//
bool BigInt::operator<<=(long howmany)
{
	// If only shifting one, use faster algorithm
	//
	if (howmany == 1)
		return shift_left_one();

	// If zero, no amount of shifting will change that
	//
	if (zero())
		return true;
	
	// If shifting zero, we're done
	//
	if (howmany == 0)
		return true;

	// If shifting negative, shift the other way
	//
	if (howmany < 0)
		return *this >>= -howmany;

	long digits = howmany / DIGITBITS;
	unsigned char bits = (unsigned char) (howmany % DIGITBITS);

	unsigned char bitsc = DIGITBITS - bits;
	DIGIT himask = DIGITMASK << bitsc;

	// Extend value array
	//
	extend(digits + (value[msd]&himask ? 1 : 0) - msd);

	if (digits)
	{
		// Copy the digits left, and zero out the empties
		long i;
		for (i=msd; i<=lsd; i++)
			value[i-digits] = value[i];
		for (i=lsd; i>lsd-digits; i--)
			value[i] = 0;
		msd -= digits;
	}

	if (bits)
	{
		// Do the shifting
		for (long i=(msd>0 ? msd-1 : 0); i<=lsd-digits; i++)
		{
			value[i] <<= bits;

			// Copy the top bits from the digit to the right into the bottom
			// of this digit
			if (i < lsd)
				value[i] |= (value[i+1] & himask) >> bitsc;
		}
		if (msd>0 && value[msd-1])
			--msd;
	}

	return true;
}

// Special case of <<=, shifts one bit.
//
bool BigInt::shift_left_one()
{
	// If zero, no amount of shifting will change that
	//
	if (zero())
		return true;
	
	// Set msd and extend value array if necessary
	//
	if (value[msd] & DIGITHIGHBIT)
	{
		if (msd == 0)
			if (!extend(2))
				return false;
		--msd;
	}

	// Do the shifting
	//
	for (long i=msd; i<lsd; i++)
		value[i] = (value[i] << 1) | (value[i+1] >> (DIGITBITS-1));
	value[lsd] <<= 1;

	return true;
}

// This operator handles expressions of the form:
//
//	BigInt >>= long
//
bool BigInt::operator>>=(long howmany)
{
	// If zero, no amount of shifting will change that
	//
	if (zero())
		return true;
	
	// If shifting zero, we're done
	//
	if (howmany == 0)
		return true;

	// If shifting negative, shift the other way
	//
	if (howmany < 0)
		return *this <<= -howmany;

	long digits = howmany / DIGITBITS;
	unsigned char bits = (unsigned char) (howmany % DIGITBITS);

	if (digits)
	{
		// If we're shifting all the digits we have, we're left with zero
		if (digits > lsd-msd)
			return set_zero();

		// Copy the digits right, and zero out the empties
		long i;
		for (i=lsd; i>=digits+msd; i--)
			value[i] = value[i-digits];
		for (i=msd; i<digits+msd; i++)
			value[i] = 0;
		msd += digits;
	}

	if (bits)
	{
		unsigned char bitsc = DIGITBITS - bits;
		DIGIT lomask = DIGITMASK >> bitsc;

		// Do the shifting
		for (long i=lsd; i>=msd; i--)
		{
			value[i] >>= bits;

			// Copy the bottom bits from the digit to the left into the top
			// of this digit
			if (i > msd)
				value[i] |= (value[i-1] & lomask) << bitsc;
		}
		if (msd<lsd && !value[msd])
			++msd;
	}

	// If now zero, make sure negative is not set
	//
	if (zero())
		negative = false;

	return true;
}



// Output

// Returns the number of bytes needed by the value.
//
long BigInt::byte_length() const
{
	long length = (lsd+1-msd)*DIGITBYTES;
	for (DIGIT i=0xFF<<(8*(DIGITBYTES-1)); i && !(value[msd]&i) && length>1; i>>=8)
		--length;
	return length;
}

// Returns a newly-allocated byte array containing the value.
//
unsigned char* BigInt::byte_array_value() const
{
	return byte_array_value(byte_length());
}

// Stores the value in the given byte array.
//
void BigInt::byte_array_value(unsigned char* result) const
{
	byte_array_value(result, byte_length());
}

// Returns a newly-allocated byte array of the given length containing the 
// value.  If the length is not long enough to hold the value, returns 
// NULL.  The msb will be in array position length - byte_length().
//
unsigned char* BigInt::byte_array_value(long length) const
{
	if (length < byte_length()) return NULL;
	unsigned char* result = new unsigned char[length];
	byte_array_value(result, length);
	return result;
}

// Stores the value in the first 'length' bytes of the given byte array.  
// The msb will be in position length - byte_length().  It is assumed that 
// length is enough to hold the value (i.e. that it is at least 
// byte_length()).  If the value is negative, the array contains the two's 
// complement.
//
void BigInt::byte_array_value(unsigned char* result, long length) const
{
	memset(result, 0, length);
	DIGIT digit = 0;
	unsigned char byte = 0;
	long j = lsd;
	for (long i = length; i && (byte || j>=msd); i--)
	{
		if (!byte)
		{
			digit = value[j--];
			byte = DIGITBYTES;
		}
		result[i-1] = (unsigned char)(digit & (DIGIT)0xFF);
		digit >>= 8;
		--byte;
	}

	if (negative)
		complement_bytes(result, length);
}

long BigInt::long_value() const
{
#if DIGITBITS == 32
	return negative ? -(long)value[lsd] : value[lsd];
#else
	long result = 0;
	
	for (long i=msd; i<=lsd; i++)
	{
		result <<= DIGITBITS;
		result += value[i];
	}

	return negative ? -result : result;
#endif
}

unsigned long BigInt::ul_value() const
{
#if DIGITBITS == 32
	return value[lsd];
#else	
	unsigned long result = 0;

	for (long i=msd; i<=lsd; i++)
	{
		result <<= DIGITBITS;
		result += value[i];
	}

	return result;
#endif
}

char* BigInt::decimal_string_value() const
{
	// If this is zero, just do something simple.  Note we shouldn't 
	// just return "0", since we expect the caller to delete[].
	//
	if (zero())
	{
		char* result = new char[2];
		result[0] = '0';
		result[1] = 0;
		return result;
	}
	
	// TODO: this is horribly inefficient!
	// Allocate more than enough characters, plus one for a minus sign if needed
	//
	char* result = new char[(lsd+1-msd)*(DIGITBITS/2)+1];
	long sofar = 0;
	//
	BigInt tmp = *this;
	tmp.negative = false;	// only interested in magnitude
	while (!tmp.zero())
	{
		// Shift the ones we have so far to the right
		//
		for (long i=sofar; i>0; i--)
			result[i] = result[i-1];
		
		//
		result[0] = (char)((tmp%(unsigned long)10).ul_value()) + '0';
		tmp /= (unsigned long)10;
		++sofar;
	}
	result[sofar] = 0;
	
	if (negative)
	{
		// Shift the ones we have so far to the right
		//
		for (long i=sofar+1; i>0; i--)
			result[i] = result[i-1];
		
		// Stick in the minus sign
		//
		result[0] = '-';
	}
	
	return result;
}

long BigInt::MPint_length() const
{
	// A zero value is represented with four zero bytes in the length ul,
	// and no value bytes after that.

	if (zero())
		return 4;

	// The length is the size in bytes of the value, plus four for the
	// length ul, plus maybe one more.  We need an extra zero byte if a
	// positive value will have the high bit in the high byte set, since
	// for an MPint this is the flag that the value is negative.  By the
	// same token, we need an extra 0xFF byte if a negative value will
	// have the high bit in the high byte set, since when we twos-
	// complement it it will be clear, and for an MPint this is the flag
	// that the value is positive.  So either way we just need to add one
	// if the high bit will be set.

	long result = byte_length() + 4;
	DIGIT high = value[msd];
	while (high)
	{
		if (high & DIGITHIGHBIT)
		{
			++result;
			break;
		}
		else if (high & (DIGITMASK << (DIGITBITS - 8)))
			break;
		else
			high <<= 8;
	}
	return result;
}

unsigned char* BigInt::MPint_value() const
{
	unsigned char* result = new unsigned char[MPint_length()];
	MPint_value(result);
	return result;
}

void BigInt::MPint_value(unsigned char* result) const
{
	long length = MPint_length()-4;

	int ind = 0;
	result[ind++] = (unsigned char) (length>>24);
	result[ind++] = (unsigned char) ((length>>16) & 0xFF);
	result[ind++] = (unsigned char) ((length>>8) & 0xFF);
	result[ind++] = (unsigned char) (length & 0xFF);

	if (zero())
		return;

	if (length > byte_length())
		result[ind++] = negative ? 0xFF : 0;
	byte_array_value(&result[ind]);
}

// Return the number of (significant) bits in our integer.
unsigned long BigInt::num_bits() const
{
	for (int i=msd; i>=lsd; i--) {
		if (value[i] != 0) {
			for (int bitnum = 7; bitnum >= 0; bitnum--) {
				if (value[i] & (1 << bitnum))
					return ((i - lsd) * 8 + bitnum);
			}
		}
	}

	return 0;
}

// Utilities

// Modify array so it contains the twos-complement of the bytes it holds.
//
void BigInt::complement_bytes(unsigned char* array, long count) const
{
	long i;
	for (i=0; i<count; i++)
		array[i] = ~array[i];
	for (i=count-1; i>=0 && array[i]==0xFF; i--)
		array[i] = 0;
	array[i] += 1;
}

// Extend value[] by the given number of digits.
//
bool BigInt::extend(long digits)
{
	if (digits <= 0)
		return true;

    DIGIT* newvalue = new DIGIT[lsd+1+digits];
    if (!newvalue)  return false;

    memcpy(&newvalue[digits], value, (lsd+1)*DIGITBYTES);
    memset(newvalue, 0, digits*DIGITBYTES);

    delete[] value;
    value = newvalue;
    lsd += digits;
	msd += digits;

	return true;
}

/*
 * Local variables:
 *  tab-width: 4
 *  c-basic-offset: 4
 *  c-file-offsets: ((substatement-open . 0))
 * End:
 */
