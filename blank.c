#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <ctype.h>
#include "blank.h"

char datatype[DATATYPE_SIZE][MINLEN] = {"int", "char", "double", "float", "long"
			, "short", "ushort", "FILE", "DIR","pid"
			,"key_t", "ssize_t", "mode_t", "ino_t", "dev_t"
			, "nlink_t", "uid_t", "gid_t", "time_t", "blksize_t"
			, "blkcnt_t", "pid_t", "pthread_mutex_t", "pthread_cond_t", "pthread_t"
			, "void", "size_t", "unsigned", "sigset_t", "sigjmp_buf"
			, "rlim_t", "jmp_buf", "sig_atomic_t", "clock_t", "struct"};//datatype모음집


operator_precedence operators[OPERATOR_CNT] = {
	{"(", 0}, {")", 0}
	,{"->", 1}	
	,{"*", 4}	,{"/", 3}	,{"%", 2}	
	,{"+", 6}	,{"-", 5}	
	,{"<", 7}	,{"<=", 7}	,{">", 7}	,{">=", 7}
	,{"==", 8}	,{"!=", 8}
	,{"&", 9}
	,{"^", 10}
	,{"|", 11}
	,{"&&", 12}
	,{"||", 13}
	,{"=", 14}	,{"+=", 14}	,{"-=", 14}	,{"&=", 14}	,{"|=", 14}
};//연산자 우선순위

void compare_tree(node *root1,  node *root2, int *result)//트리를 비교하는 함수(정답트리, 학생이 입력한 답안트리)
{
	node *tmp;
	int cnt1, cnt2;

	if(root1 == NULL || root2 == NULL){//트리 자체가 없는 경우 
		*result = false;
		return;
	}

	if(!strcmp(root1->name, "<") || !strcmp(root1->name, ">") || !strcmp(root1->name, "<=") || !strcmp(root1->name, ">=")){
		if(strcmp(root1->name, root2->name) != 0){//정답의 부호와 학생의 부호가 다른경우

			if(!strncmp(root2->name, "<", 1))//정답문자에 반대방향의 부호를 복사
				strncpy(root2->name, ">", 1);

			else if(!strncmp(root2->name, ">", 1))
				strncpy(root2->name, "<", 1);

			else if(!strncmp(root2->name, "<=", 2))
				strncpy(root2->name, ">=", 2);

			else if(!strncmp(root2->name, ">=", 2))
				strncpy(root2->name, "<=", 2);

			root2 = change_sibling(root2);//형제를 바꿔주는 함수
		}
	}

	if(strcmp(root1->name, root2->name) != 0){//복사한 부호를 비교해도 학생이 입력한 부호와 다른경우
		*result = false;//틀림
		return;
	}

	if((root1->child_head != NULL && root2->child_head == NULL)
		|| (root1->child_head == NULL && root2->child_head != NULL)){//어느 한쪽의 자식노드가 없는 경우
		*result = false;
		return;
	}

	else if(root1->child_head != NULL){//학생이 입력한 답안의 자식트리는 있는경우
		if(get_sibling_cnt(root1->child_head) != get_sibling_cnt(root2->child_head)){//그 자식의 형제수가 다른경우
			*result = false;
			return;
		}

		if(!strcmp(root1->name, "==") || !strcmp(root1->name, "!="))//부호가 ==이나 !=이면
		{
			compare_tree(root1->child_head, root2->child_head, result);//다시한번 자식트리를 비교(재귀)

			if(*result == false)//result가 거짓이면 참으로 바꾸고 형제노드를 서로 바꾸어서 재비교함
			{
				*result = true;
				root2 = change_sibling(root2);
				compare_tree(root1->child_head, root2->child_head, result);
			}
		}
		else if(!strcmp(root1->name, "+") || !strcmp(root1->name, "*")
				|| !strcmp(root1->name, "|") || !strcmp(root1->name, "&")
				|| !strcmp(root1->name, "||") || !strcmp(root1->name, "&&"))
		{//해당하는 연산자의 경우(특징 : 양변이 바뀌어도 전혀 의미가 없는 연산자임)
			if(get_sibling_cnt(root1->child_head) != get_sibling_cnt(root2->child_head)){//자식의 수가 다를 경우에 false
				*result = false;
				return;
			}

			tmp = root2->child_head;//답안 트리의 값을 tmp에 임시저장

			while(tmp->prev != NULL)//tmp의 맨 끝(가장 자식)으로 이동
				tmp = tmp->prev;

			while(tmp != NULL)//
			{
				compare_tree(root1->child_head, tmp, result);//학생답안파일의 자식트리와 tmp에 저장한 값의 트리를 비교
			
				if(*result == true)
					break;
				else{
					if(tmp->next != NULL)
						*result = true;
					tmp = tmp->next;
				}
			}
		}
		else{//그 외의 경우
			compare_tree(root1->child_head, root2->child_head, result);//학생답안파일의 자식트리와 정답파일의 자식트리 비교
		}
	}	


	if(root1->next != NULL){//자식트리의 형제를 차례로 탐색

		if(get_sibling_cnt(root1) != get_sibling_cnt(root2)){//만약 답안파일의 형제노드수와 학생파일의 형제노드수가 다르다면 false를 리턴
			*result = false;
			return;
		}

		if(*result == true)//만약 결과가 true라면 
		{
			tmp = get_operator(root1);//tmp에 연산자를 읽어옴
	
			if(!strcmp(tmp->name, "+") || !strcmp(tmp->name, "*")
					|| !strcmp(tmp->name, "|") || !strcmp(tmp->name, "&")
					|| !strcmp(tmp->name, "||") || !strcmp(tmp->name, "&&"))
			{	//만약 연산자가 다음과 같다면
				tmp = root2;//tmp에 답안을 저장
	
				while(tmp->prev != NULL)//prev노드로 이동
					tmp = tmp->prev;

				while(tmp != NULL)
				{
					compare_tree(root1->next, tmp, result);//학생파일의 형제노드와 tmp트리를 비교함

					if(*result == true)
						break;
					else{
						if(tmp->next != NULL)
							*result = true;
						tmp = tmp->next;
					}
				}
			}

			else//그외의 연산자인경우
				compare_tree(root1->next, root2->next, result);//그대로 비교
		}
	}
}

