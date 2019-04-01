#include "sfp.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

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
	int sign, cnt;
	int temp;
	sfp frac;
	sign = input / 0x8000;

	cnt = input << 1;
	cnt = cnt >> 10;
	cnt = cnt - 31;

	frac = input & 0x01ff;

	if(cnt == 32){
		if(frac == 0){
			if(sign==0) return 0x7fffffff;
			else return 0x80000000;
		}
		else{
			return 0x80000000;
		}
	}
	else if(cnt < 0){
		return 0;
	}
	else if(cnt == 31){
		if(sign==0) return 0x7fffffff;
		else {
			printf("asdfasdf");
			if(frac == 0) return 0x80000000;
		}
	}

	memcpy((int*)(&temp), (sfp*)(&input), 4);
	temp = temp << 21;
	temp = temp & 0x3fe00000;
	temp = temp + 0x40000000;
	temp = temp >> (30 - cnt);

	if(sign == 1){
		temp -= 1;
		temp = ~temp;
	}
	return temp;
}

sfp float2sfp(float input){
	int sign, exp, frac;
	memcpy((int*)(&sign), (float*)(&input), 4);
	memcpy((int*)(&exp), (float*)(&input), 4);
	memcpy((int*)(&frac), (float*)(&input), 4);
	sign = sign & 0x80000000;
	exp = exp & 0x7f800000;
	exp = exp >> 23;
	exp = exp - 127;
	frac = frac & 0x007fffff;
	frac = frac >> 14;
	if(input == 0) return 0;
	if(exp > 31){
		return (0x7e00 + sign);
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

	int temp = sign + exp + frac;
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
		frac = frac << 14;
		exp -= cnt;
		exp += 127;
		exp = exp << 23;
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
	sign[0] = in1 / 0x8000;
	sign[1] = in2 / 0x8000;

	exp[0] = in1 & 0x7e00;
	exp[0] = exp[0] >> 9;
	exp[0] = exp[0] - 31;
	exp[1] = in2 & 0x7e00;
	exp[1] = exp[1] >> 9;
	exp[1] = exp[1] - 31;

	frac[0] = (in1 & 0x01ff) + 0x0200;
	frac[1] = (in2 & 0x01ff) + 0x0200;

	if(sign[0] != sign[1]){
		if(exp[0] > exp[1]){
			e = exp[0];
			s = sign[0];
			diff = exp[0] - exp[1];
			frac[1] = frac[1] >> diff;
			f = frac[0] - frac[1];
			while(f < 0x0200){
				cnt++;
				f << 1;
			}
			f = f - 0x0200;
			e -= cnt;
			return (s * 0x8000) + ((e + 31) << 9) + f;
		}
		else if(exp[0] < exp[1]){
			e = exp[1];
		       	s = sign[1];
			diff = exp[1] - exp[0];
			frac[0] = frac[0] >> diff;
			f = frac[1] - frac[0];
			while(f < 0x0200){
				cnt++;
				f << 1;
			}
			f = f - 0x0200;
			e -= cnt;
			return (s * 0x8000) + ((e + 31) << 9) + f;
		}
		else{
			if(frac[0] > frac[1]){
				e = exp[0];
			       	s = sign[0];
				f = frac[0] - frac[1];
				while(f < 0x0200){
					cnt++;
					f << 1;
				}
				f = f - 0x0200;
				e -= cnt;
				return (s * 0x8000) + ((e + 31) << 9) + f;
			}
			else if(frac[0] < frac[1]){
				e = exp[1];
			       	s = sign[1];
				f = frac[1] - frac[0];
				while(f < 0x0200){
					cnt++;
					f << 1;
				}
				f = f - 0x0200;
				e -= cnt;
				return (s * 0x8000) + ((e + 31) << 9) + f;
			}
			else return 0;
		}
	}
	else {
		s = sign[0];
		if(exp[0] > exp[1]){
			e = exp[0];
			diff = exp[0] - exp[1];
			frac[1] = frac[1] >> diff;
		}
		else if(exp[1] >= exp[0]){
			e = exp[1];
			diff = exp[1] - exp[0];
			frac[0] = frac[0] >> diff;
		}
		f = frac[0] + frac[1];
		if(f >= 0x0400){
			f >> 1;
			e += 1;
		}
		f = f - 0x0200;
		return (s * 0x8000) + ((e + 31) << 9) + f;
	}
}

sfp sfp_mul(sfp in1, sfp in2){
}
