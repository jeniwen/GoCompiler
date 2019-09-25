#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include<math.h>

#define append(slice, element) _Generic((element), void*: appendv, int:appendi,float:appendf,bool:appendb, double:appendf, struct DATA:appendSD)(slice,element)

typedef enum {
	intType,
	floatType,
	runeType,
	stringType,
	boolType,
	nullType,
	structType,
	arrayType,
	sliceType,
	unknownType
} TypeKind;

struct TYPE_BASE {
	TypeKind kind;
	int size;
struct TYPE_BASE* underlying;
int length;};

typedef struct TYPE_BASE TYPE;

struct DATA {
	void* pointer;
	unsigned int capacity;
	unsigned int length;
	TYPE* underlying;
int* refs;
};

typedef struct DATA ARRAY;
typedef struct DATA SLICE;

SLICE slice_copy(SLICE slice);

char temp_stringcast[4*sizeof(char)];

char * string_cast(int num) {
		num = num % 128;
		sprintf(temp_stringcast, "%c", num);
		return temp_stringcast;
}

void* array_access(ARRAY array_name, int index){
	if(index < 0){
		fprintf(stderr, "Error: Array access out of bounds error\n");
		exit(1);
	} else if (index >= array_name.length) {
		fprintf(stderr, "Error: Array access out of bounds error\n");
		exit(1);
	} else {
		return (void*)((char*)array_name.pointer + index*array_name.underlying->size);
	}
}

void* slice_access(SLICE slice_name, int index){
	if(index < 0){
		fprintf(stderr, "Error: Slice access out of bounds error\n");
		exit(1);
	} else if (index >= slice_name.length) {
		fprintf(stderr, "Error: Slice access out of bounds error\n");
		exit(1);
	} else {
		return (slice_name.pointer + index);
	}
}

ARRAY array_copy(ARRAY array){
	ARRAY new_array;
	new_array.refs = NULL;
	new_array.underlying = array.underlying;
	new_array.capacity = array.capacity;
	new_array.length = array.length;
	new_array.pointer = malloc(array.capacity * array.underlying->size);
	if(array.underlying->kind == arrayType){
	for(int i = 0; i < array.capacity; i++){
	((ARRAY*)new_array.pointer)[i] = array_copy(((ARRAY*)array.pointer)[i]);
	}
	}else if (array.underlying->kind == sliceType){
	for(int i = 0; i < array.capacity; i++){
	((SLICE*)new_array.pointer)[i] = slice_copy(((SLICE*)array.pointer)[i]);
	}
	}else 
	{memcpy(new_array.pointer, array.pointer, array.capacity*array.underlying->size);
	}
	return new_array;}

SLICE slice_copy(SLICE slice){
	SLICE new_slice;
	new_slice.refs = slice.refs;
	new_slice.capacity = slice.capacity;
	new_slice.length = slice.length;
	new_slice.underlying = slice.underlying;
	new_slice.pointer = slice.pointer;
	return new_slice;
}


bool __golite__true = true;
bool __golite__false = false;
bool arrays_equal(ARRAY array1, ARRAY array2){
	if(array1.underlying->kind == array2.underlying->kind){
		if(array1.length == array2.length){
			bool bool_tally = true;
			switch(array1.underlying->kind){
				case arrayType:
					for(int i = 0; i < array1.length; i++){
						bool_tally &= arrays_equal(*(ARRAY*)(array1.pointer + i), *(ARRAY*)(array2.pointer + i));
						if(bool_tally == false) break;
					}
					return bool_tally;
				case sliceType:
					return false; // Slices aren't comparable
				case unknownType:
					fprintf(stderr, "Warning: (2) <THIS IS IN arrays_equal(...) THIS ERROR SHOULDN'T BE POSSIBLE, FIX IT>\n"); // This error should be dealt with by the way I imagine this all eventually being implememted. If this arises, it likely implies a communication error
					return false; // Shouln't happen, but not doing an exit, just a loud warning
				default:
					if(memcmp(array1.pointer, array2.pointer, array1.underlying->size * array1.length) == 0){
						return true;
					} else {
						return false;
					}
				}
		} else {
			fprintf(stderr, "Warning: (3) <THIS IS IN arrays_equal(...) THIS ERROR SHOULDN'T BE POSSIBLE, FIX IT>\n"); // The errors like this should've been ruled out by the typechecker
			return false; // Shouldn't happen, but not doing an exit, just a loud warning
		}
	} else {
		fprintf(stderr, "Warning: (1) <THIS IS IN arrays_equal(...) THIS ERROR SHOULDN'T BE POSSIBLE, FIX IT>\n"); // The errors like this should've been ruled out by the typechecker
		return false; // Shouldn't happen, but not doing an exit, just a loud warning
	}
}