int make_tokens(char *str, char tokens[TOKEN_CNT][MINLEN])//주어진 문자열을 언어요소로 만드는 함수(str : 답안,tokens:언어요소를 저장할 토큰배열)
{
	char *start, *end;//문자열을 탐색해서 토큰으로 만들기 위한 포인터
	char tmp[BUFLEN];
	char str2[BUFLEN];
	char *op = "(),;><=!|&^/+-*\""; //연산자리스트
	int row = 0;//현재 채워진 토큰의 인덱스
	int i;
 	int isPointer;
	int lcount, rcount;
	int p_str;
	
	clear_tokens(tokens);//토큰배열 초기화

	start = str;//문자열의 시작주소 가져옴
	
	if(is_typeStatement(str) == 0)//가져온 답안이 데이터타입인지 체크
		return false;	
	
	while(1)
	{
		if((end = strpbrk(start, op)) == NULL)//연산자가(op) 없다면 break
			break;

		if(start == end){//기호가 맨앞에 있다면

			if(!strncmp(start, "--", 2) || !strncmp(start, "++", 2)){//문자열이 증감연산자인경우
				if(!strncmp(start, "++++", 4)||!strncmp(start,"----",4))//잘못된 증감연산자를 사용한 경우에 false를 리턴
					return false;

				if(is_character(*ltrim(start + 2))){//증감연산자 바로 뒤에 문자가 온다면
					if(row > 0 && is_character(tokens[row - 1][strlen(tokens[row - 1]) - 1]))//한개이상의 토큰을 가지고 있고 마지막이 문자라면 false를 리턴
						return false; 

					end = strpbrk(start + 2, op);//다음에 나오는 연산자를 읽어들이기 전까지 end를 계속 증가함
					if(end == NULL)//end가 문자의 끝을 넘어서면 str의 마지막위치에 end를 둔다
						end = &str[strlen(str)];
					while(start < end) {//start부터 end(다음 연산자 전)까지
						if(*(start - 1) == ' ' && is_character(tokens[row][strlen(tokens[row]) - 1]))
							return false;//start이전문자가 ' '이고 마지막 인자가 문자라면 false를 리턴한다
						else if(*start != ' ')
							strncat(tokens[row], start, 1);//토큰에 start 1바이트를 붙임
						start++;//그 다음 문자를 가리키게함
					}
				}
				
				else if(row>0 && is_character(tokens[row - 1][strlen(tokens[row - 1]) - 1])){//이전 토큰이 문자라면 
					if(strstr(tokens[row - 1], "++") != NULL || strstr(tokens[row - 1], "--") != NULL)//그 문자열이 ++이거나 --인경우 false
						return false;

					memset(tmp, 0, sizeof(tmp));//tmp를 비움
					strncpy(tmp, start, 2);//tmp에 증감연산자를 복사한다
					strcat(tokens[row - 1], tmp);//토큰내용에 tmp를 복사
					start += 2;//start 두칸 이동
					row--;//현재 토큰개수를 감소
				}
				else{//그외
					memset(tmp, 0, sizeof(tmp));//tmp비우고
					strncpy(tmp, start, 2);//증감연산자 복사하고
					strcat(tokens[row], tmp);//그대로 2바이트 저장
					start += 2;//start이동
				}
			}

			else if(!strncmp(start, "==", 2) || !strncmp(start, "!=", 2) || !strncmp(start, "<=", 2)
				|| !strncmp(start, ">=", 2) || !strncmp(start, "||", 2) || !strncmp(start, "&&", 2) 
				|| !strncmp(start, "&=", 2) || !strncmp(start, "^=", 2) || !strncmp(start, "!=", 2) 
				|| !strncmp(start, "|=", 2) || !strncmp(start, "+=", 2)	|| !strncmp(start, "-=", 2) 
				|| !strncmp(start, "*=", 2) || !strncmp(start, "/=", 2)){//문자열이 비교, 대입연산자인경우

				strncpy(tokens[row], start, 2);//그 2바이트를 토큰에 저장하고
				start += 2;//start 2바이트 이동시킴
			}
			else if(!strncmp(start, "->", 2))//포인터접근연산자인경우
			{
				end = strpbrk(start + 2, op);//그 뒤를 end로 지정

				if(end == NULL)
					end = &str[strlen(str)];//end가 문자열의 긑을 넘어섰다면 str의 끝을 end로 지정

				while(start < end){//end까지 반복
					if(*start != ' ')//start가 공백이 아니면 
						strncat(tokens[row - 1], start, 1);//그 값을 앞의 토큰에 이어붙인다
					start++;//start 한칸 이동
				}
				row--;//토큰개수 1 감소
			}
			else if(*end == '&')//end가 '&'인 경우
			{
				
				if(row == 0 || (strpbrk(tokens[row - 1], op) != NULL)){//이전 토큰이나 첫번째 토큰에 연산자가 없었다면 
					end = strpbrk(start + 1, op);//end에 연산자위치+1을 저장함
					if(end == NULL)//end가 문자열의 끝을 넘었다면 
						end = &str[strlen(str)];//str의 마지막 문자를 end를 지정
					
					strncat(tokens[row], start, 1);//토큰에 &를 집어넣음
					start++;//start & 크기(1바이트)만큼 이동

					while(start < end){//end가기 전까지 차례로 증가
						if(*(start - 1) == ' ' && tokens[row][strlen(tokens[row]) - 1] != '&')//만약 이전값이 공백인데 마지막문자가 &인거라면 false리턴
							return false;
						else if(*start != ' ')//공백이아니라면
							strncat(tokens[row], start, 1);//토큰에 차례로 하나씩 붙임
						start++;//start위치 이동
					}
				}
				
				else{//그외
					strncpy(tokens[row], start, 1);//토큰에 1바트 붙이고 이동
					start += 1;
				}
				
			}
		  	else if(*end == '*')//end가 *을 읽어들인 경우
			{
				isPointer=0;

				if(row > 0)
				{
					
					for(i = 0; i < DATATYPE_SIZE; i++) {// 데이터타입이 맞는지 확인하기 위함임
						if(strstr(tokens[row - 1], datatype[i]) != NULL){//앞 토큰에 datatype이 들어간다면
							strcat(tokens[row - 1], "*");//앞토큰에 *을 추가하고
							start += 1;//start위치 한칸 이동하고
							isPointer = 1;//포인터임을 알림
							break;
						}
					}
					if(isPointer == 1)//만약 포인터면 탈출
						continue;
					if(*(start+1) !=0)//다음문자가 NULL이 아니라면 
						end = start + 1; //end를 한칸 이동함

					
					if(row>1 && !strcmp(tokens[row - 2], "*") && (all_star(tokens[row - 1]) == 1)){//만약 현재토큰의 두바이트 전에도 *이 있다면 
						strncat(tokens[row - 1], start, end - start);//앞토큰에 갖다붙이고
						row--;//현재 토큰개수 감소함
					}
					
					
					else if(is_character(tokens[row - 1][strlen(tokens[row - 1]) - 1]) == 1){//이전토큰에 문자(숫자 or 알파벳)가 들어간다면
						strncat(tokens[row], start, end - start);   //현재토큰에 연결함
					}

					
					else if(strpbrk(tokens[row - 1], op) != NULL){//이전 토큰에 연산자가 하나라도 있으면
						strncat(tokens[row] , start, end - start); //현재토큰에 연결함
							
					}
					else//그외의 경우에도 현재토큰에 end-start만큼의 값을 입력함
						strncat(tokens[row], start, end - start);

					start += (end - start);//start위치 토큰의 크기만큼 이동
				}

			 	else if(row == 0)//첫번째 토큰이라면 
				{
					if((end = strpbrk(start + 1, op)) == NULL){//바로뒤에 연산자가 없다면
						strncat(tokens[row], start, 1);//토큰에 갖다붙이고 
						start += 1;//start위치 1증가
					}
					else{//바로뒤에 연산자가 있다면 
						while(start < end){
							if(*(start - 1) == ' ' && is_character(tokens[row][strlen(tokens[row]) - 1]))//
								return false;
							else if(*start != ' ')
								strncat(tokens[row], start, 1);
							start++;	
						}
						if(all_star(tokens[row]))
							row--;
						
					}
				}
			}
			else if(*end == '(') //(인경우 
			{
				lcount = 0;//'('와 ')'의 개수를 각각 세는 변수 설정
				rcount = 0;
				if(row>0 && (strcmp(tokens[row - 1],"&") == 0 || strcmp(tokens[row - 1], "*") == 0)){//(앞토큰에 *나 &이 나온경우
					while(*(end + lcount + 1) == '(')//(이 계속 나온다면 lcount증가하고
						lcount++;
					start += lcount;//괄호가 끝나는 위치로 start이동

					end = strpbrk(start + 1, ")");//start 이후부터 )이 나오는 위치에 end를 저장

					if(end == NULL)//")"이 없다면 false
						return false;
					else{//
						while(*(end + rcount +1) == ')')//)의 개수 셈
							rcount++;
						end += rcount;//end위치를 괄호닫는 다음으로 이동

						if(lcount != rcount)//"("와 ")"의 개수가 다른 경우 false
							return false;

						if( (row > 1 && !is_character(tokens[row - 2][strlen(tokens[row - 2]) - 1])) || row == 1){//두개 전 토큰이 문자가 아니라면
							strncat(tokens[row - 1], start + 1, end - start - rcount - 1);//이전 토큰에 괄호안의내용을 집어넣는다
							row--;//현재 토큰 수 감소
							start = end + 1;//괄호 다음으로 start위치 이동
						}
						else{//
							strncat(tokens[row], start, 1);//현재토큰에 start 1바이트 갖다붙임
							start += 1;//start위치이동
						}
					}
						
				}
				else{//그 외의경우엔 
					strncat(tokens[row], start, 1);//1바이트를 붙이고 
					start += 1;//위치이동
				}

			}
			else if(*end == '\"') //따옴표가 들어가면
			{
				end = strpbrk(start + 1, "\"");//end에 그다음에 나오는 따옴표의 위치를 대입
				
				if(end == NULL)//다음따옴표가 안나오면 false
					return false;

				else{//그게아니면
					strncat(tokens[row], start, end - start + 1);//토큰에 따옴표 사이의 내용을 복사함
					start = end + 1;//start위치이동
				}

			}

			else{//그 외엔
				
				if(row > 0 && !strcmp(tokens[row - 1], "++"))//이전토큰이 ++라면 false
					return false;

				
				if(row > 0 && !strcmp(tokens[row - 1], "--"))//이전토큰이 --면 false
					return false;
	
				strncat(tokens[row], start, 1);//현재토큰에 1바이트 붙임
				start += 1;//start위치 이동
				
			
				if(!strcmp(tokens[row], "-") || !strcmp(tokens[row], "+") || !strcmp(tokens[row], "--") || !strcmp(tokens[row], "++")){//현재 토큰이 다음과 같다면

				
					if(row == 0)//토큰이 처음이면 row를 하나 빼줌
						row--;

					
					else if(!is_character(tokens[row - 1][strlen(tokens[row - 1]) - 1])){//이전토큰의 마지막 인자가 문자가아니면
					
						if(strstr(tokens[row - 1], "++") == NULL && strstr(tokens[row - 1], "--") == NULL)//이전토큰에 ++,-- 없다면 row를 감소함
							row--;
					}
				}
			}
		}
		else{ //start와 end가 다르면(맨 앞이 아니라면) 
			if(all_star(tokens[row - 1]) && row > 1 && !is_character(tokens[row - 2][strlen(tokens[row - 2]) - 1]))   //만약 이전 토큰이 포인터라면 현재토큰위치 감소(row>2)
				row--;				

			if(all_star(tokens[row - 1]) && row == 1)//이전 토큰이 포인터라면(row==1)
				row--;	

			for(i = 0; i < end - start; i++){//토큰의 길이만큼 하나씩 증가
				if(i > 0 && *(start + i) == '.'){//만약에 그 사이에 .이 들어가면
					strncat(tokens[row], start + i, 1);//토큰에 그 .을 붙여줌

					while( *(start + i +1) == ' ' && i< end - start )
						i++; 
				}
				else if(start[i] == ' '){//만약에 start의 위치에 스페이스가 들어간다면
					while(start[i] == ' ')//스페이스의 개수만큼을 세서 제거한다
						i++;
					break;
				}
				else//그외의 경우엔 토큰에 값을 추가함
					strncat(tokens[row], start + i, 1);
			}

			if(start[0] == ' '){//첫번째 토큰이 비어있다면 개수를 저장함(제거하기 위해)
				start += i;
				continue;
			}
			start += i;//start이동
		}
			
		strcpy(tokens[row], ltrim(rtrim(tokens[row])));//좌우공백 제거하여서 다시 token에 저장

		 if(row > 0 && is_character(tokens[row][strlen(tokens[row]) - 1]) 
				&& (is_typeStatement(tokens[row - 1]) == 2 
					|| is_character(tokens[row - 1][strlen(tokens[row - 1]) - 1])
					|| tokens[row - 1][strlen(tokens[row - 1]) - 1] == '.' ) ){//(1)현재토큰의 마지막이 char이고이고 이전 토큰이 데이터타입이거나 (2)이전토큰이 문자이거나 (3)앞토큰이 .으로 끝난다면

			if(row > 1 && strcmp(tokens[row - 2],"(") == 0)//전전토큰이 (일때
			{
				if(strcmp(tokens[row - 1], "struct") != 0 && strcmp(tokens[row - 1],"unsigned") != 0)
					return false;//바로전 토큰이 struct와 unsigned가 아니라면 false임
			}
			else if(row == 1 && is_character(tokens[row][strlen(tokens[row]) - 1])) {//현재토큰위치가 1이고 끝이 문자일때
				if(strcmp(tokens[0], "extern") != 0 && strcmp(tokens[0], "unsigned") != 0 && is_typeStatement(tokens[0]) != 2)//첫 토큰이 extern,unsigned가 아니고 데이터타입도 아니라면 false를 리턴함
					return false;
			}
			else if(row > 1 && is_typeStatement(tokens[row - 1]) == 2){//이전 토큰이 데이터타입일때(row>1)
				if(strcmp(tokens[row - 2], "unsigned") != 0 && strcmp(tokens[row - 2], "extern") != 0)
					return false;//전전토큰이 unsigned나 extern이 아니라면 false
			}
			
		}

		if((row == 0 && !strcmp(tokens[row], "gcc")) ){//첫번째토큰이 gcc면 
			clear_tokens(tokens);//토큰 비움
			strcpy(tokens[0], str);	//토큰에 str복사
			return 1;
		} 

		row++;
	}

	if(all_star(tokens[row - 1]) && row > 1 && !is_character(tokens[row - 2][strlen(tokens[row - 2]) - 1]))  
		row--;//이전토큰에 *이 들어가고 전전토큰의 끝이 문자가 아니라면 토큰크기 1감소
	if(all_star(tokens[row - 1]) && row == 1)   
		row--;	//이전토큰에 *이 들어가고 row=1이면 현재토큰크기 1감소함

	for(i = 0; i < strlen(start); i++)
	{
		if(start[i] == ' ')//공백일경우 공백이 끝나기 전까지 i를 하나씩 차례로 증가
		{
			while(start[i] == ' ')
				i++;
			if(start[0]==' ') {//처음이 공백이면
				start += i;//공백이 아닐때까지 i를 증가시키고(start위치를 이동하는거임) 다시 초기화함
				i = 0;
			}
			else
				row++;//처음이 공백이 아닐경우엔 토큰개수 증가함
			
			i--;
		} 
		else
		{
			strncat(tokens[row], start + i, 1);//현재 토큰에 1바이트만큼을 추가저장
			if( start[i] == '.' && i<strlen(start)){//만약 현재위치가 .이고 i가 start길이를 넘지 않았다면
				while(start[i + 1] == ' ' && i < strlen(start))//' '이 끝날때까지 i를 증가시킴
					i++;

			}
		}
		strcpy(tokens[row], ltrim(rtrim(tokens[row])));//공백을 제거 후 토큰에 집어넣음

		if(!strcmp(tokens[row], "lpthread") && row > 0 && !strcmp(tokens[row - 1], "-")){ //만약 -이 나오는 토큰 뒤에 lpthread를 만나게 된다면 옵션에 해당하므로 하나로 합쳐줘야함
			strcat(tokens[row - 1], tokens[row]);//이전 토큰으로 합치는 과정
			memset(tokens[row], 0, sizeof(tokens[row]));//현재토큰에 있던 값 초기화
			row--;//현재 토큰개수 감소
		}
	 	else if(row > 0 && is_character(tokens[row][strlen(tokens[row]) - 1]) 
				&& (is_typeStatement(tokens[row - 1]) == 2 
					|| is_character(tokens[row - 1][strlen(tokens[row - 1]) - 1])
					|| tokens[row - 1][strlen(tokens[row - 1]) - 1] == '.') ){
			//현재 토큰의 끝이 문자이고 데이터타입이거나 이전 토큰의 끝이 문자이거나 '.'일때
			if(row > 1 && strcmp(tokens[row-2],"(") == 0)
			{//전전토큰이 (이고 토큰이 2 이상일때
				if(strcmp(tokens[row-1], "struct") != 0 && strcmp(tokens[row-1], "unsigned") != 0)
					return false;//이전토큰이 struct이나 unsigned이 아닐때 false를 리턴함
			}
			else if(row == 1 && is_character(tokens[row][strlen(tokens[row]) - 1])) {//현재 입력토큰이 1개이고 그 마지막 인자가 문자라면
				if(strcmp(tokens[0], "extern") != 0 && strcmp(tokens[0], "unsigned") != 0 && is_typeStatement(tokens[0]) != 2)	//그 토큰이 extern이나 unsigned가 아니라면 false를 리턴
					return false;
			}
			else if(row > 1 && is_typeStatement(tokens[row - 1]) == 2){//토큰이 2개이상이고 이전토큰이 데이터타입에 해당한다면
				if(strcmp(tokens[row - 2], "unsigned") != 0 && strcmp(tokens[row - 2], "extern") != 0)
					return false;//전전토큰이 unsigned이거나 extern이 아니라면 false를 리턴
			}
		} 
	}


	if(row > 0)//토큰이 1개이상인 경우
	{

		
		if(strcmp(tokens[0], "#include") == 0 || strcmp(tokens[0], "include") == 0 || strcmp(tokens[0], "struct") == 0){ //토큰의 시작에 #include,include,struct중 하나의 문자열이 들어간다면
			clear_tokens(tokens); //토큰을 초기화하고
			strcpy(tokens[0], remove_extraspace(str)); //불필요한 스페이스바를 제거한 str값을 받아와 tokens에 복사함
		}
	}

	if(is_typeStatement(tokens[0]) == 2 || strstr(tokens[0], "extern") != NULL){//토큰이 extern이거나 데이터타입리스트에 해당하면
		for(i = 1; i < TOKEN_CNT; i++){//토큰 개수만큼 돌면서 
			if(strcmp(tokens[i],"") == 0)  //아무 정보도 얻지 못했다면 break함
				break;		       

			if(i != TOKEN_CNT -1 )//i가 토큰의 마지막이 아니라면
				strcat(tokens[0], " ");//맨 처음토큰에 공백을 추가하고
			strcat(tokens[0], tokens[i]);//첫토큰의 맨 앞에 i번째 토큰을 붙이고
			memset(tokens[i], 0, sizeof(tokens[i]));//i번째 토큰의 메모리를 아예 초기화한다
		}
	}
	
	
	while((p_str = find_typeSpecifier(tokens)) != -1){ //형변환 연산자인 경우 그에 해당하는 토큰넘버를 받아들이고 토큰재정렬
		if(!reset_tokens(p_str, tokens))//재정렬하는과정에서 문법적 오류 발생시 false
			return false;
	}

	
	while((p_str = find_typeSpecifier2(tokens)) != -1){  //struct구조체인 경우 그에 해당하는 토큰넘버를 받아들이고 토큰 재정렬
		if(!reset_tokens(p_str, tokens))//재정렬과정에서 문법적 오류 발생시 false
			return false;
	}
	
	return true;
}

