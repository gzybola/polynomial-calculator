/** @file
  Interfejs klasy wielomianów. 
  @author Aleksandra Grzyb 
  @copyright Uniwersytet Warszawski
  @date 2017-04-15
  */
#include <stdio.h>
#include <stdlib.h>
#include "poly.h"
#include <math.h>
#include "utils.h"
/**
 * Usuwa listę jednomianów.
 * @param[in] l
 */
void PolyDestroyList (List *l) {
	if (l != NULL) {
		MonoDestroy(&(l->value));
		PolyDestroyList(l->next);
		free(l);
	}
}

void PolyDestroy (Poly *p) {
	PolyDestroyList(p->monos);
	p->monos=NULL;
	p->coef = 0;
}

/**
 * Tworzy nową listę jednomianów, alokując jeden element.
 * @return nowa lista
 */
static List * NewList(){
	List * res = (List *)malloc(sizeof(List));
	assert(res != NULL);
	res->next = NULL;
	res->value.p = PolyZero();
	res->value.exp = 0;
	return res;
}

/**
 * Kopiuje listę jednomianów.
 * @param[in] p : lista jednomianów
 * @return skopiowana lista
 */
List * ListClone(List *p) {
	if (p == NULL)
		return NULL;
	List *anc = p;
	List *result = NewList();
	List *first = result;
	while (anc != NULL) {
		result->next = NewList();
		result->next->value = MonoClone(&(anc->value));
		anc = anc->next;
		result = result->next;
	}
	result = first->next;
	free(first);
	return result;
}

Poly PolyClone(const Poly *p) {
	Poly clone;
	clone.coef = p->coef;
	clone.monos = ListClone(p->monos);
	return clone;
}

/**
 *Dodawanie  wielomianu durgiego do pierwszego
 *@param[in] p : wielomian do którego będzie dodany pierwszy
 *@param[in] q : wielomian dodawany
 */
void PolyAddTo(Poly *p, Poly *q) {
	List *monos = NewList();
	List *first = monos;
	List *listP = p->monos;
	List *listQ = q->monos;	
	p->coef = p->coef + q->coef;
	if (p->monos != NULL && p->monos->value.exp == 0) {
		p->monos->value.p.coef += p->coef;
		p->coef = 0;
	}		
	else if (q->monos != NULL && q->monos->value.exp == 0) {
		q->monos->value.p.coef += p->coef;
		p->coef = 0;
	}

	while (listP != NULL && listQ != NULL) {
		if (listP->value.exp < listQ->value.exp) {
			monos->next = listP;
			listP = listP->next;
			monos = monos->next;
		}
		else if (listP->value.exp > listQ->value.exp) {
			monos->next = listQ;
			listQ = listQ->next;
			monos = monos->next;
		}
		else {
			Poly *tmp = &(listP->value.p);
			Poly *tmp2 = &(listQ->value.p);
			PolyAddTo((tmp), (tmp2));
			if (!(PolyIsCoeff((tmp)) && listP->value.exp == 0) && !PolyIsZero((tmp))) {
				monos->next = listP;
				monos = monos->next;
				listP = listP->next;
			}
			else {
				p->coef += listP->value.p.coef; 
				List *tmpp = listP;
				listP = listP->next;
				free(tmpp);
			}
			List *tmmp = listQ;
			listQ = listQ->next;
			free(tmmp);
		}
	}
	if (listQ != NULL) 
		monos->next = listQ;
	else if (listP != NULL)
		monos->next = listP;
	else monos->next = NULL;
	p->monos = first->next;
	free(first);
}

Poly PolyAdd(const Poly *p, const Poly *q) {
	Poly a = PolyClone(p);
	Poly b = PolyClone(q);
	PolyAddTo(&a, &b);
	return a;
}