TYPE* make_type(TypeKind kind, int size, TYPE* underlying, int capacity){
	TYPE* new_type = (TYPE*)malloc(sizeof(TYPE));
	new_type->kind = kind;
	new_type->size = size;
	TYPE* underlying_type = NULL;
	if(underlying)
	{
		underlying_type = malloc(sizeof(TYPE));
		*underlying_type = *underlying;
	}
	new_type->underlying = underlying_type;
	new_type->length = capacity;
	return new_type;
}

SLICE make_slice(TYPE* underlying){
	SLICE new_slice;
	new_slice.pointer = NULL;
	new_slice.capacity = 0;
	new_slice.refs = malloc(sizeof(int));
	*(new_slice.refs) = 1;
	new_slice.length = 0;
	new_slice.underlying = underlying;
	return new_slice;
}

ARRAY make_array(TYPE* underlying, int capacity){
	ARRAY new_array;
	new_array.pointer = malloc(underlying->size * capacity);
	if(underlying->kind == arrayType)
	{
		for(int i = 0; i < capacity; i++){
			((ARRAY*)new_array.pointer)[i] = make_array((underlying->underlying), underlying->length);
		}
	} else if (underlying->kind == sliceType){
		for(int i = 0; i < capacity; i++){
			((SLICE*)new_array.pointer)[i] = make_slice((underlying->underlying));
		}
	} else if (underlying->kind == structType) {
		for(int i = 0; i < capacity; i++) {
			//((ARRAY*)new_array.pointer)[i] = make_struct(malloc(sizeof(underlying->size)));
		}
	} else {
		memset(new_array.pointer, '\0', underlying->size * capacity);
	}
	new_array.refs = NULL;
	new_array.capacity = capacity;
	new_array.length = capacity;
	new_array.underlying = underlying;
	return new_array;
}

char * concat(char * str1, char * str2) {
char * dest = (char *) malloc(sizeof(*str1)+ sizeof(str1) + 2);
strcat(dest, str1);
strcat(dest, str2); return dest;
}

char * boolToString(int b) { if (b) return "true"; return "false";}

SLICE appendv(SLICE slice, void* element)
{
	if(slice.length == slice.capacity)
	{
		if (slice.capacity == 0)
		{
			slice.capacity = 2;
			slice.pointer = malloc(slice.underlying->size*2);
			memset(slice.pointer, '\0', slice.underlying->size*2);
		}
		else
		{
			slice.capacity *= 2;
			*(slice.refs) -= 1;
			void* temp = slice.pointer;
			slice.pointer = malloc(slice.underlying->size * slice.capacity);
			memset(slice.pointer, '\0', slice.underlying->size * slice.capacity);
			if(slice.underlying->kind == arrayType)
			{
				for(int i = 0; i < slice.length; i++)
				{
				((ARRAY*)slice.pointer)[i] = array_copy(((ARRAY*)temp)[i]);
				}
			}
			else if (slice.underlying->kind == sliceType)
			{
				for(int i = 0; i < slice.length; i++)
				{
					((SLICE*)slice.pointer)[i] = slice_copy(((SLICE*)temp)[i]);
				}
			}
			else 
			{
				memcpy(slice.pointer, temp, slice.underlying->size * slice.capacity);
			}
			int* temp2 = slice.refs;
			slice.refs = malloc(sizeof(int));				*(slice.refs) = 1;
			if(slice.pointer != NULL && *(slice.refs) == 0)
			{
				free(temp);
				free(temp2);
			}
		}
	}
	memcpy(slice.pointer + slice.length*slice.underlying->size, element, slice.underlying->size);
	slice.length += 1;
	return slice;
}