node *make_tree(node *root, char (*tokens)[MINLEN], int *idx, int parentheses)//토큰화 한 걸 트리로 만들어주는 함수(이유 : 복수정답을 채점하기 위해  [ex) a<b, b>a는 같은 식]
{
	node *cur = root;
	node *new;
	node *saved_operator;
	node *operator;
	int fstart;
	int i;

	while(1)
	{
		if(strcmp(tokens[*idx], "") == 0)// 토큰이 비어있는 경우
			break;
	
		if(!strcmp(tokens[*idx], ")"))//토큰이 ')'면 부모노드로 돌아감
			return get_root(cur);

		else if(!strcmp(tokens[*idx], ","))//토큰이 ','이면 부모노드로 돌아감
			return get_root(cur);

		else if(!strcmp(tokens[*idx], "("))//토큰이 '('이면
		{
			
			if(*idx > 0 && !is_operator(tokens[*idx - 1]) && strcmp(tokens[*idx - 1], ",") != 0){//'(' 앞 토큰이 연산자,또는 ','가 아닐때
				fstart = true;

				while(1)
				{
					*idx += 1;//다음 토큰으로 이동

					if(!strcmp(tokens[*idx], ")"))//괄호가 닫히면 바로 종료 
						break;
					
					new = make_tree(NULL, tokens, idx, parentheses + 1);//자식트리 생성(괄호개수를 나타내는 변수 하나 증가)
					
					if(new != NULL){//자식 트리가 생성된 경우
						if(fstart == true){//함수라면
							cur->child_head = new;//cur의 자식을 new로 지정
							new->parent = cur;//new의 부모를 cur로 지정
	
							fstart = false;
						}
						else{//함수가 아니라면
							cur->next = new;//다음노드에 new트리 연결
							new->prev = cur;//이전 노드를 현재 노드에 연결
						}

						cur = new;//새로 만든 트리로 이동
					}

					if(!strcmp(tokens[*idx], ")"))//괄호가 닫히면 종료
						break;
				}
			}
			else{//이전 토큰이 부호일때
				*idx += 1;//다음토큰으로 이동
	
				new = make_tree(NULL, tokens, idx, parentheses + 1);//새로운 자식트리 생성

				if(cur == NULL)//최상위부모노드인 경우 자식 트리로 노드이동
					cur = new;

				else if(!strcmp(new->name, cur->name)){//자식과 현재의 토큰명이 같다면
					if(!strcmp(new->name, "|") || !strcmp(new->name, "||")
						|| !strcmp(new->name, "&") || !strcmp(new->name, "&&"))
					{//노드의 부호가 다음과 같다면 (|,||,&&,&)/논리연산자
						cur = get_last_child(cur);//마지막 자식노드로 이동
						if(new->child_head != NULL){//자식노드의 트리가 있다면
							new = new->child_head;
							new->parent->child_head = NULL;
							new->parent = NULL;
							new->prev = cur;
							cur->next = new;//자식관계를 형제관계로 바꿈
						}
					}
					else if(!strcmp(new->name, "+") || !strcmp(new->name, "*"))//+나 *라면 
					{
						i = 0;

						while(1)
						{
							if(!strcmp(tokens[*idx + i], ""))//토큰이 비어있다면 break
								break;

							if(is_operator(tokens[*idx + i]) && strcmp(tokens[*idx + i], ")") != 0)//다음토큰이 연산자이거나 )이 아니라면 break;
								break;

							i++;
						}
						
						if(get_precedence(tokens[*idx + i]) < get_precedence(new->name))//새 연산자가 연산자 우선순위가 더 높은 경우
						{
							cur = get_last_child(cur);//기존연산자를 막내 노드로 이동
							cur->next = new;//막내 노드를 new로 지정
							new->prev = cur;
							cur = new;
						}
						else//기존 연산자가 우선순위가 더 높은 경우
						{
							cur = get_last_child(cur);//막내자식으로 이동

							if(new->child_head != NULL){//새 연산자의 자식노드를 기존 연산자의 형제노드로 이동(그 다음 형제)
								new = new->child_head;

								new->parent->child_head = NULL;
								new->parent = NULL;
								new->prev = cur;
								cur->next = new;
							}
						}
					}
					else{//그 외의 경우
						cur = get_last_child(cur);//현재 연산자를 가장 막내인 노드로 이동 
						cur->next = new;//new를 막내노드로 지정함
						new->prev = cur;
						cur = new;
					}
				}
	
				else//그외의 경우
				{
					cur = get_last_child(cur);//막내 노드로 이동하고

					cur->next = new;//new를 막내에 지정한다
					new->prev = cur;
	
					cur = new;
				}
			}
		}
		else if(is_operator(tokens[*idx]))//해당 토큰이 연산자인경우
		{
			if(!strcmp(tokens[*idx], "||") || !strcmp(tokens[*idx], "&&")
					|| !strcmp(tokens[*idx], "|") || !strcmp(tokens[*idx], "&") 
					|| !strcmp(tokens[*idx], "+") || !strcmp(tokens[*idx], "*"))
			{//그중에서도 다음과 같은 연산자인경우
				if(is_operator(cur->name) == true && !strcmp(cur->name, tokens[*idx]))//현재의 토큰과 다음 토큰의 연산자가 같은경우
					operator = cur;//현재 노드를 연산자로 지정
		
				else
				{
					new = create_node(tokens[*idx], parentheses);//새로운 노드 생성
					operator = get_most_high_precedence_node(cur, new);//새 연산자와 기존 최고 우선순위를 가진 연산자의 우선순위를 비교

					if(operator->parent == NULL && operator->prev == NULL){//더 높은 연산자 노드가 부모가 없고 형제도 없다면

						if(get_precedence(operator->name) < get_precedence(new->name)){//최고우선순위연산자보다 새연산자가 우선순위가 더 높을때 새 연산자를 부모로 둔다
							cur = insert_node(operator, new);
						}

						else if(get_precedence(operator->name) > get_precedence(new->name))//최고우선순위 연산자보다 새연산자가 더 낮다면 새 연산자가 부모가 된다
						{
							if(operator->child_head != NULL){//연산자한테 자식이 있다면 새 연산자가 막내가 된다.
								operator = get_last_child(operator);
								cur = insert_node(operator, new);
							}
						}
						else//그외의경우
						{
							operator = cur;//최우선연산자를 현재의연산자로 지정
	
							while(1)
							{
								if(is_operator(operator->name) == true && !strcmp(operator->name, tokens[*idx]))
									break;
						
								if(operator->prev != NULL)//기존연산자의 형노드가 있으면 형으로 이동
									operator = operator->prev;
								else//없으면 break
									break;
							}

							if(strcmp(operator->name, tokens[*idx]) != 0)//기존연산자와 토큰이 다른경우 부모로 이동
								operator = operator->parent;

							if(operator != NULL){//연산자노드가 있다면 기존토큰과 같을때 현재노드를 연산자 노드로 지정
								if(!strcmp(operator->name, tokens[*idx]))
									cur = operator;
							}
						}
					}

					else
						cur = insert_node(operator, new);
				}

			}
			else//그 외의 연산자인 경우
			{
				new = create_node(tokens[*idx], parentheses);//그 토큰의 노드 생성

				if(cur == NULL)//현재노드가 없으면 생성한 노드에 대입
					cur = new;

				else//현재노드 있으면
				{
					operator = get_most_high_precedence_node(cur, new);//연산자우선순위비교 후 operator에 더 큰노드할당

					if(operator->parentheses > new->parentheses)//괄호의 수 비교했을때 기존이 더 많다면 새 연산자를 앞에 둔다
						cur = insert_node(operator, new);

					else if(operator->parent == NULL && operator->prev ==  NULL){//최상위노드일경우
					
						if(get_precedence(operator->name) > get_precedence(new->name))//기존연산자가 더 높을때
						{
							if(operator->child_head != NULL){//자식이 있을때	
								operator = get_last_child(operator);//막내의 형인 노드로 만들어준다
								cur = insert_node(operator, new);
							}
						}
					
						else	//새연산자의 우선순위가 더 높을때
							cur = insert_node(operator, new);
					}
	
					else//자식,형제가 있는 경우
						cur = insert_node(operator, new);
				}
			}
		}
		else //토큰이 문자열이라면
		{
			new = create_node(tokens[*idx], parentheses);//새 노드 생성

			if(cur == NULL)//기존노드가 없다면 새 생성노드를 cur로 지정
				cur = new;

			else if(cur->child_head == NULL){//자식노드가 없다면 자식노드로 지정
				cur->child_head = new;
				new->parent = cur;

				cur = new;
			}
			else{//자식이 있는 형태라면

				cur = get_last_child(cur);//자식중 막내로 이동

				cur->next = new;
				new->prev = cur;

				cur = new;
			}
		}

		*idx += 1;//토큰인덱스 증가
	}

	return get_root(cur);//트리의 루트노드 반환
}

