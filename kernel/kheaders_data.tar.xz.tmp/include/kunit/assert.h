/* SPDX-License-Identifier: GPL-2.0 */


#ifndef _KUNIT_ASSERT_H
#define _KUNIT_ASSERT_H

#include <linux/err.h>
#include <linux/printk.h>

struct kunit;
struct string_stream;


enum kunit_assert_type {
	KUNIT_ASSERTION,
	KUNIT_EXPECTATION,
};


struct kunit_loc {
	int line;
	const char *file;
};

#define KUNIT_CURRENT_LOC { .file = __FILE__, .line = __LINE__ }


struct kunit_assert {
	void (*format)(const struct kunit_assert *assert,
		       const struct va_format *message,
		       struct string_stream *stream);
};

void kunit_assert_prologue(const struct kunit_loc *loc,
			   enum kunit_assert_type type,
			   struct string_stream *stream);


struct kunit_fail_assert {
	struct kunit_assert assert;
};

void kunit_fail_assert_format(const struct kunit_assert *assert,
			      const struct va_format *message,
			      struct string_stream *stream);


#define KUNIT_INIT_FAIL_ASSERT_STRUCT {					\
	.assert = { .format = kunit_fail_assert_format },		\
}


struct kunit_unary_assert {
	struct kunit_assert assert;
	const char *condition;
	bool expected_true;
};

void kunit_unary_assert_format(const struct kunit_assert *assert,
			       const struct va_format *message,
			       struct string_stream *stream);


#define KUNIT_INIT_UNARY_ASSERT_STRUCT(cond, expect_true) {		       \
	.assert = { .format = kunit_unary_assert_format },		       \
	.condition = cond,						       \
	.expected_true = expect_true					       \
}


struct kunit_ptr_not_err_assert {
	struct kunit_assert assert;
	const char *text;
	const void *value;
};

void kunit_ptr_not_err_assert_format(const struct kunit_assert *assert,
				     const struct va_format *message,
				     struct string_stream *stream);


#define KUNIT_INIT_PTR_NOT_ERR_STRUCT(txt, val) {			       \
	.assert = { .format = kunit_ptr_not_err_assert_format },	       \
	.text = txt,							       \
	.value = val							       \
}


struct kunit_binary_assert_text {
	const char *operation;
	const char *left_text;
	const char *right_text;
};


struct kunit_binary_assert {
	struct kunit_assert assert;
	const struct kunit_binary_assert_text *text;
	long long left_value;
	long long right_value;
};

void kunit_binary_assert_format(const struct kunit_assert *assert,
				const struct va_format *message,
				struct string_stream *stream);


#define KUNIT_INIT_BINARY_ASSERT_STRUCT(format_func,			       \
					text_,				       \
					left_val,			       \
					right_val) {			       \
	.assert = { .format = format_func },				       \
	.text = text_,							       \
	.left_value = left_val,						       \
	.right_value = right_val					       \
}


struct kunit_binary_ptr_assert {
	struct kunit_assert assert;
	const struct kunit_binary_assert_text *text;
	const void *left_value;
	const void *right_value;
};

void kunit_binary_ptr_assert_format(const struct kunit_assert *assert,
				    const struct va_format *message,
				    struct string_stream *stream);


struct kunit_binary_str_assert {
	struct kunit_assert assert;
	const struct kunit_binary_assert_text *text;
	const char *left_value;
	const char *right_value;
};

void kunit_binary_str_assert_format(const struct kunit_assert *assert,
				    const struct va_format *message,
				    struct string_stream *stream);

#endif 
