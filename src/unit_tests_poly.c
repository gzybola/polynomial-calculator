#include <stdio.h>
#include <stdarg.h>
#include <setjmp.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "cmocka.h"
#define UTILS_H
#define MAX_INT_LENGTH 40
#include "poly.h"
/**
 *Pomocniczy bufor dla fprintf i printf
 */
static char fprintf_buffer[1000];
static char printf_buffer[10000];
static int fprintf_position = 0;
static int printf_position = 0;

/**
 *  Pomocniczy bufor, z którego korzystają atrapy funkcji operujących na stdin.
 **/
static char input_stream_buffer[1000];
static int input_stream_position = 0;
static int input_stream_end = 0;
int read_char_count = 0;

extern int calc_poly_main();

/**
 * Funkcja wołana przed każdym testem korzystającym z stdout lub stderr.
 **/
static int test_setup(void **state) {
    (void)state;

    memset(fprintf_buffer, 0, sizeof(fprintf_buffer));
    memset(printf_buffer, 0, sizeof(printf_buffer));
    printf_position = 0;
    fprintf_position = 0;

    /* Zwrócenie zera oznacza sukces. */
    return 0;
}

int mock_fprintf(FILE* const file, const char *format, ...) {
	int return_value;
	va_list args;

	assert_true(file == stderr);
	/* Poniższa asercja sprawdza też, czy fprintf_position jest nieujemne.
	 *     W buforze musi zmieścić się kończący bajt o wartości 0. */
	assert_true((size_t)fprintf_position < sizeof(fprintf_buffer));

	va_start(args, format);
	return_value = vsnprintf(fprintf_buffer + fprintf_position,
			sizeof(fprintf_buffer) - fprintf_position,
			format,
			args);
	va_end(args);

	fprintf_position += return_value;
	assert_true((size_t)fprintf_position < sizeof(fprintf_buffer));
	return return_value;
}

/**
 * Atrapa funkcji scanf używana do przechwycenia czytania z stdin.
 **/
int mock_scanf(const char *format, ...) {
	va_list fmt_args;
	int ret;
	va_start(fmt_args, format);
	ret = vsscanf(input_stream_buffer + input_stream_position, format, fmt_args);
	va_end(fmt_args);

	if (ret < 0) { /* ret == EOF */
		input_stream_position = input_stream_end;
	}
	else {
		assert_true(read_char_count >= 0);
		input_stream_position += read_char_count;
		if (input_stream_position > input_stream_end) {
			input_stream_position = input_stream_end;
		}
	}
	return ret;
}

/** 
 * Atrapa funkcji printf sprawdzająca poprawność wypisywania na stderr.  
 **/
int mock_printf(const char *format, ...) {
	int return_value;
	va_list args;

	/* Poniższa asercja sprawdza też, czy printf_position jest nieujemne.
	 *     W buforze musi zmieścić się kończący bajt o wartości 0. */
	assert_true((size_t)printf_position < sizeof(printf_buffer));

	va_start(args, format);
	return_value = vsnprintf(printf_buffer + printf_position,
			sizeof(printf_buffer) - printf_position,
			format,
			args);
	va_end(args);

	printf_position += return_value;
	assert_true((size_t)printf_position < sizeof(printf_buffer));
	return return_value;
}

/**
 * Funkcja inicjująca dane wejściowe dla programu korzystającego ze stdin.
 **/
static void init_input_stream(const char *str) {
	memset(input_stream_buffer, 0, sizeof(input_stream_buffer));
	input_stream_position = 0;
	input_stream_end = strlen(str);
	assert_true((size_t)input_stream_end < sizeof(input_stream_buffer));
	strcpy(input_stream_buffer, str);
}

static void test_PolyCompose(void **state) {
	(void)state;
	
	Poly p = PolyZero();
	int count = 0;
	
	Poly result = PolyCompose(&p, count, NULL);
	
	assert_true(PolyIsEq(&p, &result));
	PolyDestroy(&result);
}

static void test_PolyCompose2(void **state) {
	(void)state;
	
	Poly p = PolyZero();
	Poly x[] = {PolyFromCoeff(2)};
	int count = 1;
	
	Poly result = PolyCompose(&p, count, x);
	
	assert_true(PolyIsEq(&p, &result));
	PolyDestroy(&result);
}

static void test_PolyCompose3(void **state) {
	(void)state;

	int count = 0;
	Poly p = PolyFromCoeff(2);
	Poly result = PolyCompose(&p, count, NULL);
	
	assert_true(PolyIsEq(&p, &result));
	PolyDestroy(&result);
}

static void test_PolyCompose4(void **state) {
	(void)state;
	
	Poly x[] = {PolyFromCoeff(3)};
	int count = 1;
	Poly p = PolyFromCoeff(2);
	
	Poly result = PolyCompose(&p, count, x);
	
	assert_true(PolyIsEq(&p, &result));
	PolyDestroy(&result);
}

static void test_PolyCompose5(void **state) {
	(void)state;
	
	Poly tmp = PolyFromCoeff(3);
	Poly tmp2 = PolyFromCoeff(5);
	Mono mono[] = {MonoFromPoly(&tmp, 2), 
		MonoFromPoly(&tmp2, 3)};
	Poly p = PolyAddMonos(2, mono);
	p.coef = 5;
	int count = 0;
	
	Poly result = PolyCompose(&p, count, NULL);
	Poly result2 = PolyFromCoeff(5);
	
	assert_true(PolyIsEq(&result2, &result));
	PolyDestroy(&p);
	PolyDestroy(&result);
	PolyDestroy(&result2);
}

