#ifndef __BIGINT_H
#define __BIGINT_H
#include <inttypes.h>

#ifndef BIGINT_PRIMITIVE_SIZE
#define BIGINT_PRIMITIVE_SIZE 32
#endif

#if BIGINT_PRIMITIVE_SIZE == 64

// Values stored in 64-bit primitives
#define TWODIGITS uint64_t
#define SIGNEDTWODIGITS int64_t
#define DIGIT uint32_t
#define SIGNEDDIGIT int32_t
#define TWODIGITS_IS_UL 0
#define TWODIGITS_CONSTYPE uint64_t
#define DIGITBYTES 4
#define DIGITBITS 32
#define DIGITMASK 0xFFFFFFFF
#define DIGITHIGHBIT 0x80000000

#elif BIGINT_PRIMITIVE_SIZE == 32

// Values stored in 32-bit primitives
#define DIGIT uint16_t
#define SIGNEDDIGIT int16_t
#define TWODIGITS uint32_t
#define TWODIGITS_IS_UL 1
#define TWODIGITS_CONSTYPE unsigned long
#define SIGNEDTWODIGITS int32_t
#define DIGITBYTES 2
#define DIGITBITS 16
#define DIGITMASK 0xFFFF
#define DIGITHIGHBIT 0x8000
#else

// Values stored in 16-bit primitives
#define DIGIT uint8_t
#define SIGNEDDIGIT int8_t
#define TWODIGITS uint16_t
#define TWODIGITS_IS_UL 0
#define TWODIGITS_CONSTYPE unsigned long
#define SIGNEDTWODIGITS int16_t
#define DIGITBYTES 1
#define DIGITBITS 8
#define DIGITMASK 0xFF
#define DIGITHIGHBIT 0x80

#endif


#define PARTIALS 64
#define WINDOWSIZE 6

class BigInt
{
public:		// constructors & destructors
	BigInt();
	BigInt(const BigInt&);
	BigInt(unsigned char*, long);	// defaults to positive
	BigInt(unsigned char*, long, bool);
	BigInt(long);
	BigInt(TWODIGITS_CONSTYPE);
	BigInt(const char*);
	BigInt(const unsigned char*);
	BigInt(long, DIGIT);
	~BigInt();

public:		// methods
	// Assignment
	bool operator=(const BigInt&);
	bool operator=(const long);
	bool operator=(char*);
	bool copy_bytes(const unsigned char*, long);	// defaults to positive
	bool copy_bytes(const unsigned char*, long, bool);
	bool use_value(DIGIT*, long);	// defaults to positive
	bool use_value(DIGIT*, long, bool);
	void set_high_bit();

	// Addition
	const BigInt& operator++();	// prefix
	const BigInt operator++(int);	// postfix
	BigInt operator+(const BigInt&) const;
	friend BigInt operator+(long, const BigInt&);
	bool operator+=(const BigInt&);

	// Subtraction
	const BigInt& operator--();	// prefix
	const BigInt operator--(int);	// postfix
	BigInt operator-(const BigInt&) const;
	friend BigInt operator-(long, const BigInt&);
	bool operator-=(const BigInt&);

	// Multiplication
	BigInt operator*(const BigInt&) const;
	friend BigInt operator*(long, const BigInt&);
	bool operator*=(const BigInt&);
	bool square();
	bool squaremod(const BigInt&);
	bool negate();

	// Division
	BigInt operator/(const BigInt&) const;
	friend BigInt operator/(long, const BigInt&);
	bool operator/=(const BigInt&);

	// Modulation
	BigInt operator%(const BigInt&) const;
	friend BigInt operator%(long, const BigInt&);
	bool operator%=(const BigInt&);
	bool multmod(const BigInt&, const BigInt&);

	// Exponentiation
	BigInt exp(const BigInt&);
	BigInt expmod(const BigInt&, const BigInt&) const;

	// Multiplicative inverse
	BigInt inv(const BigInt&) const;

	// Multiplicative inverse
	BigInt gcd(const BigInt&) const;

	// Comparison
	friend bool operator==(long, const BigInt&);
	bool operator==(const BigInt&) const;
	friend bool operator!=(long, const BigInt&);
	bool operator!=(const BigInt&) const;
	friend bool operator<(long, const BigInt&);
	bool operator<(const BigInt&) const;
	friend bool operator<=(long, const BigInt&);
	bool operator<=(const BigInt&) const;
	friend bool operator>(long, const BigInt&);
	bool operator>(const BigInt&) const;
	friend bool operator>=(long, const BigInt&);
	bool operator>=(const BigInt&) const;

	bool is_positive() const;
	bool is_negative() const;
	bool zero() const;
	bool one() const;
	bool negative_one() const;
	bool odd() const;
	bool even() const;

	// Shifting
	bool operator<<=(long);
	bool operator>>=(long);

	// Output
	long byte_length() const;
	unsigned char* byte_array_value() const;
	void byte_array_value(unsigned char*) const;
	unsigned char* byte_array_value(long) const;
	void byte_array_value(unsigned char*, long) const;
	long long_value() const;
	unsigned long ul_value() const;
	char* decimal_string_value() const;
//	friend ostream& operator<<(ostream&, const BigInt&);
	long MPint_length() const;
	unsigned char* MPint_value() const;
	void MPint_value(unsigned char*) const;
	unsigned long num_bits() const;

private:	// methods
	// Assignment
	bool set_value(long);
	bool set_value(unsigned long);
	bool set_value(const char*);
	bool set_value(const unsigned char*);
	bool set_zero();
	bool copy_value(DIGIT*, long);	// defaults to positive
	bool copy_value(DIGIT*, long, bool);

	// Addition
	bool add_BigInt(const BigInt&);
	bool add_digit(DIGIT);

	// Subtraction
	bool subtract_from_BigInt(const BigInt&);
	void subtract_BigInt(const BigInt&);
	void subtract_digit(DIGIT);

	// Exponentiation
	BigInt& get_partial (BigInt**, long, const BigInt&) const;

	// Comparison
	int value_compare(const BigInt&) const;

	// Shifting
	bool shift_left_one();

	// Utilities
	void complement_bytes(unsigned char*, long) const;
	bool extend(long digits);

private:	// member variables
	DIGIT* value;
	long msd, lsd;
	bool negative;
};

#endif

/*
 * Local variables:
 *  tab-width: 4
 *  c-basic-offset: 4
 *  c-file-offsets: ((substatement-open . 0))
 * End:
 */