int cap(struct DATA data){
	return data.capacity;
}

int len(struct DATA data){
	return data.length;
}

ARRAY array_assign(ARRAY array1, ARRAY array2){
	if(array1.pointer != NULL)free(array1.pointer);
	array1.pointer = array2.pointer;

	array1.length = array2.length;
	array1.capacity = array2.capacity;
	return array1;
}

SLICE slice_assign(SLICE slice1, SLICE slice2){
	SLICE tmp = slice_copy(slice2);
	*(tmp.refs) += 1;
	*(slice1.refs) -= 1;

	if(*(slice1.refs) == 0)
	{
		free(slice1.refs);
		free(slice1.pointer);
	}

	slice1 = tmp;

	return slice1;
}

void print_bool(bool value){
	printf("%s", value?"true":"false");
}

void print_float(float value){
	(value == 0.0)?printf("+0.000000e+000"):printf("%+e", value);
}

char* stradd(char* str1,char* str2){
	int length1 = strlen(str1);
	int length2 = strlen(str2);
	char* new = (char*)malloc(sizeof(char)*(length1 + length2));
	for(int i = 0; i < length1; i++) new[i] = str1[i];
	for(int i = length1; i < length1 + length2; i++) new[i] = str2[i - length1];
	new[length1 + length2] = '\0';
	return new;
}

SLICE appendSD(SLICE slice, struct DATA element){	return appendv(slice, &element);}

SLICE appendi(SLICE slice, int element){	return appendv(slice, &element);}

SLICE appendf(SLICE slice, float element){return appendv(slice, &element);}

SLICE appendb(SLICE slice, bool element){return appendv(slice, &element);}

void* tmp_struct; // I was gonna use this for struct equality, but nvm

typedef struct { 
	int __golite__maxW;
	int __golite__currItems;	
} __golite_struct_1;

__golite_struct_1 * struct_copy1(__golite_struct_1 * toCopy){
	__golite_struct_1 * toRet = malloc(sizeof(__golite_struct_1));
	if (toCopy) {
		toRet->__golite__maxW = toCopy->__golite__maxW;
		toRet->__golite__currItems = toCopy->__golite__currItems;
	} else {
		toRet->__golite__maxW = 0;
		toRet->__golite__currItems = 0;
	}
	return toRet;
}
int __golite__seed;

int __golite__randint ();
int __golite__max (int __golite__a, int __golite__b);
void __golite__main();

void __golite__definitions(){
	int tmp__golite__seed = 317;
 __golite__seed = tmp__golite__seed;

}
int __golite__randint ()  {

		{
			int tmp_0_var = ((214013 * __golite__seed) + 2531011);
			__golite__seed = tmp_0_var;
		}
		return ((__golite__seed >> 16) & 32767);
	}


int __golite__max (int __golite__a, int __golite__b)  {
{
			bool tmp_switch_exp28 = (__golite__a > __golite__b);
			if (tmp_switch_exp28 == __golite__true){
								return __golite__a;
			}
			 else if (tmp_switch_exp28 == __golite__false){
								return __golite__b;
			}
			break__lbl28:;
		}
		return __golite__a;
	}


