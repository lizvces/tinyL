#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "InstrUtils.h"
#include "Utils.h"

int main()
{
	Instruction *head;

	head = ReadInstructionList(stdin);
	if (!head) {
		WARNING("No instructions\n");
		exit(EXIT_FAILURE);
	}
	Instruction *temp = head;
	//Instruction * t2 = temp;
	Instruction *tNext = head->next;
	int flag = 1;
	while(flag == 1){
		int change = 0;
		temp = head;
		tNext = head->next;
		while(temp && tNext && tNext->next){
			if(tNext && temp->opcode==LOADI && tNext->opcode == LOADI && tNext->next){
					switch(tNext->next->opcode){
						case ADD:
							change = 1;
							temp->field1 = tNext->next->field1;
							temp->field2 = temp->field2 + tNext->field2;
							break;
						case SUB:
							change = 1;
							temp->field1 = tNext->field1;
							temp->field2 = (tNext->next->field2 > tNext->next->field3) ? temp->field2 - tNext->field2 : temp->field2 - tNext->field2;
							break;
						case MUL:
							change = 1;
							temp->field1 = tNext->next->field1;
							temp->field2 = temp->field2 * tNext->field2;
							break;
						default:
							break;
					}
					if(change == 1){
						temp->next = tNext->next->next;
						tNext->next->next->prev = temp;
						free(tNext);
						free(tNext->next);
					}	
				}
			temp = temp->next;
			tNext = temp->next;
		}

		if(change == 0) flag = 0;
	}

	if (head) {
		PrintInstructionList(stdout, head);
		DestroyInstructionList(head);
	}
	return EXIT_SUCCESS;
}

