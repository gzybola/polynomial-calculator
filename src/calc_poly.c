/** @file
  Kalkulator wielomianów. 
  @author Aleksandra Grzyb 
  @copyright Uniwersytet Warszawski
  @date 2017-04-15
  */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "poly.h"
#include "utils.h"
#define MAX_COMMAND_LENGTH 9  ///<maksymalna długość komendy
#define NUM_BEG 1 ///<począktowy numner linii
#define NEW_LINE '\n' ///<nowa linia
#define PLUS '+' ///<plus
#define EMPTY_CHAR '\0' ///<pusty char
/** Przechowuje liczbową reprezentację komend*/
enum command {
	ADD = 193450094,
	AT = 5862138,
	CLONE = 210669826326,
	COMPOSE = 229419555988923,
	DEG = 193453397,
	DEG_BY = 6952134833711,
	IS_COEFF = 7571106913169155,
	IS_ZERO = 229427483033344, 
	IS_EQ = 210677210550,
	MUL = 193463731,
	NEG = 193464287,
	POP = 193466804,
	PRINT = 210685452402,
	SUB = 193470255,
	ZERO = 638475315 
};

Poly ReadPoly(int, int *, char *, bool *);
void PrintPoly (Poly *, bool);

/**
 *Struktura przechowująca listę monomianów.
 *Zbudowany na liście jednokierunkowej.
 **/
typedef struct MonoList {
	Mono value;///<jednomian
	struct MonoList *next;///<wskaźnik na następny element
} MonoList;

/**
 *Struktura reprezentująca stos.
 *Przechowuje wielkość stosu.
 **/
typedef struct Stack {
	Poly value;///<wielomian
	unsigned long size;///<rozmiar stosu
	struct Stack *pop;///<wskaźnik na poprzedni element stosu
} Stack;

/**
 *Tworzy nowy element stosu o danym numerze
 *@param[in] p : wielomian
 *@param[in] size : rozmiar stosu
 *@return nowy element stosu
 **/
Stack *NewStack(Poly p, unsigned long size) {
	Stack *s = (Stack *)malloc(sizeof(Stack));
	assert (s != NULL);
	s->size = size;
	s->value = p;
	s->pop = NULL;
	return s;
}

/**
 *Tworzy nowy element listy monomianów
 *@param[in] mono : monomian
 *@return nowy element listy
 **/
MonoList *NewMonoList(Mono mono) {
	MonoList *monoList = (MonoList *)malloc(sizeof(MonoList));
	assert(monoList != NULL);
	monoList->next = NULL;
	monoList->value = mono;
	return monoList;
}

/**
 *Niszczy listę monomianów
 *@param[in] list : lista monomianów
 **/
void DestroyMonoList(MonoList *list) {
	MonoList *tmp = list->next;
	free(list);
	if (tmp != NULL)
		DestroyMonoList(tmp);
}

/**
 *Oblicza długość listy monomianu
 *@param[in] mono : lista monomianów
 *@return rozmiar listy
 **/
unsigned SizeMonoList(MonoList *mono) {
	if (mono != NULL)
		return 1 + SizeMonoList(mono->next);
	else return 0;
}

/**
 *Dodaje do stosu nowy element
 *@param[in] s : stos do którego będzie dodany nowy element
 *@param[in] p : wielomian, który będzie dodany do stosu
 *@return obecny stos
 **/
Stack *AddStack(Stack *s, Poly p) {
	Stack *tmp = NewStack(p, s->size + 1);
	tmp->pop = s;
	return tmp;
}

/**
 *Zdejmuje element ze stosu niszczy wierzchołkowy wielomian
 *@param[in] s : stos, z którego będzie zdjęty element
 *@param[in] k : ilość elementów do zdjęcia
 *@return obecny stos
 **/
Stack *PopStack(Stack *s, int k) {
	if (s != NULL) {
		Stack *tmp  = s->pop;
		PolyDestroy(&(s->value));
		free(s);
		if (k > 1) 
			return PopStack(tmp, k - 1);
		else return tmp;
	}
	return NULL;
}

/**
 *Usuwa cały stos
 *@param[in] s : stos do usunięcia
 */
void DeleteStack(Stack *s) {
	while (s != NULL)
		s = PopStack(s, 1);
}

/**
 *Sprawdza czy liczba mieści się w longu
 *@param[in] a : liczba do sprawdzenia
 *@param[in] sgn : znak liczby
 *@return zwraca true jeśli liczba mieści się w zakresie, false w przeciwnym razie
 */