void __golite__main(){
__golite_struct_1 * tmp__golite__myKS = struct_copy1(NULL);
__golite_struct_1 * __golite__myKS = tmp__golite__myKS;

ARRAY tmp__golite__w = make_array(make_type(intType, sizeof(int), NULL, 0), 100);
ARRAY __golite__w = tmp__golite__w;

ARRAY tmp__golite__v = make_array(make_type(intType, sizeof(int), NULL, 0), 100);
ARRAY __golite__v = tmp__golite__v;

int tmp__golite__scale = 1000;
int __golite__scale = tmp__golite__scale;

int tmp__golite__currHigh = (-1);
int __golite__currHigh = tmp__golite__currHigh;


{
	int tmp_0_var = 5000;
	__golite__myKS->__golite__maxW = tmp_0_var;
}
{
		int __golite__i;

	{
		int tmp_0_var = 0;
		__golite__i = tmp_0_var;
	}
while( (__golite__i < len(__golite__w )) ) {
 {

			{
				int tmp_0_var = __golite__randint();
				__golite__seed = tmp_0_var;
			}

			{
				int tmp_0_var = (__golite__seed / __golite__scale);
				*(int * )array_access( __golite__w, __golite__i ) = tmp_0_var;
			}

			{
				int tmp_0_var = __golite__randint();
				__golite__seed = tmp_0_var;
			}

			{
				int tmp_0_var = (__golite__seed / __golite__scale);
				*(int * )array_access( __golite__v, __golite__i ) = tmp_0_var;
			}
			printf("%i", *(int * )array_access( __golite__w, __golite__i ));
printf(" ");
			printf("%i", *(int * )array_access( __golite__v, __golite__i ));
printf("\n");
		}
continue__lbl49:;
(__golite__i)++;
	}
break__lbl49:;

}
ARRAY tmp__golite__m = make_array(make_type(arrayType, sizeof(ARRAY), (make_array(make_type(intType, sizeof(int), NULL, 0), 5001).underlying), 5001), 100);
ARRAY __golite__m = tmp__golite__m;

{
		int __golite__j;

	{
		int tmp_0_var = 0;
		__golite__j = tmp_0_var;
	}
while( (__golite__j <= __golite__myKS->__golite__maxW) ) {
 {

			{
				int tmp_0_var = 0;
				*(int * )array_access( *(ARRAY * )array_access( __golite__m, 0 ), __golite__j ) = tmp_0_var;
			}
		}
continue__lbl56:;
(__golite__j)++;
	}
break__lbl56:;

}
{
		int __golite__i;

	{
		int tmp_0_var = 1;
		__golite__i = tmp_0_var;
	}
while( (__golite__i < 100) ) {
 {
{
					int __golite__j;

				{
					int tmp_0_var = 0;
					__golite__j = tmp_0_var;
				}
while( (__golite__j <= __golite__myKS->__golite__maxW) ) {
 {
						if ((*(int * )array_access( __golite__w, __golite__i ) > __golite__j)) {

							{
								int tmp_0_var = *(int * )array_access( *(ARRAY * )array_access( __golite__m, (__golite__i - 1) ), __golite__j );
								*(int * )array_access( *(ARRAY * )array_access( __golite__m, __golite__i ), __golite__j ) = tmp_0_var;
							}
						} else  {

							{
								int tmp_0_var = __golite__max(*(int * )array_access( *(ARRAY * )array_access( __golite__m, (__golite__i - 1) ), __golite__j ), (*(int * )array_access( *(ARRAY * )array_access( __golite__m, (__golite__i - 1) ), (__golite__j - *(int * )array_access( __golite__w, __golite__i )) ) + *(int * )array_access( __golite__v, __golite__i )));
								*(int * )array_access( *(ARRAY * )array_access( __golite__m, __golite__i ), __golite__j ) = tmp_0_var;
							}
						}
						printf("%s", "items:" );
printf(" ");
						printf("%i", __golite__i);
printf(" ");
						printf("%s", "weight:" );
printf(" ");
						printf("%i", __golite__j);
printf(" ");
						printf("%i", *(int * )array_access( *(ARRAY * )array_access( __golite__m, __golite__i ), __golite__j ));
printf("\n");
						if ((*(int * )array_access( *(ARRAY * )array_access( __golite__m, __golite__i ), __golite__j ) > __golite__currHigh)) {

							{
								int tmp_0_var = *(int * )array_access( *(ARRAY * )array_access( __golite__m, __golite__i ), __golite__j );
								__golite__currHigh = tmp_0_var;
							}
						}
					}
continue__lbl71:;
(__golite__j)++;
				}
break__lbl71:;

}
		}
continue__lbl72:;
(__golite__i)++;
	}
break__lbl72:;

}
printf("%s", "Highest possible value:" );
printf(" ");
printf("%i", __golite__currHigh);
printf("\n");
}

int main () {
	__golite__definitions();
	__golite__main();
	return 0;
}