/**
 * Porównuje dwa jednomiany w pierwszej kolejności, patrząc na wykładnik, 
 * następnie sorutje je po wielomianie jednomianu, uwzględniając czy jest współczynnikiem.
 * @param[in] p1 : pierwszy monomian
 * @param[in] p2 : drugi monomian
 * @return jeśi p1 > p2 zwraca 1, p1 = p2 0, w przeciwnym razie -1
 */
int Compare (const void *p1, const void *p2) {
	if ( ((Mono *)p1)->exp < ((Mono *)p2)->exp) return -1;
	if ( ((Mono *)p1)->exp > ((Mono *)p2)->exp) return 1;
	if ( PolyIsCoeff(&(((Mono *)p1)->p))) return -1;
	if ( PolyIsCoeff(&(((Mono *)p2)->p))) return 1;	
	return 0;
}

Poly PolyAddMonos(unsigned count, const Mono mono[]) {
	Poly result = PolyZero();
	List *monosList = NewList();
	monosList->next = NewList();
	List *first = monosList;
	qsort((Mono *)mono, count, sizeof(Mono), Compare);
	poly_coeff_t anteriorExp = -1; 
	unsigned i = 0;
	bool add = false;
	while (i < count && mono[i].exp == 0 && PolyIsCoeff(&(mono[i].p))) {
		result.coef += mono[i].p.coef;
		i++;	
	}
	for (; i < count ; i++) {
		if (anteriorExp == mono[i].exp) {
			PolyAddTo(&(monosList->next->value.p), (Poly *)&(mono[i].p));
			add = true;
		}
		else {	
			if ((add && !PolyIsZero(&(monosList->next->value.p))) || !add) {	
				monosList = monosList->next;
				monosList->next = NewList();
			}
			anteriorExp = mono[i].exp;
			monosList->next->value = mono[i];
			add = false;
		}
	}
	if (add && PolyIsZero(&(monosList->next->value.p))) {
		free(monosList->next);
		monosList->next = NULL;
	}
	result.monos = first->next->next;
	free(first->next);
	free(first);
	return result;
}

/**
 * Mnoży wielomian przez niezerowy współczynnik 
 * @param[in] p : wielomian
 * @param[in] coef : niezerowy współczynnik 
 */
void MultiplyPolyByNumberNo0 (Poly *p, poly_coeff_t coef) {
	p->coef *= coef;
	List *newList = NewList();
	List *first = newList;
	List *ancillaryList = p->monos;
	while (ancillaryList != NULL) {
		MultiplyPolyByNumberNo0(&(ancillaryList->value.p), coef);
		if (!PolyIsZero(&(ancillaryList->value.p))) {
			newList->next = ancillaryList;
			newList = newList->next;
			ancillaryList = ancillaryList->next;
		}
		else {
			List *pop = ancillaryList;
			ancillaryList = ancillaryList->next;
			free(pop);
		}
	}
	p->monos = first->next;
	free(first);
}

/**
 * Mnoży wielomian przez współczynnik 
 * @param[in] p : wielomian
 * @param[in] coef : współczynnik 
 */
void MultiplyPolyByNumber (Poly *p, poly_coeff_t coef) {
	if (coef == 0) {  
		PolyDestroy(p);
		*p = PolyZero();
	}
	else {
		List *newList = NewList();
		List *first = newList;
		p->coef *= coef;
		List *ancillaryList = p->monos;
		while (ancillaryList != NULL) {
			MultiplyPolyByNumberNo0(&(ancillaryList->value.p), coef);
			if (!PolyIsZero(&(ancillaryList->value.p))) {
				newList->next = ancillaryList;
				newList = newList->next;
				ancillaryList = ancillaryList->next;
			}
			else {
				List *pop = ancillaryList;
				ancillaryList = ancillaryList->next;
				free(pop);
			}
		}
		p->monos = first->next;
		free(first);
	}
}