bool ValidateLONG(unsigned long a, int sgn) {
	if (sgn == -1 && a > 0) a--;
	return a <= LONG_MAX;
} 

/**
 *Sprawdza czy liczba mieści się w unsigned
 *@param[in] a : liczba do sprawdzenia
 *@param[in] sgn : znak liczby
 *@return zwraca true jeśli liczba mieści się w zakresie, false w przeciwnym razie
 */
bool ValidateUNSIGNED (unsigned long a, int sgn) {
	if (sgn == 1 || a == 0)
		return a <= UINT_MAX;
	else return false;
}

/**
 *Sprawdza czy liczba mieści się w int
 *@param[in] a : liczba do sprawdzenia
 *@param[in] sgn : znak liczby
 *@return zwraca true jeśli liczba mieści się w zakresie, false w przeciwnym razie
 */
bool ValidateINT (unsigned long a, int sgn) {
	if(sgn == -1 && a > 0) a--;
	return a <= INT_MAX;
}

/**
 *Wypisuje błąd: zły argument
 *@param[in] line : numer błednej linii 
 *@param[in] command: liczba reprezentująca metodę do wykonania
 **/
void ErrArg (int line, unsigned long command) {
	fprintf(stderr, "%s%d%s", "ERROR ", line, " WRONG");
	if (command == AT)
		fprintf(stderr, "%s\n", " VALUE");
	else if (command == DEG_BY)
		fprintf(stderr, "%s\n", " VARIABLE");
	else if (command == COMPOSE)
		fprintf(stderr, "%s\n", " COUNT");
}

/**
 *Wypisuje błąd: zła komenda
 *@param[in] line : numer błednej linii 
 **/
void ErrCommand(int line) {
	fprintf(stderr, "%s%d%s\n", "ERROR ", line, " WRONG COMMAND");
}

/**
 *Wypisuje błąd: zły wielomian
 *@param[in] number : number błędnej kolumny
 *@param[in] line : numer błednej linii 
 **/
void ErrPoly(int number, int line) {
	fprintf(stderr, "%s%d%s%d\n", "ERROR ", line, " ", number);
}

/**
 *Wypisuje błąd: za mało danych na stosie
 *@param[in] line : numer błednej linii 
 **/
void ErrOverflow(int line) {
	fprintf(stderr, "%s%d%s\n", "ERROR ", line, " STACK UNDERFLOW");
}

/**
 *Sprawdza czy znak jest literą
 *@param[in] c : znak do sprawdzenia
 *@return true jesli argument jest literą, false w przeciwnym razie
 */
bool IsLetter (char c) {
	return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z'); 
}

/**
 *Sprawdza czy znak jest liczbą
 *@param[in] c : znak do sprawdzenia
 *@return true jesli argument jest liczbą, false w przeciwnym razie
 */
bool IsNumber (char c) {
	return c >= '0' && c <= '9';
}

/**
 *Sprawdza czy znak jest liczbą ze znakiem
 *@param[in] c : znak do sprawdzenia
 *@return true jesli argument jest liczbą badź minusem, false w przeciwnym razie
 */
bool IsNumberNeg (char c) {
	return IsNumber(c) || c == '-';
}

/**
 *Wczytuje literę
 *@param[in] number : licznik kolumn
 *@param[in] c : miejsce do wczytania litery
 **/
void ReadLetter(int *number, char *c) {
	if (scanf("%c", c) > 0)
		(*number)++;
}

/**
 *Wczytuje liczbę
 *@param[in] c : miejsce do wcyztania znaku
 *@param[in] number : licznik kolumn
 *@param[in] proper : pamięta, czy liczba jest poprawna
 *@param[in] f : funkcja do walidacji liczby
 *@return wczytana liczba
 */
long ReadNumb(char *c, int *number, bool *proper, bool(*f)(unsigned long, int)) {
	unsigned long result = 0;
	long sgn = 1;
	if (*c == '-') {
		sgn = -1;
		ReadLetter(number, c);
	}
	while(IsNumber(*c) && *proper) {
		result *= 10;
		result += (unsigned long)(*c - '0');
		if ((*f)(result, sgn))
			ReadLetter(number, c);
		else *proper = false;
	}
	return sgn * result;
}

