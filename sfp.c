#include "sfp.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define TMAX 0x7fffffff
#define TMIN 0x80000000

sfp int2sfp(int input){
	int sign;
	int cnt = 0;
	sfp temp;
	if(input == 0) return 0;
	else if(input < 0){	// input < 0  -> sign=1, two's complement
		sign = 1;
		input = ~input;
		input += 1;
	}
	else sign = 0;

	while(input / 0x80000000 == 0){
		cnt++;
		input = input << 1;
	}

	input = input & 0x7fc00000;
	input = input >> 6;
	//printf(""BTBP""BTBP"", BTB(input >> 16), BTB(input));
	cnt = 62 - cnt;

	input += (cnt << 25);

	if(sign == 1) input += 0x80000000;
	input = input >> 16;

	memcpy((sfp*)(&temp), (int*)(&input), 2);
	return temp;
}

int sfp2int(sfp input){	//need exception case
	int sign, exp;
	int temp;
	sfp frac;
	sign = input / 0x8000;

	exp = input << 1;
	exp = exp >> 10;
	exp = exp - 31;

	frac = input & 0x01ff;

	if(exp == 32){
		if(frac == 0){
			if(sign==0) return TMAX;
			else return TMIN;
		}
		else{
			return TMIN;
		}
	}
	else if(exp < 0){
		return 0;
	}
	else if(exp == 31){
		if(sign==0) return TMAX;
		else {
			if(frac == 0) return 0x80000000;
			return TMIN;
		}
	}

	memcpy((int*)(&temp), (sfp*)(&input), 4);
	temp = temp << 21;
	temp = temp & 0x3fe00000;
	temp = temp + 0x40000000;
	temp = temp >> (30 - exp);

	if(sign == 1){
		temp -= 1;
		temp = ~temp;
	}
	return temp;
}

sfp float2sfp(float input){
	int sign, exp, frac, temp;
	sfp s_sign;
	memcpy((int*)(&sign), (float*)(&input), 4);
	memcpy((int*)(&exp), (float*)(&input), 4);
	memcpy((int*)(&frac), (float*)(&input), 4);

	sign = sign & 0x80000000;
	temp = sign >> 16;
	memcpy((sfp*)(&s_sign), (int*)(&temp), 2);
	
	exp = exp & 0x7f800000;
	exp = exp >> 23;
	exp = exp - 127;
	frac = frac & 0x007fffff;
	frac = frac >> 14;

	if(input == 0) return 0;
	if(exp > 31){
		return (0x7e00 + s_sign);
	}
	else if(exp <= -31){
		frac = frac + 0x00000200;
		int diff = -31 - exp;
		diff++;
		frac = frac >> diff;
	}
	sign = sign >> 16;
	exp = exp + 31;
	exp = exp << 9;

	temp = sign + exp + frac;
	sfp sol;
	memcpy((sfp*)(&sol), (int*)(&temp), 2);
	return sol;
}

float sfp2float(sfp input){
	int temp;
	int sign, exp, frac;
	float output;
	int cnt = 0;

	if(input == 0){
		return 0;
	}

	memcpy((int*)(&sign), (sfp*)(&input), 2);
	memcpy((int*)(&exp), (sfp*)(&input), 2);
	memcpy((int*)(&frac), (sfp*)(&input), 2);
	sign = sign & 0x00008000;
	sign = sign << 16;
	exp = exp & 0x00007e00;
	exp = exp >> 9;
	exp = exp - 31;
	frac = frac & 0x000001ff;

	if(exp == -31){
		while(frac < 0x00000200){
			cnt++;
			frac = frac << 1;
		}
		frac -= 0x00000200;
		frac = frac << 14;
		exp -= cnt;
		exp += 127;
		exp = exp << 23;
	}
	else if(exp == 32){
		if(frac == 0){
			temp = sign + 0x7f800000 + frac;
			memcpy((float*)(&output), (int*)(&temp), 4);
			return output;
		}
	}
	else{
		exp += 127;
		exp = exp << 23;
		frac = frac << 14;
	}


	temp = sign + exp + frac;
	memcpy((float*)(&output), (int*)(&temp), 4);
	return output;
}