/**
 * Mnoży pierwszy argument przez współczynnik i dodaje do wyniku.
 * @param[in] p : wielomian
 * @param[in] coef : współczynnik
 * @param[in] result : wartość jaką chcemy dodać do wyniku
 * @return suma wyniku oraz wielomianu przemonożonego przez współczynnik
 */
void PolyMulOnlyCoef (const Poly *p, poly_coeff_t coef, Poly *result) {
	Poly ancillaryPoly = PolyClone(p);
	MultiplyPolyByNumber(&ancillaryPoly, coef);
	PolyAddTo(result, &ancillaryPoly);
}

/**
 * Mierzy długość listy.
 * @param[in] p : lista
 * @return długość listy 
 */
unsigned Length (List *p) {
	List *ancillary = p;
	unsigned i = 0;
	while (ancillary != NULL) {
		i++;
		ancillary = ancillary->next;
	}
	return i;
}

Poly PolyMul(const Poly *p, const Poly *q) {
	Poly ancillaryPoly;
	Poly result = PolyZero();
	unsigned size = Length(p->monos) * Length(q->monos);  
	Mono t[size];
	PolyMulOnlyCoef(p, q->coef, &result);
	result.coef = 0;
	PolyMulOnlyCoef(q, p->coef, &result);
	unsigned index = 0;
	List *listP = p->monos;
	while (listP != NULL) {
		List *listQ = q->monos;
		while (listQ != NULL) {
			ancillaryPoly = PolyMul(&(listP->value.p), &(listQ->value.p));
			if (!PolyIsZero(&ancillaryPoly)) 
				t[index++] = MonoFromPoly(&ancillaryPoly, listP->value.exp + listQ->value.exp);	
			listQ = listQ->next;
		}
		listP = listP->next;
	}
	ancillaryPoly = PolyAddMonos(size, t);
	PolyAddTo(&result, &ancillaryPoly);
	return result;
}

Poly PolyNeg(const Poly *p) {
	Poly result = PolyClone(p);
	MultiplyPolyByNumber(&result, -1);
	return result;
}

Poly PolySub(const Poly *p, const Poly *q) {
	Poly neg = PolyNeg(q);
	Poly result = PolyAdd(&neg, p);
	PolyDestroy(&neg);
	return result;
}

/**
 *Zwraca większy wykładnik.
 *@param[in] a pierwszy wykładnik
 *@param[in] b drugi wykładnik
 *@return większy z wykładników a b
 */
poly_exp_t Max (poly_exp_t a, poly_exp_t b) {
	return a > b ? a : b;
}

poly_exp_t PolyDegBy(const Poly *p, unsigned var_idx) {
	poly_exp_t result = 0;
	if (PolyIsZero(p)) 
		return -1;
	if (PolyIsCoeff(p)) 
		return 0;
	List *monos = p->monos;
	if (var_idx == 0) {
		while (monos->next != NULL)
			monos = monos->next;
		result = monos->value.exp;
	}
	else {
		while (monos != NULL) {
			result = Max(result, PolyDegBy(&(monos->value.p), var_idx - 1));
			monos = monos->next;
		}
	}
	return result;
}

poly_exp_t PolyDeg(const Poly *p) {
	poly_exp_t result = 0;
	if (PolyIsZero(p))
		return -1;
	if (PolyIsCoeff(p))
		return 0;
	List *list = p->monos;
	while (list != NULL) {
		result = Max(result, PolyDeg(&(list->value.p))+list->value.exp);
		list = list->next;
	}
	return result;
}

bool PolyIsEq(const Poly *p, const Poly *q) {
	if (PolyDegBy(p, 0) != PolyDegBy(q, 0))
		return false;
	if (p->coef != q->coef)
		return false;
	List *listP = p->monos;
	List *listQ = q->monos;
	bool a = true;
	while (a && listP != NULL && listQ != NULL) {
		if (listP->value.exp != listQ->value.exp)
			a = false;
		else if (!PolyIsEq(&(listP->value.p), &(listQ->value.p)))
			a = false;
		listP = listP->next;
		listQ = listQ->next;
	}
	if (listP != NULL || listQ != NULL)
		return false;
	return a;
}

