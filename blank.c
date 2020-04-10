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
				compare_tree(root1->child_head, tmp, result);//root1의 자식과 tmp의 트리를 비교
			
				if(*result == true)
					break;
				else{
					if(tmp->next != NULL)
						*result = true;
					tmp = tmp->next;
				}
			}
		}
		else{
			compare_tree(root1->child_head, root2->child_head, result);
		}
	}	


	if(root1->next != NULL){

		if(get_sibling_cnt(root1) != get_sibling_cnt(root2)){
			*result = false;
			return;
		}

		if(*result == true)
		{
			tmp = get_operator(root1);
	
			if(!strcmp(tmp->name, "+") || !strcmp(tmp->name, "*")
					|| !strcmp(tmp->name, "|") || !strcmp(tmp->name, "&")
					|| !strcmp(tmp->name, "||") || !strcmp(tmp->name, "&&"))
			{	
				tmp = root2;
	
				while(tmp->prev != NULL)
					tmp = tmp->prev;

				while(tmp != NULL)
				{
					compare_tree(root1->next, tmp, result);

					if(*result == true)
						break;
					else{
						if(tmp->next != NULL)
							*result = true;
						tmp = tmp->next;
					}
				}
			}

			else
				compare_tree(root1->next, root2->next, result);
		}
	}
}