node *change_sibling(node *parent)//형제를 바꿔주는 함수(동일한 연산식을 같은 답으로 처리하기 위해서임)
{
	node *tmp;
	
	tmp = parent->child_head;

	parent->child_head = parent->child_head->next;
	parent->child_head->parent = parent;
	parent->child_head->prev = NULL;

	parent->child_head->next = tmp;
	parent->child_head->next->prev = parent->child_head;
	parent->child_head->next->next = NULL;
	parent->child_head->next->parent = NULL;		

	return parent;
}

node *create_node(char *name, int parentheses)//새로운 노드를 생성하는 함수
{
	node *new;

	new = (node *)malloc(sizeof(node));
	new->name = (char *)malloc(sizeof(char) * (strlen(name) + 1));
	strcpy(new->name, name);//기존의 이름을 new노드에 있는 이름에 복사

	new->parentheses = parentheses;//기존 괄호 수 복사
	new->parent = NULL;//단순 생성이므로 아직까진 노드 구조체에 할당된 값이 없음
	new->child_head = NULL;
	new->prev = NULL;
	new->next = NULL;

	return new;
}

int get_precedence(char *op)//연산자의 우선순위 구함
{
	int i;

	for(i = 2; i < OPERATOR_CNT; i++){
		if(!strcmp(operators[i].operator, op))//op가 연산자구조체에 들어있는지 확인
			return operators[i].precedence;//저장한 값을 출력
	}
	return false;
}

