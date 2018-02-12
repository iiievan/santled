#include <lib_8.h>

/// Generate an 8-bit random number
uint8_t random_8(void)
{
	uint8_t i;
	
	srand(adc_rng_get());   // зерно для получения случайного числа.
	
	i = rand();	    // случайный  кадр из 255-х.
	
	// return the sum of the high and low bytes, for better
	//  mixing and non-sequential correlation
	return i;
}

/// Generate an 8-bit random number between 0 and lim
/// @param lim the upper bound for the result
uint8_t random_lim(uint8_t lim)
{
	uint8_t r = random_8();
	
	r = r%lim;
	
	return r;
}

/// Generate an 8-bit random number in the given range
/// @param min the lower bound for the random number
/// @param lim the upper bound for the random number
uint8_t random_min_max(uint8_t min, uint8_t max)
{
	uint8_t delta = max - min;
	
	uint8_t r = random_lim(delta) + min;
	
	return r;
}

/// add one byte to another, saturating at 0xFF
/// @param i - first byte to add
/// @param j - second byte to add
/// @returns the sum of i & j, capped at 0xFF
uint8_t qadd_8(uint8_t i, uint8_t j)
{
	unsigned int t = i + j;
	if (t > 255) t = 255;
	return t;
}

/// subtract one byte from another, saturating at 0x00
/// @returns i - j with a floor of 0
uint8_t qsub_8(uint8_t i, uint8_t j)
{
	int t = i - j;
	if (t < 0) t = 0;
	return t;
}

uint8_t scale_8_video(uint8_t i, uint8_t scale)
{
	uint8_t j = (i * scale) + ((i && scale) ? 1 : 0);
	// uint8_t nonzeroscale = (scale != 0) ? 1 : 0;
	// uint8_t j = (i == 0) ? 0 : ((i * scale ) >> 8) + nonzeroscale;
	return j;
}