sfp sfp_add(sfp in1, sfp in2){
	int sign[2], exp[2], frac[2];
	int s, e, f;
	int diff=0, cnt = 0;
	int max, min;

	sign[0] = in1 / 0x8000;
	sign[1] = in2 / 0x8000;

	exp[0] = in1 & 0x7e00;
	exp[0] = exp[0] >> 9;
	exp[0] = exp[0] - 31;
	exp[1] = in2 & 0x7e00;
	exp[1] = exp[1] >> 9;
	exp[1] = exp[1] - 31;

	frac[0] = (in1 & 0x01ff);
	frac[1] = (in2 & 0x01ff);

	if(exp[0] > -31 && exp[0] < 32) frac[0] += 0x0200;
	if(exp[1] > -31 && exp[1] < 32) frac[1] += 0x0200;

	if(sign[0] != sign[1]){
		if(exp[0] > exp[1]){
			max = 0;
			min = 1;
		}
		else if (exp[0] < exp[1]){
			max = 1;
			min = 0;
		}
		else{
			max = 0;
			min = 0;
		}

		if(min != max){
			e = exp[max];
			s = sign[max];
			diff = exp[max] - exp[min];
			frac[min] = frac[min] >> diff;
			f = frac[max] - frac[min];

			if(e >= 32){
				if(frac[max] != 0) return (s*0x8000) + 0x7e00 + frac[max];
				else{
					return (s*0x8000) + 0x7e00;
				}
			}

			while(f < 0x0200 && f != 0){
				cnt++;
				f = f << 1;
			}
			e -= cnt;
			if (e > -31){
				f = f - 0x0200;
				return (s * 0x8000) + ((e + 31) << 9) + f;
			}
			else{
				diff = -31 - e + 1;
				f = f >> diff;
				return (s * 0x8000) + f;
			}
		}
		else{
			if(frac[0] > frac[1]){
				max = 0;
				min = 1;
			}
			else if(frac[0] < frac[1]){
				max = 1;
				min = 0;
			}
			else{
				max = 0;
				min = 0;
			}

			if(max != min){
				e = exp[max];
			       	s = sign[max];
				f = frac[max] - frac[min];

				if(e >= 32){
					return (s * 0x8000) + 0x7e00 + f;
				}

				while(f < 0x0200){
					cnt++;
					f = f << 1;
				}
				e -= cnt;
				if (e > -31){
					f = f - 0x0200;
					return (s * 0x8000) + ((e + 31) << 9) + f;
				}
				else{
					diff = -31 - e + 1;
					f = f >> diff;
					return (s * 0x8000) + f;
				}
			}
			else {
				if(exp[max] >= 32){
					return (s * 0x8000) + 0x7e11;
				}
				return 0;
			}
		}
	}
	else {
		s = sign[0];
		if(exp[0] >= exp[1]){
			max = 0;
			min = 1;
		}
		else if(exp[0] < exp[1]){
			max = 1;
			min = 0;
		}
	
		e = exp[max];
		diff = exp[max] - exp[min];
		frac[min] = frac[min] >> diff;

		if(e >= 32){
			return (s * 0x8000) + 0x7e00 + frac[max];
		}

		f = frac[max] + frac[min];
		if (f >= 0x0400){
			printf("%d\n", e);
			f = f >> 1;
			e += 1;
		}
		if(e >= 32) return (s*0x8000) + 0x7e00;
		else if(e > -31) f = f - 0x0200;
		return (s * 0x8000) + ((e + 31) << 9) + f;
	}
}

sfp sfp_mul(sfp in1, sfp in2){
	int sign[2], exp[2], frac[2];
	int s, e, f;
	int diff=0, cnt = -9;
	sign[0] = in1 / 0x8000;
	sign[1] = in2 / 0x8000;

	exp[0] = in1 & 0x7e00;
	exp[0] = exp[0] >> 9;
	exp[0] = exp[0] - 31;
	exp[1] = in2 & 0x7e00;
	exp[1] = exp[1] >> 9;
	exp[1] = exp[1] - 31;

	frac[0] = (in1 & 0x01ff);
	frac[1] = (in2 & 0x01ff);

	if(exp[0] > -31) frac[0] += 0x0200;
	if(exp[1] > -31) frac[1] += 0x0200;

	s = sign[0] ^ sign[1];
	e = exp[0] + exp[1];
	f = frac[0] * frac[1];

	if(f == 0){
		return 0;
	}

	while(f >= 0x00000400){
		cnt++;
		f = f >> 1;
	}
	while(f < 0x00000200){
		cnt--;
		f = f << 1;
	}
	e += cnt;

	if(e<=-31){
		diff = -31 - e + 1;
		f = f >> diff;
		e += 31;
		return (s * 0x8000) + (e << 9) + f;
	}
	else{
		f -= 0x0200;
		e += 31;
		return (s * 0x8000) + (e << 9) + f;
	}
	
}