int is_operator(char *op)//연산자가 있는지 확인
{
	int i;

	for(i = 0; i < OPERATOR_CNT; i++)
	{
		if(operators[i].operator == NULL)
			break;
		if(!strcmp(operators[i].operator, op)){//오퍼레이터 구조체에 op의 값이 들어있으면 true리턴
			return true;
		}
	}

	return false;//없으면 false
}

void print(node *cur)//트리를 출력하는 함수
{
	if(cur->child_head != NULL){//자식이 있다면 자식헤드를 출력하고 띄어쓰기
		print(cur->child_head);
		printf("\n");
	}

	if(cur->next != NULL){//그다음 형제노드가 있다면 탭을 이용하고 출력
		print(cur->next);
		printf("\t");
	}
	printf("%s", cur->name);//토큰 출력
}

node *get_operator(node *cur)//트리의 부모를 구하는 함수
{
	if(cur == NULL)
		return cur;

	if(cur->prev != NULL)
		while(cur->prev != NULL)
			cur = cur->prev;

	return cur->parent;
}

node *get_root(node *cur)//토큰의 루트를 찾는 함수
{
	if(cur == NULL)//더이상 이전값이 없으므로 이게 답임
		return cur;

	while(cur->prev != NULL)//이전값이 있으면 이전으로 감
		cur = cur->prev;

	if(cur->parent != NULL)//부모가 있으면 부모로 감(재귀)
		cur = get_root(cur->parent);

	return cur;
}