/**
 *Wczytuje jednomian
 *@param[in] line : obecna linia
 *@param[in] number : obecna kolumna
 *@param[in] c : miejsce na wczytanie znaku
 *@param[in] proper : pamięta czy wczytywanie jest poprawne
 *@return wczytany monomian
 **/
Mono ReadMono(int line, int *number, char *c, bool *proper) {
	Poly p = PolyZero();
	int resultExp = 0;
	if (*c == '(') {
		ReadLetter(number, c);
		p = ReadPoly(line, number, c, proper);
	}
	else *proper = false;
	if (*c == ',' && *proper) {
		ReadLetter(number, c);
		if (IsNumber(*c))
			resultExp = (int)ReadNumb(c, number, proper, ValidateINT);
		else *proper = false;
	}
	else *proper = false;
	if (*c == ')' && *proper) 
		ReadLetter(number, c);
	else *proper = false;
	if (PolyIsZero(&p))
		resultExp = 0;
	return MonoFromPoly(&p, resultExp);
}

/**
 *Zapisuje listę monomianów do tablicy
 *@param[in] monos : tablica monomianów, do których będą zapisane monomiany z listy
 *@param[in] list : lista monomianów 
 **/
void WriteListToMatrix(Mono monos[], MonoList *list) {
	int index = 0;
	while (list != NULL) {
		monos[index] = list->value;
		index++;
		list = list->next;
	}
}

/**
 *Wczytuje jednomiany i zwraca stowrzony z nich wielomian
 *@param[in] line : obecna linia
 *@param[in] number : obecna kolumna
 *@param[in] c : miejsce na wczytanie znaku
 *@param[in] proper : pamięta czy wczytywanie jest poprawne
 *@return stworzony z jednomianów wielomian
 **/
Poly ReadMonos (int line, int *number, char *c, bool *proper) {
	Poly p = PolyZero();
	MonoList *monoList = NewMonoList(MonoFromPoly(&p, 0));
	MonoList *first = monoList;
	do {
		monoList->next = NewMonoList(ReadMono(line, number, c, proper));
		monoList = monoList->next;
		if (*c == '+') {
			ReadLetter(number, c);
			if (*c == NEW_LINE)
				*proper = false;
		}
		else if (*c == '(')
			*proper = false;
	} while (*c == '(' && *proper);
	unsigned size = SizeMonoList(first->next);
	Mono monos[size];
	WriteListToMatrix(monos, first->next);
	DestroyMonoList(first);
	p = PolyAddMonos(size, monos);
	return p;
}

/**
 *Wczytuje wielomian 
 *@param[in] line : obecna linia
 *@param[in] number : obecna kolumna
 *@param[in] c : miejsce na wczytanie znaku
 *@param[in] proper : pamięta czy wczytywanie jest poprawne
 *@return wczytany wielomian
 **/
Poly ReadPoly(int line, int *number, char *c, bool *proper) {
	Poly result = PolyZero();
	if (IsNumberNeg(*c)) {
		result.coef = (long)ReadNumb(c, number, proper, ValidateLONG);
	}
	else if (*c == '(' && *proper) {
		result = ReadMonos(line, number, c, proper);
	}
	else *proper = false;
	return result;
}

/**
 *Drukuje monomian
 *@param[in] p : wielomian do wydrukowania
 *@param[in] e : wykładnik
 *@param[in] add : pamięta czy należy dodać plusa
 **/
void PrintMono (Poly p, poly_exp_t e, bool add) {
	if (add)
		printf("%c", PLUS);
	printf("%c", '(');
	if (PolyIsCoeff(&p))
		printf("%ld", p.coef);
	else 
		PrintPoly(&p, EMPTY_CHAR);
	printf("%c%d%c",',', e, ')');
}

/**
 *Drukuje wielomian, ktory nie jest współczynnikiem
 *@param[in] p : wielomian do wydrukowania
 *@param[in] add : pamięta czy należy dodać plusa
 **/
void PrintPoly(Poly *p, bool add) {
	List *mono = p->monos;
	if (mono != NULL && mono->value.exp == 0) {
		mono->value.p.coef += p->coef;
		p->coef = 0;
	}
	if (p->coef != 0) {
		PrintMono(PolyFromCoeff(p->coef), 0, add);
		add = true;
	}
	while (mono != NULL) {
		PrintMono(mono->value.p, mono->value.exp, add);
		mono = mono->next;
		add = true;
	}
}