int make_tokens(char *str, char tokens[TOKEN_CNT][MINLEN])//문자열을 언어요소로 만드는 함수(str : 학생의 답안,tokens:언어요소를 저장할 토큰배열)
{
	char *start, *end;
	char tmp[BUFLEN];
	char str2[BUFLEN];
	char *op = "(),;><=!|&^/+-*\""; 
	int row = 0;
	int i;
 	int isPointer;
	int lcount, rcount;
	int p_str;
	
	clear_tokens(tokens);

	start = str;
	
	if(is_typeStatement(str) == 0)//해당하는 타입에 없는 경우
		return false;	
	
	while(1)
	{
		if((end = strpbrk(start, op)) == NULL)
			break;

		if(start == end){

			if(!strncmp(start, "--", 2) || !strncmp(start, "++", 2)){
				if(!strncmp(start, "++++", 4)||!strncmp(start,"----",4))
					return false;

				if(is_character(*ltrim(start + 2))){
					if(row > 0 && is_character(tokens[row - 1][strlen(tokens[row - 1]) - 1]))
						return false; 

					end = strpbrk(start + 2, op);
					if(end == NULL)
						end = &str[strlen(str)];
					while(start < end) {
						if(*(start - 1) == ' ' && is_character(tokens[row][strlen(tokens[row]) - 1]))
							return false;
						else if(*start != ' ')
							strncat(tokens[row], start, 1);
						start++;	
					}
				}
				
				else if(row>0 && is_character(tokens[row - 1][strlen(tokens[row - 1]) - 1])){
					if(strstr(tokens[row - 1], "++") != NULL || strstr(tokens[row - 1], "--") != NULL)	
						return false;

					memset(tmp, 0, sizeof(tmp));
					strncpy(tmp, start, 2);
					strcat(tokens[row - 1], tmp);
					start += 2;
					row--;
				}
				else{
					memset(tmp, 0, sizeof(tmp));
					strncpy(tmp, start, 2);
					strcat(tokens[row], tmp);
					start += 2;
				}
			}

			else if(!strncmp(start, "==", 2) || !strncmp(start, "!=", 2) || !strncmp(start, "<=", 2)
				|| !strncmp(start, ">=", 2) || !strncmp(start, "||", 2) || !strncmp(start, "&&", 2) 
				|| !strncmp(start, "&=", 2) || !strncmp(start, "^=", 2) || !strncmp(start, "!=", 2) 
				|| !strncmp(start, "|=", 2) || !strncmp(start, "+=", 2)	|| !strncmp(start, "-=", 2) 
				|| !strncmp(start, "*=", 2) || !strncmp(start, "/=", 2)){

				strncpy(tokens[row], start, 2);
				start += 2;
			}
			else if(!strncmp(start, "->", 2))
			{
				end = strpbrk(start + 2, op);

				if(end == NULL)
					end = &str[strlen(str)];

				while(start < end){
					if(*start != ' ')
						strncat(tokens[row - 1], start, 1);
					start++;
				}
				row--;
			}
			else if(*end == '&')
			{
				
				if(row == 0 || (strpbrk(tokens[row - 1], op) != NULL)){
					end = strpbrk(start + 1, op);
					if(end == NULL)
						end = &str[strlen(str)];
					
					strncat(tokens[row], start, 1);
					start++;

					while(start < end){
						if(*(start - 1) == ' ' && tokens[row][strlen(tokens[row]) - 1] != '&')
							return false;
						else if(*start != ' ')
							strncat(tokens[row], start, 1);
						start++;
					}
				}
				
				else{
					strncpy(tokens[row], start, 1);
					start += 1;
				}
				
			}
		  	else if(*end == '*')
			{
				isPointer=0;

				if(row > 0)
				{
					
					for(i = 0; i < DATATYPE_SIZE; i++) {
						if(strstr(tokens[row - 1], datatype[i]) != NULL){
							strcat(tokens[row - 1], "*");
							start += 1;	
							isPointer = 1;
							break;
						}
					}
					if(isPointer == 1)
						continue;
					if(*(start+1) !=0)
						end = start + 1;

					
					if(row>1 && !strcmp(tokens[row - 2], "*") && (all_star(tokens[row - 1]) == 1)){
						strncat(tokens[row - 1], start, end - start);
						row--;
					}
					
					
					else if(is_character(tokens[row - 1][strlen(tokens[row - 1]) - 1]) == 1){ 
						strncat(tokens[row], start, end - start);   
					}

					
					else if(strpbrk(tokens[row - 1], op) != NULL){		
						strncat(tokens[row] , start, end - start); 
							
					}
					else
						strncat(tokens[row], start, end - start);

					start += (end - start);
				}

			 	else if(row == 0)
				{
					if((end = strpbrk(start + 1, op)) == NULL){
						strncat(tokens[row], start, 1);
						start += 1;
					}
					else{
						while(start < end){
							if(*(start - 1) == ' ' && is_character(tokens[row][strlen(tokens[row]) - 1]))
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
			else if(*end == '(') 
			{
				lcount = 0;
				rcount = 0;
				if(row>0 && (strcmp(tokens[row - 1],"&") == 0 || strcmp(tokens[row - 1], "*") == 0)){
					while(*(end + lcount + 1) == '(')
						lcount++;
					start += lcount;

					end = strpbrk(start + 1, ")");

					if(end == NULL)
						return false;
					else{
						while(*(end + rcount +1) == ')')
							rcount++;
						end += rcount;

						if(lcount != rcount)
							return false;

						if( (row > 1 && !is_character(tokens[row - 2][strlen(tokens[row - 2]) - 1])) || row == 1){ 
							strncat(tokens[row - 1], start + 1, end - start - rcount - 1);
							row--;
							start = end + 1;
						}
						else{
							strncat(tokens[row], start, 1);
							start += 1;
						}
					}
						
				}
				else{
					strncat(tokens[row], start, 1);
					start += 1;
				}

			}
			else if(*end == '\"') 
			{
				end = strpbrk(start + 1, "\"");
				
				if(end == NULL)
					return false;

				else{
					strncat(tokens[row], start, end - start + 1);
					start = end + 1;
				}

			}

			else{
				
				if(row > 0 && !strcmp(tokens[row - 1], "++"))
					return false;

				
				if(row > 0 && !strcmp(tokens[row - 1], "--"))
					return false;
	
				strncat(tokens[row], start, 1);
				start += 1;
				
			
				if(!strcmp(tokens[row], "-") || !strcmp(tokens[row], "+") || !strcmp(tokens[row], "--") || !strcmp(tokens[row], "++")){


				
					if(row == 0)
						row--;

					
					else if(!is_character(tokens[row - 1][strlen(tokens[row - 1]) - 1])){
					
						if(strstr(tokens[row - 1], "++") == NULL && strstr(tokens[row - 1], "--") == NULL)
							row--;
					}
				}
			}
		}
		else{ 
			if(all_star(tokens[row - 1]) && row > 1 && !is_character(tokens[row - 2][strlen(tokens[row - 2]) - 1]))   
				row--;				

			if(all_star(tokens[row - 1]) && row == 1)   
				row--;	

			for(i = 0; i < end - start; i++){
				if(i > 0 && *(start + i) == '.'){
					strncat(tokens[row], start + i, 1);

					while( *(start + i +1) == ' ' && i< end - start )
						i++; 
				}
				else if(start[i] == ' '){
					while(start[i] == ' ')
						i++;
					break;
				}
				else
					strncat(tokens[row], start + i, 1);
			}

			if(start[0] == ' '){
				start += i;
				continue;
			}
			start += i;
		}
			
		strcpy(tokens[row], ltrim(rtrim(tokens[row])));

		 if(row > 0 && is_character(tokens[row][strlen(tokens[row]) - 1]) 
				&& (is_typeStatement(tokens[row - 1]) == 2 
					|| is_character(tokens[row - 1][strlen(tokens[row - 1]) - 1])
					|| tokens[row - 1][strlen(tokens[row - 1]) - 1] == '.' ) ){

			if(row > 1 && strcmp(tokens[row - 2],"(") == 0)
			{
				if(strcmp(tokens[row - 1], "struct") != 0 && strcmp(tokens[row - 1],"unsigned") != 0)
					return false;
			}
			else if(row == 1 && is_character(tokens[row][strlen(tokens[row]) - 1])) {
				if(strcmp(tokens[0], "extern") != 0 && strcmp(tokens[0], "unsigned") != 0 && is_typeStatement(tokens[0]) != 2)	
					return false;
			}
			else if(row > 1 && is_typeStatement(tokens[row - 1]) == 2){
				if(strcmp(tokens[row - 2], "unsigned") != 0 && strcmp(tokens[row - 2], "extern") != 0)
					return false;
			}
			
		}

		if((row == 0 && !strcmp(tokens[row], "gcc")) ){
			clear_tokens(tokens);
			strcpy(tokens[0], str);	
			return 1;
		} 

		row++;
	}

	if(all_star(tokens[row - 1]) && row > 1 && !is_character(tokens[row - 2][strlen(tokens[row - 2]) - 1]))  
		row--;				
	if(all_star(tokens[row - 1]) && row == 1)   
		row--;	

	for(i = 0; i < strlen(start); i++)   
	{
		if(start[i] == ' ')  
		{
			while(start[i] == ' ')
				i++;
			if(start[0]==' ') {
				start += i;
				i = 0;
			}
			else
				row++;
			
			i--;
		} 
		else
		{
			strncat(tokens[row], start + i, 1);
			if( start[i] == '.' && i<strlen(start)){
				while(start[i + 1] == ' ' && i < strlen(start))
					i++;

			}
		}
		strcpy(tokens[row], ltrim(rtrim(tokens[row])));

		if(!strcmp(tokens[row], "lpthread") && row > 0 && !strcmp(tokens[row - 1], "-")){ 
			strcat(tokens[row - 1], tokens[row]);
			memset(tokens[row], 0, sizeof(tokens[row]));
			row--;
		}
	 	else if(row > 0 && is_character(tokens[row][strlen(tokens[row]) - 1]) 
				&& (is_typeStatement(tokens[row - 1]) == 2 
					|| is_character(tokens[row - 1][strlen(tokens[row - 1]) - 1])
					|| tokens[row - 1][strlen(tokens[row - 1]) - 1] == '.') ){
			
			if(row > 1 && strcmp(tokens[row-2],"(") == 0)
			{
				if(strcmp(tokens[row-1], "struct") != 0 && strcmp(tokens[row-1], "unsigned") != 0)
					return false;
			}
			else if(row == 1 && is_character(tokens[row][strlen(tokens[row]) - 1])) {
				if(strcmp(tokens[0], "extern") != 0 && strcmp(tokens[0], "unsigned") != 0 && is_typeStatement(tokens[0]) != 2)	
					return false;
			}
			else if(row > 1 && is_typeStatement(tokens[row - 1]) == 2){
				if(strcmp(tokens[row - 2], "unsigned") != 0 && strcmp(tokens[row - 2], "extern") != 0)
					return false;
			}
		} 
	}


	if(row > 0)
	{

		
		if(strcmp(tokens[0], "#include") == 0 || strcmp(tokens[0], "include") == 0 || strcmp(tokens[0], "struct") == 0){ //토큰의 시작에 다음과 같은 토큰이 들어간다면
			clear_tokens(tokens); 
			strcpy(tokens[0], remove_extraspace(str)); //불필요한 스페이스바를 제거한 str값을 받아와 tokens에 복사함
		}
	}

	if(is_typeStatement(tokens[0]) == 2 || strstr(tokens[0], "extern") != NULL){//토큰이 extern이거나 데이터타입리스트에 해당하면
		for(i = 1; i < TOKEN_CNT; i++){
			if(strcmp(tokens[i],"") == 0)  
				break;		       
.
			if(i != TOKEN_CNT -1 )
				strcat(tokens[0], " ");
			strcat(tokens[0], tokens[i]);
			memset(tokens[i], 0, sizeof(tokens[i]));
		}
	}
	
	
	while((p_str = find_typeSpecifier(tokens)) != -1){ //형변환 연산자인 경우 그에 해당하는 토큰넘버를 받아들이고 토큰재정렬
		if(!reset_tokens(p_str, tokens))
			return false;
	}

	
	while((p_str = find_typeSpecifier2(tokens)) != -1){  //struct구조체인 경우 그에 해당하는 토큰넘버를 받아들이고 토큰 재정렬
		if(!reset_tokens(p_str, tokens))
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
		if(strcmp(tokens[*idx], "") == 0)//토큰이 비어있으면 멈춤
			break;
	
		if(!strcmp(tokens[*idx], ")"))//토큰이 ',',')'면 루트를 리턴
			return get_root(cur);

		else if(!strcmp(tokens[*idx], ","))
			return get_root(cur);

		else if(!strcmp(tokens[*idx], "("))//토큰이 '('이면
		{
			
			if(*idx > 0 && !is_operator(tokens[*idx - 1]) && strcmp(tokens[*idx - 1], ",") != 0){//이전 토큰이 연산자,또는 ','가 아닐때
				fstart = true;

				while(1)
				{
					*idx += 1;//다음 인덱스로 이동

					if(!strcmp(tokens[*idx], ")"))//괄호가 닫히면 바로 종료 
						break;
					
					new = make_tree(NULL, tokens, idx, parentheses + 1);//기존 토큰의 자식트리 생성(괄호개수를 나타내는 변수 하나 증가)
					
					if(new != NULL){//루트노드가 존재하고
						if(fstart == true){
							cur->child_head = new;//기존파일의 자식노드에 만들어둔 자식트리를 넣음
							new->parent = cur;//자식트리의 부모에 cur을 대입
	
							fstart = false;
						}
						else{
							cur->next = new;
							new->prev = cur;
						}

						cur = new;
					}

					if(!strcmp(tokens[*idx], ")"))
						break;
				}
			}
			else{//이전 토큰이 부호일때
				*idx += 1;//인덱스를 증가하고 트리 생성
	
				new = make_tree(NULL, tokens, idx, parentheses + 1);

				if(cur == NULL)//새 루트노드를 cur에 저장
					cur = new;

				else if(!strcmp(new->name, cur->name)){
					if(!strcmp(new->name, "|") || !strcmp(new->name, "||")
						|| !strcmp(new->name, "&") || !strcmp(new->name, "&&"))
					{//노드의 부호가 다음과 같다면 (|,||,&&,&)
						cur = get_last_child(cur);//자식노드를 받아옴
						if(new->child_head != NULL){//자식이 있다면
							new = new->child_head;
							new->parent->child_head = NULL;
							new->parent = NULL;
							new->prev = cur;
							cur->next = new;
						}
					}
					else if(!strcmp(new->name, "+") || !strcmp(new->name, "*"))//+나 *라면 
					{
						i = 0;

						while(1)
						{
							if(!strcmp(tokens[*idx + i], ""))
								break;

							if(is_operator(tokens[*idx + i]) && strcmp(tokens[*idx + i], ")") != 0)
								break;

							i++;
						}
						
						if(get_precedence(tokens[*idx + i]) < get_precedence(new->name))
						{
							cur = get_last_child(cur);
							cur->next = new;
							new->prev = cur;
							cur = new;
						}
						else
						{
							cur = get_last_child(cur);

							if(new->child_head != NULL){
								new = new->child_head;

								new->parent->child_head = NULL;
								new->parent = NULL;
								new->prev = cur;
								cur->next = new;
							}
						}
					}
					else{
						cur = get_last_child(cur);
						cur->next = new;
						new->prev = cur;
						cur = new;
					}
				}
	
				else
				{
					cur = get_last_child(cur);

					cur->next = new;
					new->prev = cur;
	
					cur = new;
				}
			}
		}
		else if(is_operator(tokens[*idx]))
		{
			if(!strcmp(tokens[*idx], "||") || !strcmp(tokens[*idx], "&&")
					|| !strcmp(tokens[*idx], "|") || !strcmp(tokens[*idx], "&") 
					|| !strcmp(tokens[*idx], "+") || !strcmp(tokens[*idx], "*"))
			{
				if(is_operator(cur->name) == true && !strcmp(cur->name, tokens[*idx]))
					operator = cur;
		
				else
				{
					new = create_node(tokens[*idx], parentheses);
					operator = get_most_high_precedence_node(cur, new);

					if(operator->parent == NULL && operator->prev == NULL){

						if(get_precedence(operator->name) < get_precedence(new->name)){
							cur = insert_node(operator, new);
						}

						else if(get_precedence(operator->name) > get_precedence(new->name))
						{
							if(operator->child_head != NULL){
								operator = get_last_child(operator);
								cur = insert_node(operator, new);
							}
						}
						else
						{
							operator = cur;
	
							while(1)
							{
								if(is_operator(operator->name) == true && !strcmp(operator->name, tokens[*idx]))
									break;
						
								if(operator->prev != NULL)
									operator = operator->prev;
								else
									break;
							}

							if(strcmp(operator->name, tokens[*idx]) != 0)
								operator = operator->parent;

							if(operator != NULL){
								if(!strcmp(operator->name, tokens[*idx]))
									cur = operator;
							}
						}
					}

					else
						cur = insert_node(operator, new);
				}

			}
			else
			{
				new = create_node(tokens[*idx], parentheses);

				if(cur == NULL)
					cur = new;

				else
				{
					operator = get_most_high_precedence_node(cur, new);

					if(operator->parentheses > new->parentheses)
						cur = insert_node(operator, new);

					else if(operator->parent == NULL && operator->prev ==  NULL){
					
						if(get_precedence(operator->name) > get_precedence(new->name))
						{
							if(operator->child_head != NULL){
	
								operator = get_last_child(operator);
								cur = insert_node(operator, new);
							}
						}
					
						else	
							cur = insert_node(operator, new);
					}
	
					else
						cur = insert_node(operator, new);
				}
			}
		}
		else 
		{
			new = create_node(tokens[*idx], parentheses);

			if(cur == NULL)
				cur = new;

			else if(cur->child_head == NULL){
				cur->child_head = new;
				new->parent = cur;

				cur = new;
			}
			else{

				cur = get_last_child(cur);

				cur->next = new;
				new->prev = cur;

				cur = new;
			}
		}

		*idx += 1;
	}

	return get_root(cur);
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
	if(is_operator(cur->name))
		if(get_precedence(cur->name) < get_precedence(new->name))
			return cur;

	if(cur->prev != NULL){
		while(cur->prev != NULL){//더 높은 형제노드가 있다면 거기로 이동(재귀)
			cur = cur->prev;
			
			return get_high_precedence_node(cur, new);
		}


		if(cur->parent != NULL)//부모노드가 있다면 거기로 이동
			return get_high_precedence_node(cur->parent, new);
	}

	if(cur->parent == NULL)//더이상의 부모가 없다면 그 노드를 반환
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
	if(old->prev != NULL){
		new->prev = old->prev;
		old->prev->next = new;
		old->prev = NULL;
	}

	new->child_head = old;
	old->parent = new;

	return new;
}

node *get_last_child(node *cur)//자식노드를 불러오는 함수
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
	if(cur->child_head != NULL)
		free_node(cur->child_head);

	if(cur->next != NULL)
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

int all_character(char *str)//받아온 문자열이 모두 char형인지 확인하는 함수
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