node *get_high_precedence_node(node *cur, node *new)//더 높은 우선순위를 가지는 노드를 구함
{
	if(is_operator(cur->name))//cur노드가 연산자인 경우
		if(get_precedence(cur->name) < get_precedence(new->name))//new노드의 우선순위가 더 높다면 cur리턴
			return cur;

	if(cur->prev != NULL){//cur에서 더 높은 형제노드가 있다면 이동(첫째)
		while(cur->prev != NULL){
			cur = cur->prev;
			
			return get_high_precedence_node(cur, new);//첫째와 new를 비교함(재귀)
		}


		if(cur->parent != NULL)//새로만든 연산자가 더 높은 우선순위를 지닌다면
			return get_high_precedence_node(cur->parent, new);//부모와 비교(재귀)
	}

	if(cur->parent == NULL)//부모가 없다면(루트라면) cur리턴
		return cur;
}

node *get_most_high_precedence_node(node *cur, node *new)//가장 우선순위가 높은 노드를 구함
{
	node *operator = get_high_precedence_node(cur, new);
	node *saved_operator = operator;

	while(1)
	{
		if(saved_operator->parent == NULL)
			break;

		if(saved_operator->prev != NULL)
			operator = get_high_precedence_node(saved_operator->prev, new);

		else if(saved_operator->parent != NULL)
			operator = get_high_precedence_node(saved_operator->parent, new);

		saved_operator = operator;
	}
	
	return saved_operator;
}

node *insert_node(node *old, node *new)//노드를 삽입하는 함수
{
	if(old->prev != NULL){//기존노드의 형이 있다면
		new->prev = old->prev;//new의 형노드로 변경
		old->prev->next = new;//new를 old의 다음형제로 변경
		old->prev = NULL;
	}

	new->child_head = old;//새로만든 노드의 자식을 기존노드로 바꿈
	old->parent = new;//새 노드를 기존노드의 부모에 대입

	return new;
}

node *get_last_child(node *cur)//가장 작은 새끼를 리턴하는 함수
{
	if(cur->child_head != NULL)//자식이 있다면 그 자식노드로 이동
		cur = cur->child_head;

	while(cur->next != NULL)//마지막 형제로 이동
		cur = cur->next;

	return cur;//자식노드리턴
}

int get_sibling_cnt(node *cur)//형제의 수를 얻어오는 함수
{
	int i = 0;

	while(cur->prev != NULL)//이전 형제노드로 끝까지 이동
		cur = cur->prev;

	while(cur->next != NULL){//마지막 형제노드로 이동하면서 i값(형제 수를 담아두는 변수)증가시킴
		cur = cur->next;
		i++;
	}

	return i;
}

