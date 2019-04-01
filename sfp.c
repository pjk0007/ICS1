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
	if(exp > 31){
		return (0x7e00 + sign);
	}
	else if(exp < -31){
		return 0;
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
	exp = exp - 31 + 127;
	exp = exp << 23;
	frac = frac & 0x000001ff;
	frac = frac << 14;

	temp = sign + exp + frac;
	memcpy((float*)(&output), (int*)(&temp), 4);
	return output;
}

sfp sfp_add(sfp in1, sfp in2){
}

sfp sfp_mul(sfp in1, sfp in2){
}
