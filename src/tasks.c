#include "functional.h"
#include "tasks.h"
#include "tests.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

void rev(void *acc, void *elem) {
	char **new_list = (char **)acc;
	char *s = calloc(strlen(*new_list) + 1, sizeof(char));
	strcpy(s, *new_list);
	sprintf(*new_list, "%0*d", 10, *(int *)elem);
	strncpy((*new_list) + strlen(*new_list), s, strlen(s));
	free(s);
	acc = new_list;
}

void string_to_int(void *new_elem, void *elem) {
	char *s = (char *)elem;
	*(s + 10) = '\0'; // pentru a lua cate un elem de 10 char
	int *nr = (int *)new_elem;
	*nr = atoi(s);
	*(s + 10) = '0';
	new_elem = nr;
}

array_t reverse(array_t list) {
	array_t new_list;
	new_list.data = calloc(list.len, 11);
	new_list.destructor = NULL;
	// int are max 10 cif => in string 10 poz alocate fiecarui elem
	new_list.elem_size = 10;
	new_list.len = list.len;

	reduce(rev, &new_list.data, list); // salvez elem reversed intr-un char*
	// transform char* in int-uri
	array_t new_new_list = map(string_to_int, list.elem_size,
							   list.destructor, new_list);
	return new_new_list;
}

void rational_number(void *elem, void **list) {
	number_t *nr = (number_t *)elem;
	nr->integer_part = *(int *)list[0];
	nr->fractional_part = *(int *)list[1];
	nr->string = calloc(100, sizeof(char));
	sprintf(nr->string, "%d.%d", (*nr).integer_part, (*nr).fractional_part);
	elem = nr;
}

void create_number_destructor(void *elem) {
	number_t *nr = (number_t *)elem;
	free(nr->string);
	elem = nr;
}

array_t create_number_array(array_t integer_part, array_t fractional_part) {
	return map_multiple(rational_number,
						integer_part.elem_size * 2 + 100 * sizeof(char),
						create_number_destructor,
						2, integer_part, fractional_part);
}

boolean greater5(void *elem) {
	int *nr = (int *)elem;
	return *nr > 5;
}

void only_name(void *acc, void *elem) {
	char **name = (char **)acc;
	*name = calloc(100, sizeof(char));
	student_t *student = (student_t *)elem;
	strcpy(*name, student->name);
	acc = name;
}

array_t get_passing_students_names(array_t list) {
	array_t filtered = filter(greater5, list);
	return map(only_name, 100 * sizeof(char), NULL, filtered);
}

void add(void *acc, void *elem) {
	*(int *)acc += *(int *)elem;
}

void maxim(void *elem, void **list) {
	boolean *new_list_elem = (boolean *)elem;
	int sum = 0;
	reduce(add, &sum, *(array_t *)list[1]); // calc suma elem lista curenta
	*new_list_elem = (sum >= *(int *)list[0]);
	elem = new_list_elem;
}

array_t check_bigger_sum(array_t list_list, array_t int_list) {
	return map_multiple(maxim, sizeof(boolean), NULL, 2, int_list, list_list);
}

boolean always_true(void *elem) {
	(void)elem;
	return 1;
}

void the_same(void *new_elem, void *elem) {
	char **new_elem2 = (char **)new_elem;
	*new_elem2 = calloc(100, sizeof(char));
	char **elem2 = (char **)elem;
	strcpy(*new_elem2, *elem2);
	new_elem = elem2;
}

void fancy_string_destructor(void *elem)
{
	free(*(char **)(elem + sizeof(char **)));
	free(*(char **)elem);
}

array_t get_even_indexed_strings(array_t list) {
	/*daca este un nr impar de elem, aloc unul in plus
	si il pun la finalul listei
	ca sa nu dea eroare atunci cand incerc sa dau free
	la al doilea "element" din clusterul de 2
	format din schimbarea elem_size-ului la lista parametru.
	cea de-a doua lista va avea elemente de lungimea initiala
	a listei primita ca parametru*/
	char *s = calloc(100, sizeof(char));
	if (list.len % 2) {
		list.data = realloc(list.data, (list.len + 1) * list.elem_size);
		memmove(list.data + list.len * list.elem_size, s, list.elem_size);
	}
	free(s);
	list.elem_size *= 2;
	list.len = (list.len + 1) / 2;
	list.destructor = fancy_string_destructor;
	array_t list2 = map(the_same, list.elem_size / 2, list.destructor, list);
	return list2;
}

void put_index(void *acc, void *elem) {
	int *nr = (int *)elem;
	int *index = (int *)acc;
	*nr = *index;
	(*index)++;
	acc = index;
	elem = nr;
}

void put_lists(void *acc, void *elem) {
	int *n = (int *)acc;
	array_t *list = (array_t *)elem;
	(*list).data = calloc(*n, sizeof(int));
	(*list).len = *n;
	(*list).destructor = NULL;
	(*list).elem_size = sizeof(int);
	int nr = 1;
	reduce(put_index, &nr, *list);
	elem = list;
	acc = n;
}

void add_inc(void *acc, void *elem) {
	int *nr = (int *)elem;
	int *inc = (int *)acc;
	*nr += *inc;
	acc = inc;
	elem = nr;
}

void increase(void *acc, void *elem) {
	array_t *list = (array_t *)elem;
	int *nr = (int *)acc;
	reduce(add_inc, nr, *list);
	(*nr)++;
	elem = list;
	acc = nr;
}

void matrix_destructor(void *elem) {
	free(((array_t *)elem)->data);
}

array_t generate_square_matrix(int n) {
	array_t list;
	list.data = calloc(n, sizeof(array_t));
	list.destructor = matrix_destructor;
	list.elem_size = sizeof(array_t);
	list.len = n;

	reduce(put_lists, &n, list); // pun 1 2 3... pe toate liniile
	int nr = 0;
	reduce(increase, &nr, list); // adun la fiecare linie cat trebuie
	return list;
}