void free_node(node *cur)//노드해제함수
{
	if(cur->child_head != NULL)//자식이 있다면 자식노드를 해제
		free_node(cur->child_head);

	if(cur->next != NULL)//형
		free_node(cur->next);

	if(cur != NULL){
		cur->prev = NULL;
		cur->next = NULL;
		cur->parent = NULL;
		cur->child_head = NULL;
		free(cur);
	}
}


int is_character(char c)//char형인지 체크하는 함수(아니면 0, 맞으면 1 리턴)
{
	return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

int is_typeStatement(char *str)//답안에 타입정의문이 있는지 확인
{ 
	char *start;
	char str2[BUFLEN] = {0}; 
	char tmp[BUFLEN] = {0}; 
	char tmp2[BUFLEN] = {0}; 
	int i;	 
	
	start = str;//start가 배열의 시작주소를 가리키게 함
	strncpy(str2,str,strlen(str));//str2에 답안배열을 임시저장
	remove_space(str2);//복사한 배열의 불필요한 공백 제거

	while(start[0] == ' ')//시작값에 아무것도 없으면 start에 1을 더함(시작 스페이스 무시)
		start += 1;

	if(strstr(str2, "gcc") != NULL)//문장에 gcc가 들어있다면
	{
		strncpy(tmp2, start, strlen("gcc"));//tmp2 문자열에 start문자열의 세 글자를 복사함(gcc가 처음에 들어가는지 아닌지 확인하기 위함임)
		if(strcmp(tmp2,"gcc") != 0)//gcc가 아닌경우 0 리턴
			return 0;
		else//tmp2의 내용이 'gcc'인경우 2를 리턴
			return 2;
	}
	
	for(i = 0; i < DATATYPE_SIZE; i++)//타입수만큼 반복
	{
		if(strstr(str2,datatype[i]) != NULL)//str2에 데이터타입 리스트 중 해당하는 문자열이 있다면
		{	
			strncpy(tmp, str2, strlen(datatype[i]));//tmp배열에 그 값을 저장
			strncpy(tmp2, start, strlen(datatype[i]));//tmp2배열엔 str에 처음 들어가는 데이터타입을 저장
			
			if(strcmp(tmp, datatype[i]) == 0)//tmp값과 데이터타입의 값이 일치하다면
				if(strcmp(tmp, tmp2) != 0)//tmp와 tmp2가 다르면 0,같으면 2를 리턴함.
					return 0;  
				else
					return 2;
		}

	}
	return 1;

}

int find_typeSpecifier(char tokens[TOKEN_CNT][MINLEN]) //형변환을 확인하고 그에 해당하는 토큰넘버를 반환하는 함수
{
	int i, j;

	for(i = 0; i < TOKEN_CNT; i++)//datatype을 찾음
	{
		for(j = 0; j < DATATYPE_SIZE; j++)
		{
			if(strstr(tokens[i], datatype[j]) != NULL && i > 0)
			{
				if(!strcmp(tokens[i - 1], "(") && !strcmp(tokens[i + 1], ")") 
						&& (tokens[i + 2][0] == '&' || tokens[i + 2][0] == '*' 
							|| tokens[i + 2][0] == ')' || tokens[i + 2][0] == '(' 
							|| tokens[i + 2][0] == '-' || tokens[i + 2][0] == '+' 
							|| is_character(tokens[i + 2][0])))//datatype를 괄호가 감싸고 있고 그 뒤에 부호나 이름이 나온다면 해당하는 토큰넘버를 반환  
					return i;
			}
		}
	}
	return -1;//형변환이 존재하지 않는다면 -1 리턴
}

int find_typeSpecifier2(char tokens[TOKEN_CNT][MINLEN])//struct구조체가 나오면 그에 해당하는 토큰넘버를 반환하는 함수 
{
    int i, j;

   
    for(i = 0; i < TOKEN_CNT; i++)
    {
        for(j = 0; j < DATATYPE_SIZE; j++)
        {
            if(!strcmp(tokens[i], "struct") && (i+1) <= TOKEN_CNT && is_character(tokens[i + 1][strlen(tokens[i + 1]) - 1]))//구조체 + 이름의 형태인경우  
                    return i;//그에 해당하는 토큰넘버를 반환
        }
    }
    return -1;
}

int all_star(char *str)//문자열에 *이 있는지 확인하는 함수
{
	int i;
	int length= strlen(str);//문자열의 길이를 받아와서
	
 	if(length == 0)	
		return 0;
	
	for(i = 0; i < length; i++)//그 길이만큼 i값을 증가시키며 *이 들어가는지 확인
		if(str[i] != '*')
			return 0;//없으면 0 있으면 1
	return 1;

}

int all_character(char *str)//받아온 문자열 중 알파벳이나 숫자가 들어있다면 1을 리턴함
{
	int i;

	for(i = 0; i < strlen(str); i++)//str의 길이만큼 증가시키면서 char형의 여부를 확인
		if(is_character(str[i]))
			return 1;
	return 0;
	
}

int reset_tokens(int start, char tokens[TOKEN_CNT][MINLEN])//형변환, struct와 같은 토큰을 읽었을 때 문법을 확인하고 그 결과를 리턴해주는 함수(start : 형변환 or struct가 위치한 토큰넘버)
{
	int i;
	int j = start - 1;//포인터의 역할을 수행(괄호 체크)
	int lcount = 0, rcount = 0;
	int sub_lcount = 0, sub_rcount = 0;

	if(start > -1){
		if(!strcmp(tokens[start], "struct")) {//불러온 토큰이 'struct'이라면
			strcat(tokens[start], " ");//struct +' '+ 그 다음 토큰까지를 하나의 토큰으로 합침
			strcat(tokens[start], tokens[start+1]);

			for(i = start + 1; i < TOKEN_CNT - 1; i++){//토큰배열 재정리
				strcpy(tokens[i], tokens[i + 1]);
				memset(tokens[i + 1], 0, sizeof(tokens[0]));
			}
		}

		else if(!strcmp(tokens[start], "unsigned") && strcmp(tokens[start+1], ")") != 0) {//토큰이 unsigned이고 그 뒤에 ')'이 온다면		
			strcat(tokens[start], " ");//unsigned + ' '+그다음 토큰 두개까지를 하나의 토큰으로 합침(ex>unsigned char i 까지를 하나의 토큰으로 취급)
			strcat(tokens[start], tokens[start + 1]);	     
			strcat(tokens[start], tokens[start + 2]);

			for(i = start + 1; i < TOKEN_CNT - 1; i++){//토큰배열재정리
				strcpy(tokens[i], tokens[i + 1]);
				memset(tokens[i + 1], 0, sizeof(tokens[0]));
			}
		}

     		j = start + 1;           
        	while(!strcmp(tokens[j], ")")){//')'이 나온 횟수를 측정함
                	rcount ++;
                	if(j==TOKEN_CNT)//맨 뒤까지 가버리면 멈춤
                        	break;
                	j++;
        	}
	
		j = start - 1;
		while(!strcmp(tokens[j], "(")){//토큰 앞에 '('이 나온 횟수를 측정함
        	        lcount ++;
                	if(j == 0)//맨 앞으로까지 가버리면 멈춤
                        	break;
               		j--;
		}
		if( (j!=0 && is_character(tokens[j][strlen(tokens[j])-1]) ) || j==0)//(의 왼쪽 끝의 왼쪽에 char형 변수가 있거나 아무것도 없다면
			lcount = rcount;//(,)개수가 같다고 취급

		if(lcount != rcount )//(,)의 개수가 다른경우
			return false;

		if( (start - lcount) >0 && !strcmp(tokens[start - lcount - 1], "sizeof")){//토큰을 싸고있는 (의 바로 왼쪽에 sizeof가 있는 경우 ex>sizeof(((int)))
			return true; 
		}
		
		else if((!strcmp(tokens[start], "unsigned") || !strcmp(tokens[start], "struct")) && strcmp(tokens[start+1], ")")) {//unsigned나 struct의 바로 뒤에 )가 나온 경우	
			strcat(tokens[start - lcount], tokens[start]);//불필요하게 씌어진 괄호들을 제거
			strcat(tokens[start - lcount], tokens[start + 1]);
			strcpy(tokens[start - lcount + 1], tokens[start + rcount]);
		 
			for(int i = start - lcount + 1; i < TOKEN_CNT - lcount -rcount; i++) {//제거한 이후의 토큰들을 정렬
				strcpy(tokens[i], tokens[i + lcount + rcount]);
				memset(tokens[i + lcount + rcount], 0, sizeof(tokens[0]));//그 이후의 토큰 초기화
			}


		}
 		else{//그 외의 경우
			if(tokens[start + 2][0] == '('){//토큰으로부터 오른쪽으로 두번째 칸에 (이 있다면
				j = start + 2;
				while(!strcmp(tokens[j], "(")){//그 뒤의 (이 끝날때까지 sub_lcount 증가시킴
					sub_lcount++;
					j++;
				} 	
				if(!strcmp(tokens[j + 1],")")){//(이 끝나는 바로 그 다음 토큰이 )이라면 
					j = j + 1;
					while(!strcmp(tokens[j], ")")){//)이 끝날때까지 sub_rcount증가
						sub_rcount++;
						j++;
					}
				}
				else 
					return false;

				if(sub_lcount != sub_rcount)//괄호의 개수가 다른경우
					return false;
				
				strcpy(tokens[start + 2], tokens[start + 2 + sub_lcount]);//start+2 이후에 들어간 (를 제거 	
				for(int i = start + 3; i<TOKEN_CNT; i++)
					memset(tokens[i], 0, sizeof(tokens[0]));//그 이후의 토큰은 의미없으므로 초기화

			}
			strcat(tokens[start - lcount], tokens[start]);//괄호를 제거하고
			strcat(tokens[start - lcount], tokens[start + 1]);
			strcat(tokens[start - lcount], tokens[start + rcount + 1]);
		 
			for(int i = start - lcount + 1; i < TOKEN_CNT - lcount -rcount -1; i++) {//토큰 재정리
				strcpy(tokens[i], tokens[i + lcount + rcount +1]);
				memset(tokens[i + lcount + rcount + 1], 0, sizeof(tokens[0]));//그 이후의 토큰 초기화

			}
		}
	}
	return true;
}

void clear_tokens(char tokens[TOKEN_CNT][MINLEN])//토큰초기화
{
	int i;

	for(i = 0; i < TOKEN_CNT; i++)
		memset(tokens[i], 0, sizeof(tokens[i]));//토큰이 들어있던 사이즈만큼 0으로 초기화함
}

char *rtrim(char *_str)//문자열의 오른쪽 공백 제거
{
	char tmp[BUFLEN];
	char *end;

	strcpy(tmp, _str);
	end = tmp + strlen(tmp) - 1;
	while(end != _str && isspace(*end))
		--end;

	*(end + 1) = '\0';
	_str = tmp;
	return _str;
}

char *ltrim(char *_str)//문자열 왼쪽 공백 제거
{
	char *start = _str;

	while(*start != '\0' && isspace(*start))
		++start;
	_str = start;
	return _str;
}

char* remove_extraspace(char *str)
{
	int i;
	char *str2 = (char*)malloc(sizeof(char) * BUFLEN);
	char *start, *end;
	char temp[BUFLEN] = "";
	int position;

	if(strstr(str,"include<")!=NULL){//학생 답안파일에 include<가 있는지 확인
		start = str;//시작위치 start에 저장
		end = strpbrk(str, "<");//<부터의 위치 저장
		position = end - start;
	
		strncat(temp, str, position);//str에 #include저장
		strncat(temp, " ", 1);//뒤에 " "추가
		strncat(temp, str + position, strlen(str) - position + 1);//그 뒤의 내용 추가(#include와 <내용>사이에 공백을 주기 위해서임)

		str = temp;		
	}
	
	for(i = 0; i < strlen(str); i++)
	{
		if(str[i] ==' ')//스페이스바가 들어있다면
		{
			if(i == 0 && str[0] ==' ')//처음부터 스페이스바가 있다면 i값을 증가함(제거하기위해)
				while(str[i + 1] == ' ')
					i++;	
			else{
				if(i > 0 && str[i - 1] != ' ')//스페이스바가 연속이 아니라면 str2에 집어넣음
					str2[strlen(str2)] = str[i];
				while(str[i + 1] == ' ')//연속이라면 i값 증가(그 값 제거)
					i++;
			} 
		}
		else//스페이스바 없으면 그대로 대입
			str2[strlen(str2)] = str[i];
	}

	return str2;//불필요한 스페이스바가 제거된 학생답안내용 리턴
}



void remove_space(char *str)//공백을 제거하는 함수
{
	char* i = str;
	char* j = str;
	
	while(*j != 0)
	{
		*i = *j++;
		if(*i != ' ')
			i++;
	}
	*i = 0;
}

int check_brackets(char *str)
{
	char *start = str;
	int lcount = 0, rcount = 0;
	
	while(1){
		if((start = strpbrk(start, "()")) != NULL){//만약 str값 중에서 ()중 하나의 문자라도 나오면 그 이후의 값을 리턴함.
			if(*(start) == '(')//(,)의 값을 각각 센다
				lcount++;
			else
				rcount++;

			start += 1; 		
		}
		else
			break;
	}

	if(lcount != rcount)//(,)의 개수가 각각 다르면 0, 맞으면 1을 리턴함
		return 0;
	else 
		return 1;
}

int get_token_cnt(char tokens[TOKEN_CNT][MINLEN])//토큰의 수를 리턴하는 함수
{
	int i;
	
	for(i = 0; i < TOKEN_CNT; i++)//더이상 읽어들일 값이 없을때까지 i값을 증가시키고 리턴
		if(!strcmp(tokens[i], ""))
			break;

	return i;
}
