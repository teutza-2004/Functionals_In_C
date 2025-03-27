#include "functional.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

void for_each(void (*func)(void *), array_t list)
{
	for (int i = 0; i < list.len; i++)
		if (func)
			func(list.data + i * list.elem_size);
}

array_t map(void (*func)(void *, void *),
			int new_list_elem_size,
			void (*new_list_destructor)(void *),
			array_t list) {
	array_t new_list;
	new_list.elem_size = new_list_elem_size;
	new_list.len = list.len;
	new_list.destructor = new_list_destructor;
	new_list.data = calloc(new_list.len, new_list.elem_size);
	for (int i = 0; i < list.len; i++) {
		func(new_list.data + i * new_list.elem_size,
			 list.data + i * list.elem_size);
		if (list.destructor)
			list.destructor(list.data + i * list.elem_size);
	}
	free(list.data);
	return new_list;
}

array_t filter(boolean(*func)(void *), array_t list)
{
	array_t new_list;
	new_list.elem_size = list.elem_size;
	new_list.destructor = list.destructor;
	new_list.len = 0;
	new_list.data = calloc(list.len, new_list.elem_size);
	for (int i = 0; i < list.len; i++) {
		if (func(list.data + i * list.elem_size))
			memmove(new_list.data + (new_list.len++) * new_list.elem_size,
					list.data + i * list.elem_size, list.elem_size);
	}
	new_list.data = realloc(new_list.data, new_list.elem_size * new_list.len);
	free(list.data);
	return new_list;
}

void *reduce(void (*func)(void *, void *), void *acc, array_t list)
{
	for (int i = 0; i < list.len; i++)
		func(acc, list.data + i * list.elem_size);
	return acc;
}

void for_each_multiple(void(*func)(void **), int varg_c, ...)
{
	int mini = BIG_ENOUGH;
	int size = 0; // calc size-ul pt vectorul aux de mai jos
	va_list arg_list;
	array_t *list = calloc(varg_c, sizeof(array_t)); // array cu toate listele

	va_start(arg_list, varg_c);
	for (int i = 0; i < varg_c; i++) {
		array_t aux = va_arg(arg_list, array_t);
		if (aux.len < mini)
			mini = aux.len;
		size += aux.elem_size;
		list[i] = aux;
	}
	va_end(arg_list);

	// parcurg "pe coloane" listele
	for (int i = 0; i < mini; i++) {
		void **aux = calloc(size, sizeof(void *));
		for (int j = 0; j < varg_c; j++)
			aux[j] = list[j].data + i * list[j].elem_size;
		func(aux);
		free(aux);
	}
	free(list);
}

array_t map_multiple(void(*func)(void *, void **),
					 int new_list_elem_size,
					 void (*new_list_destructor)(void *),
					 int varg_c, ...)
{
	int mini = BIG_ENOUGH;
	int size = 0; // calc size-ul pt vectorul aux de mai jos
	va_list arg_list;
	array_t *list = calloc(varg_c, sizeof(array_t)); // array cu toate listele

	va_start(arg_list, varg_c);
	for (int i = 0; i < varg_c; i++) {
		array_t aux = va_arg(arg_list, array_t);
		if (aux.len < mini)
			mini = aux.len;
		size += aux.elem_size;
		list[i] = aux;
	}
	va_end(arg_list);

	array_t new_list;
	new_list.elem_size = new_list_elem_size;
	new_list.destructor = new_list_destructor;
	new_list.data = calloc(mini, new_list_elem_size);
	// parcurg "pe coloane" listele
	for (int i = 0; i < mini; i++) {
		void **aux = calloc(size, sizeof(void *));
		for (int j = 0; j < varg_c; j++)
			aux[j] = list[j].data + i * list[j].elem_size;
		func(new_list.data + i * new_list.elem_size, aux);
		free(aux);
	}
	// dezalocare liste
	for (int i = 0; i < varg_c; i++) {
		for (int j = 0; j < list[i].len; j++)
			if (list[i].destructor)
				list[i].destructor(list[i].data + j * list[i].elem_size);
		free(list[i].data);
	}
	free(list);

	return new_list;
}

void *reduce_multiple(void(*func)(void *, void **), void *acc, int varg_c, ...)
{
	int mini = BIG_ENOUGH;
	int size = 0; // calc size-ul pt vectorul aux de mai jos
	va_list arg_list;
	array_t *list = calloc(varg_c, sizeof(array_t)); // array cu toate listele

	va_start(arg_list, varg_c);
	for (int i = 0; i < varg_c; i++) {
		array_t aux = va_arg(arg_list, array_t);
		if (aux.len < mini)
			mini = aux.len;
		size += aux.elem_size;
		list[i] = aux;
	}
	va_end(arg_list);

	// parcurg "pe coloane"
	for (int i = 0; i < mini; i++) {
		void **aux = calloc(size, sizeof(void *));
		for (int j = 0; j < varg_c; j++)
			aux[j] = list[j].data + i * list[j].elem_size;
		func(acc, aux);
		free(aux);
	}

	free(list);
	return acc;
}