/**
 *Drukuje wielomian
 *@param[in] p : wielomian do wydrukowania
 */
void Print(Poly *p) {
	if (PolyIsCoeff(p) || (p->monos->value.exp == 0 && PolyIsZero(&(p->monos->value.p))))
		printf("%ld", p->coef);
	else PrintPoly(p, false);

}

/**
 *Wczytuje nazwę komendy
 *@param[in] command : miejsce do wczytania komendy
 *@param[in] c : obecnie wczytywany znak
 *@param[in] proper : pamięta poprawność wczytywania (true-poprawnie)
 *@param[in] number : obecny numer kolumny
 */
void GetCommandName(char *command, char *c, bool *proper, int number) {
	if (number >= MAX_COMMAND_LENGTH)
		*proper = false;
	else if (*c != ' ' && *c != NEW_LINE) {
		char character[2] = {*c, EMPTY_CHAR};
		strcat(command, character);
		ReadLetter(&number, c);
		GetCommandName(command, c, proper, number);
	}
}

/**
 *Zapisuje string jako liczbę
 *@param[in] str : string do zakodowania
 *@return liczbowa reprezentacja
 */
unsigned long Hash(const char *str) {
	unsigned long hash = 5381;  
	int c;
	while ((c = *str++))
		hash = ((hash << 5) + hash) + c;
	return hash;
}

/**
 *Sprawdza, czy jest zadana ilość elementów na stosie
 *@param[in] argNumb : liczba potrzebnych argumentów
 *@param[in] stack : stos do sprawdzenia
 *@param[in] line : obecna linia
 *@return true jeśli jest odowiednia ilość elementów na stosie, false w przeciwnym razie
 */
bool CanMoveStack(unsigned argNumb, Stack *stack, int line) {
	if (stack->size < argNumb)  {
		ErrOverflow(line);
		return false;
	}
	return true;
}

/**
 *Sprawdza czy można wykonać ruch
 *@param[in] comm : komenda do wykonania
 *@param[in] stack : stos z wielomianami
 *@param[in] line : obecna linia
 *@param[in] c : obecnie wczytany znak
 *@param[in] arg : argument do funcji PolyAt
 *@param[in] arg2 : argument do funkcji PolyDegBy lub ilość argumentów w COMPOSE
 *@param[in] proper : pamięta czy wczytywanie się powiodło
 */
void CanMove(char *comm, Stack *stack, int line, char *c, long *arg, unsigned *arg2, bool *proper) {
	int number = NUM_BEG;
	unsigned argNumb = 0;
	*arg2 = 0;
	unsigned long command = Hash(comm);		
	switch (command) {
		case AT:
			if (*c == ' ') {
				ReadLetter(&number, c);
				if (IsNumberNeg(*c))
					*arg = ReadNumb(c, &number, proper, ValidateLONG);
				else *proper = false;
			}
			else *proper = false;
			argNumb = 1;
			if (!*proper)
				ErrArg(line, command);
			break;
		case ADD: case IS_EQ: case MUL: case SUB:
			argNumb = 2;
			break;
		case COMPOSE:
			if (*c == ' ') {
				ReadLetter(&number, c);
				if (IsNumber(*c)) {
					*arg2 = ReadNumb(c, &number, proper, ValidateUNSIGNED);
					argNumb = *arg2 + 1;
				}
				else *proper = false;
			}
			else *proper = false;
			if (!*proper)
				ErrArg(line, command);
			break;
		case DEG: case CLONE: case IS_COEFF: case IS_ZERO: case NEG: case POP: case PRINT:
			argNumb = 1;
			break;
		case DEG_BY:
			if (*c == ' ') {
				ReadLetter(&number, c);
				if (IsNumber(*c))  
					*arg2 = ReadNumb(c, &number, proper, ValidateUNSIGNED);
				else *proper = false;
			}
			else *proper = false;
			if (!*proper)
				ErrArg(line, command);
			argNumb = 1;
			break;
		case ZERO:
			argNumb = 0;
			break;
		default:

			ErrCommand(line);
			*proper = false;		
	}
	if (*proper && *c != NEW_LINE) {
		*proper = false;
		if (command != AT && command != DEG_BY && command != COMPOSE)
			ErrCommand(line);
		else
			ErrArg(line, command);
	}
	if (*proper)
		*proper = *proper & CanMoveStack(argNumb, stack, line);

} 

