#ifndef DRTE_TEST_H
#define DRTE_TEST_H

/// \file
/// This file include some macros for unit tests.
/// Usage:
/// \code
/// #include <stdio.h>
/// #include <stdlib.h>
/// #include <string.h>
/// \endcode

size_t _assertions = 0;
size_t _fails = 0;


#define test_print_message() {									\
		printf("%s: %zu/%zu\n",									\
			   __FILE__, _assertions - _fails, _assertions);	\
	}

#define test_assert_str_eql(s, t) {									\
		_assertions++;												\
		if (strcmp((s), (t)) != 0) {								\
			printf("%s:%d: Assertion failed. Got: %s Wanted: %s\n",	\
				   __func__, __LINE__, (s), (t));					\
			_fails++;												\
		}															\
	}

#define test_assert_uint_eql(u, v) {								\
		_assertions++;												\
		if ((u) != (v)) {											\
			printf("%s:%d: Assertion failed. Got: %u Wanted: %u\n",	\
				   __func__, __LINE__, (u), (v));					\
			_fails++;												\
		}															\
	}

#define test_assert_size_t_eql(u, v) {									\
		_assertions++;													\
		if ((u) != (v)) {												\
			printf("%s:%d: Assertion failed. Got: %zu Wanted: %zu\n",	\
				   __func__, __LINE__, (u), (v));						\
			_fails++;													\
		}																\
	}


#define test_assert_int_eql(i, j) {									\
		_assertions++;												\
		if ((i) != (j)) {											\
			printf("%s:%d: Assertion failed. Got: %d Wanted: %d\n",	\
				   __func__, __LINE__, (i), (j));					\
			_fails++;												\
		}															\
	}

#define test_assert_ptr_eql(p, q) {									\
		_assertions++;												\
		if ((p) != (q)) {											\
			printf("%s:%d: Assertion failed. Got: %p Wanted: %p\n",	\
				   __func__, __LINE__, (p), (q));					\
			_fails++;												\
		}															\
	}

#define test_assert_null(p) {											\
		_assertions++;													\
		if ((p) != NULL) {												\
			printf("%s:%d: Assertion failed. Got: NULL Wanted: %p\n",	\
				   __func__, __LINE__, (p));							\
			_fails++;													\
		}																\
	}

#define test_assert_not_null(p) {							\
		_assertions++;										\
		if ((p) == NULL) {									\
			printf("%s:%d: Assertion failed. Got NULL.",	\
				   __func__, __LINE__);						\
			_fails++;										\
		}													\
	}

#endif