/**
 * Przemnaża  współczynnik przez bazę różnicę współczynników razy
 * @param[in] x : współczynnik do przemnożenia
 * @param[in] base : przez jaką liczbę współczynnik będzie przemnażany
 * @param[in] exp_now : obecna potęga współczynnika 
 * @param[in] exp_want : porządana potęga współczynnika  
 */
void GetCoeff(poly_coeff_t *x, poly_coeff_t base, poly_exp_t *exp_now, poly_exp_t exp_want) {
	while (*exp_now < exp_want) {
		*x = *x * base;
		*exp_now = *exp_now + 1;
	}
}

Poly PolyAt(const Poly *p, poly_coeff_t x) {
	Poly result = PolyFromCoeff(p->coef);
	poly_coeff_t mul = 1;
	poly_exp_t exp_now = 0;
	List *list = p->monos;
	while (list != NULL) {
		Poly a = PolyClone(&(list->value.p));
		GetCoeff(&mul, x, &exp_now, list->value.exp);
		MultiplyPolyByNumber(&a, mul);
		if (PolyIsCoeff(&a)) { 
			result.coef += a.coef;
		}
		else { 
			Poly ancillary = result;
			result = PolyAdd(&result, &a);
			PolyDestroy(&ancillary);
			PolyDestroy(&a);
		}
		list = list->next;
	}
	return result;	
}

/**
 *Podnosi zadany wielomian do potęgi. Używa szybkiego potęgowania
 *@param[in] p : wielomiany, który będzie podniesiony do potęgi
 *@param[in] e : wykładnik
 *@return wielomian podniesiony do potęgi
 */
Poly PolyExp(const Poly *p, poly_exp_t e) {
	Poly result = PolyFromCoeff(1);
	if (e == 0)
		return result;
	if (e == 1)
		return PolyClone(p);
	Poly tmp = PolyMul(p, p);
	result = PolyExp(&tmp, e/2);
	PolyDestroy(&tmp);
	if (e % 2 == 1) {
		tmp = PolyMul(&result, p);
		PolyDestroy(&result);
		result = tmp;
	}
	return result;
}

/**
 *Podstawia za zmienną o numerze index wartość wielomianu z tablicy o podanym indeksie.
 *@param[in] p : wielomian do podmienienia (jest usuwany)
 *@param[in] count : liczba zmiennych do podstawienia
 *@param[in] x : wielomiany, które będą podstawiane w miejsca zmiennych
 *@param[in] index : indeks wielomianu do podstawienia
 *@return wielomian po podstawieniu
 */
Poly MulCompose (Poly *p, unsigned count, const Poly x[], unsigned index) {
	if (PolyIsCoeff(p))
		return *p;
	if (index >= count) {
		poly_coeff_t coef = p->coef;
		PolyDestroy(p);
		p = NULL;
		return PolyFromCoeff(coef);
	}
	Poly tmp, tmp2;
	List *monos = p->monos;
	while (monos != NULL) {
		tmp = MulCompose(&(monos->value.p), count, x, index + 1);
		monos->value.p = tmp;
		monos = monos->next;
	}
	Poly result = PolyFromCoeff(p->coef);
	monos = p->monos;
	while (monos != NULL) {
		if (!PolyIsZero(&(monos->value.p))) {
			tmp = PolyExp(&(x[index]), monos->value.exp);
			tmp2 = PolyMul(&tmp, &(monos->value.p));
			PolyAddTo(&result, &tmp2);
			PolyDestroy(&tmp);
		}
		monos = monos->next;
	}
	PolyDestroy(p);
	return result;
} 

Poly PolyCompose(const Poly *p, unsigned count, const Poly x[]) {
	Poly result = PolyClone(p);
	return MulCompose(&result, count, x, 0);
}