/**
 *Uzupełnia tablicę wielomianów wartościami ze stosu
 *@param[in] stack : stos
 *@param[in] count : liczba wielomianów do zdjęcia ze stosu
 *@param[in] polies : tablica wielomianów
 */
void GetPolies(Stack *stack, unsigned count, Poly polies[]) {
	Stack *tmp = stack;
	tmp = tmp->pop;
	for (unsigned i = 0 ; i < count ; i++) {
		polies[i] = tmp->value;
		tmp = tmp->pop;
	}			
}

/**
 *Wykonuje ruch
 *@param[in] comm : komenda do wykonania
 *@param[in] stack : stos wielomianów
 *@param[in] arg : argument do PolyAt
 *@param[in] arg2 : argument do PolyDegBy ilość wielomianów w COMPOSE
 */
void Move(char *comm, Stack **stack, long arg, unsigned arg2) {
	Poly result, tmp;
	Poly polies[arg2];
	unsigned long command = Hash(comm);
	switch(command) {
		case ADD:
			result = PolyAdd(&((*stack)->value), &((*stack)->pop->value));
			*stack = PopStack(*stack, 2);
			*stack = AddStack(*stack, result);
			break;
		case AT:
			result = PolyAt(&((*stack)->value), arg);		
			*stack = PopStack(*stack, 1);
			*stack = AddStack(*stack, result);		
			break;
		case CLONE:
			*stack = AddStack(*stack, PolyClone(&((*stack)->value)));
			break;
		case COMPOSE:	
			tmp = (*stack)->value;
			GetPolies(*stack, arg2, polies);
			result = PolyCompose(&tmp, arg2, polies);
			*stack = PopStack(*stack, arg2 + 1);
			*stack = AddStack(*stack, result);
			break;
		case DEG:
			printf("%d\n", PolyDeg(&((*stack)->value)));
			break;
		case DEG_BY:
			printf("%d\n", PolyDegBy(&((*stack)->value),arg2));
			break;
		case IS_COEFF:
			printf("%d\n", PolyIsCoeff(&((*stack)->value)));
			break;
		case IS_ZERO:
			printf("%d\n", PolyIsZero(&((*stack)->value)));
			break;
		case IS_EQ:
			printf("%d\n", PolyIsEq(&((*stack)->value), &((*stack)->pop->value)));
			break;
		case MUL:
			result = PolyMul(&((*stack)->value), &((*stack)->pop->value));
			*stack = PopStack(*stack, 2);
			*stack = AddStack(*stack, result);
			break;
		case NEG:
			result = PolyNeg(&((*stack)->value));
			*stack = PopStack(*stack, 1);
			*stack = AddStack(*stack, result);
			break;
		case POP:
			*stack = PopStack(*stack, 1);
			break;
		case PRINT:
			Print(&((*stack)->value));
			printf("\n");
			break;
		case SUB:
			result = PolySub(&((*stack)->value), &((*stack)->pop->value));
			*stack = PopStack(*stack, 2);
			*stack = AddStack(*stack, result);
			break;
		case ZERO:
			*stack = AddStack(*stack, PolyZero());
			break;
	}
}
//\cond
int main() {
	Init();
	char c;
	int line = NUM_BEG;
	int number = NUM_BEG;
	bool poly = false;
	bool command = false;
	Poly p;
	long arg = 0;
	unsigned arg2 = 0;
	char commandName[MAX_COMMAND_LENGTH];
	bool proper = true;
	Stack *stack = (NewStack(PolyZero(), 0));
	while(scanf("%c", &c) > 0) {
		if (IsLetter(c)) 
			command = true;
		else poly = true;	
		if (poly) {
			proper = true;
			p = ReadPoly(line, &number, &c, &proper);
			if (proper && c == NEW_LINE) 
				stack = AddStack(stack, p);
			else {
				ErrPoly(number, line);
				PolyDestroy(&p);
			}	
		}
		else if (command) {
			memset(commandName, 0, sizeof(commandName));
			proper = true;
			GetCommandName(commandName, &c, &proper, 0);
			CanMove(commandName, stack, line, &c, &arg, &arg2, &proper);
			if (proper)
				Move(commandName, &stack, arg, arg2);
		}
		while (c != NEW_LINE) {
			ReadLetter(&number, &c);
		}
		line++;
		number = NUM_BEG;
		poly = false;
		command = false;	
	}
	DeleteStack(stack);
	return 0;	
}
//\endcond
