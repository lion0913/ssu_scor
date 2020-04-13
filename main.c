#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include "ssu_score.h"

#define SECOND_TO_MICRO 1000000

void ssu_runtime(struct timeval *begin_t, struct timeval *end_t);//프로그램 실행 시간을 계산하는 함수

int main(int argc, char *argv[])
{
	struct timeval begin_t, end_t;//프로그램 시작 시간과 종료 시간을 저장하는 변수
	gettimeofday(&begin_t, NULL);//프로그램의 시작시간 저장

	ssu_score(argc, argv);//ssu_score함수 수행

	gettimeofday(&end_t, NULL);//프로그램의 종료 시간 저장
	ssu_runtime(&begin_t, &end_t);//실행 시간을 출력하는 함수로 이동

	exit(0);
}

void ssu_runtime(struct timeval *begin_t, struct timeval *end_t)//실행시간 계산&출력 함수
{
	end_t->tv_sec -= begin_t->tv_sec;//끝난 시간에서 시작 시간을 뺀 값을 저장

	if(end_t->tv_usec < begin_t->tv_usec){//끝난초<시작초라면 오류처리
		end_t->tv_sec--;//끝난시간초 -1
		end_t->tv_usec += SECOND_TO_MICRO;//끝난 시간 마이크로초를 더함
	}

	end_t->tv_usec -= begin_t->tv_usec;//초 계산
	printf("Runtime: %ld:%06ld(sec:usec)\n", end_t->tv_sec, end_t->tv_usec);//출력
}//실행시간 측정