static void test_PolyCompose6(void ** state) {
	(void)state;
	
	Poly tmp = PolyFromCoeff(3);
	Poly tmp2 = PolyFromCoeff(5);
	Mono mono[] = {MonoFromPoly(&tmp, 2), 
		MonoFromPoly(&tmp2, 3)};
	Poly p = PolyAddMonos(2, mono);
	p.coef = 5;
	Poly x[] = {PolyFromCoeff(3)};
	int count = 1;

	Poly result = PolyCompose(&p, count, x);
	Poly result2 = PolyFromCoeff(167); 
	
	assert_true(PolyIsEq(&result2, &result));
	PolyDestroy(&p);
	PolyDestroy(&result);
	PolyDestroy(&result2);
}

static void test_PolyCompose7(void ** state) {
	(void)state;
	Poly tmp = PolyFromCoeff(3);
	Poly tmp2 = PolyFromCoeff(5);
	Poly tmp3 = PolyFromCoeff(48);
	Poly tmp4 = PolyFromCoeff(320);
	Poly tmp5 = PolyFromCoeff(4);
	Mono mono[] = {MonoFromPoly(&tmp, 2), MonoFromPoly(&tmp2, 3)};
	Mono mono2 [] = {MonoFromPoly(&tmp5, 2)};
	Mono mono3 [] = {MonoFromPoly(&tmp3, 4), MonoFromPoly(&tmp4, 6)};
	Poly p = PolyAddMonos(2, mono);
	Poly x[] = {PolyAddMonos(1, mono2)};
	int count = 1;

	Poly result = PolyCompose(&p, count, x); 
	Poly result2 = PolyAddMonos(2, mono3);
	
	assert_true(PolyIsEq(&result2, &result));
	PolyDestroy(&p);
	PolyDestroy(&x[0]);
	PolyDestroy(&result);
	PolyDestroy(&result2);
}

static void test_no_parameter(void **state) {
	(void)state;
	init_input_stream("COMPOSE\n");
	assert_int_equal(calc_poly_main(), 0);
	assert_string_equal(printf_buffer, "");
	assert_string_equal(fprintf_buffer, "ERROR 1 WRONG COUNT\n");

}

static void test_min_parameter(void **state) {
	(void)state;
	init_input_stream("(3,3)\nCOMPOSE 0\nPRINT\n");
	assert_int_equal(calc_poly_main(), 0);
	assert_string_equal(printf_buffer, "0\n");
	assert_string_equal(fprintf_buffer, "");
}

static void test_max_unsigned_parameter(void ** state) {
	(void)state;
	char in[MAX_INT_LENGTH];
       	sprintf(in, "%s%u%c", "COMPOSE ",INT_MAX, '\n');
	init_input_stream(in);
	assert_int_equal(calc_poly_main(), 0);
	assert_string_equal(printf_buffer, "");
	assert_string_equal(fprintf_buffer, "ERROR 1 STACK UNDERFLOW\n");
}


static void test_max_unsigned_plus_one_parameter(void ** state) {
	(void)state;
	char in[MAX_INT_LENGTH];
       	sprintf(in, "%s%li%c", "COMPOSE ",(long)INT_MAX + 1, '\n');
	init_input_stream(in);
	assert_int_equal(calc_poly_main(), 0);
	assert_string_equal(printf_buffer, "");
	assert_string_equal(fprintf_buffer, "ERROR 1 STACK UNDERFLOW\n");
}


static void test_much_more_than_unsigned_parameter(void ** state) {
	(void)state;
	char in[MAX_INT_LENGTH];
       	sprintf(in, "%s%li%c", "COMPOSE ", LONG_MAX, '\n');
	init_input_stream(in);
	assert_int_equal(calc_poly_main(), 0);
	assert_string_equal(printf_buffer, "");
	assert_string_equal(fprintf_buffer, "ERROR 1 WRONG COUNT\n");
}

static void test_letter_parameter(void **state) {
	(void)state;
	init_input_stream("COMPOSE asasf\n");
	assert_int_equal(calc_poly_main(), 0);
	assert_string_equal(printf_buffer, "");
	assert_string_equal(fprintf_buffer, "ERROR 1 WRONG COUNT\n");
}


static void test_numb_letter_parameter(void **state) {
	(void)state;
	init_input_stream("COMPOSE 898asasf\n");
	assert_int_equal(calc_poly_main(), 0);
	assert_string_equal(printf_buffer, "");
	assert_string_equal(fprintf_buffer, "ERROR 1 WRONG COUNT\n");
}

int main() {
	const struct CMUnitTest tests[] = {
		cmocka_unit_test(test_PolyCompose),
		cmocka_unit_test(test_PolyCompose2),
		cmocka_unit_test(test_PolyCompose3),
		cmocka_unit_test(test_PolyCompose4),
		cmocka_unit_test(test_PolyCompose5),
		cmocka_unit_test(test_PolyCompose6),
		cmocka_unit_test(test_PolyCompose7), 
		cmocka_unit_test_setup(test_no_parameter, test_setup),
		cmocka_unit_test_setup(test_min_parameter, test_setup),
		cmocka_unit_test_setup(test_max_unsigned_parameter, test_setup),
		cmocka_unit_test_setup(test_max_unsigned_plus_one_parameter, test_setup),
		cmocka_unit_test_setup(test_much_more_than_unsigned_parameter, test_setup),
		cmocka_unit_test_setup(test_letter_parameter, test_setup),
		cmocka_unit_test_setup(test_numb_letter_parameter, test_setup)

	};
	return cmocka_run_group_tests(tests, NULL, NULL);
}
