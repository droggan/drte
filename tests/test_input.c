#include <stdlib.h>
#include <stdio.h>

#include "../src/input.h"
#include "test.h"

void
test_valid(void) {
	char buffer[10];
	input_set("hello");
	KeyCode c = input_get(buffer);

	test_assert_int_eql(c, KEY_VALID);
	test_assert_int_eql(buffer[0], 'h');

	c = input_get(buffer);

	test_assert_int_eql(c, KEY_VALID);
	test_assert_int_eql(buffer[0], 'e');
}

void
test_space(void) {
	char buffer[10];
	input_set(" ");
	KeyCode c = input_get(buffer);

	test_assert_int_eql(c, KEY_VALID);
	test_assert_int_eql(buffer[0], ' ');
}

void
test_esc(void) {
	char buffer[10];
	input_set("\x1B");
	KeyCode c = input_get(buffer);

	test_assert_int_eql(c, KEY_ESCAPE);
}

void
test_cursor_up_1(void) {
	char buffer[10];
	input_set("\x1B[A");
	KeyCode c = input_get(buffer);

	test_assert_int_eql(c, KEY_UP);
}

void
test_cursor_up_2(void) {
	char buffer[10];
	input_set("\x1BOA");
	KeyCode c = input_get(buffer);

	test_assert_int_eql(c, KEY_UP);
}

void
test_cursor_down_1(void) {
	char buffer[10];
	input_set("\x1B[B");
	KeyCode c = input_get(buffer);

	test_assert_int_eql(c, KEY_DOWN);
}

void
test_cursor_down_2(void) {
	char buffer[10];
	input_set("\x1BOB");
	KeyCode c = input_get(buffer);

	test_assert_int_eql(c, KEY_DOWN);
}

void
test_cursor_right_1(void) {
	char buffer[10];
	input_set("\x1B[C");
	KeyCode c = input_get(buffer);

	test_assert_int_eql(c, KEY_RIGHT);
}

void
test_cursor_right_2(void) {
	char buffer[10];
	input_set("\x1BOC");
	KeyCode c = input_get(buffer);

	test_assert_int_eql(c, KEY_RIGHT);
}

void
test_cursor_left_1(void) {
	char buffer[10];
	input_set("\x1B[D");
	KeyCode c = input_get(buffer);

	test_assert_int_eql(c, KEY_LEFT);
}

void
test_cursor_left_2(void) {
	char buffer[10];
	input_set("\x1BOD");
	KeyCode c = input_get(buffer);

	test_assert_int_eql(c, KEY_LEFT);
}

void
test_home_1(void) {
	char buffer[10];
	input_set("\x1B[F");
	KeyCode c = input_get(buffer);

	test_assert_int_eql(c, KEY_HOME);
}

void
test_home_2(void) {
	char buffer[10];
	input_set("\x1BOF");
	KeyCode c = input_get(buffer);

	test_assert_int_eql(c, KEY_HOME);
}

void
test_end_1(void) {
	char buffer[10];
	input_set("\x1B[H");
	KeyCode c = input_get(buffer);

	test_assert_int_eql(c, KEY_END);
}

void
test_end_2(void) {
	char buffer[10];
	input_set("\x1BOH");
	KeyCode c = input_get(buffer);

	test_assert_int_eql(c, KEY_END);
}

void
test_home_3(void) {
	char buffer[10];
	input_set("\x1B[7~");
	KeyCode c = input_get(buffer);

	test_assert_int_eql(c, KEY_HOME);
}

void
test_end_3(void) {
	char buffer[10];
	input_set("\x1B[8~");
	KeyCode c = input_get(buffer);

	test_assert_int_eql(c, KEY_END);
}

void
test_insert(void) {
	char buffer[10];
	input_set("\x1B[2~");
	KeyCode c = input_get(buffer);

	test_assert_int_eql(c, KEY_INSERT);
}

void
test_delete(void) {
	char buffer[10];
	input_set("\x1B[3~");
	KeyCode c = input_get(buffer);

	test_assert_int_eql(c, KEY_DELETE);
}

void
test_page_up(void) {
	char buffer[10];
	input_set("\x1B[5~");
	KeyCode c = input_get(buffer);

	test_assert_int_eql(c, KEY_PAGE_UP);
}

void
test_page_down(void) {
	char buffer[10];
	input_set("\x1B[6~");
	KeyCode c = input_get(buffer);

	test_assert_int_eql(c, KEY_PAGE_DOWN);
}

void
test_f1_1(void) {
	char buffer[10];
	input_set("\x1B[11~");
	KeyCode c = input_get(buffer);

	test_assert_int_eql(c, KEY_F1);
}

void
test_f1_2(void) {
	char buffer[10];
	input_set("\x1BOP");
	KeyCode c = input_get(buffer);

	test_assert_int_eql(c, KEY_F1);
}

void
test_f2_1(void) {
	char buffer[10];
	input_set("\x1B[12~");
	KeyCode c = input_get(buffer);

	test_assert_int_eql(c, KEY_F2);
}

void
test_f2_2(void) {
	char buffer[10];
	input_set("\x1BOQ");
	KeyCode c = input_get(buffer);

	test_assert_int_eql(c, KEY_F2);
}

void
test_f3_1(void) {
	char buffer[10];
	input_set("\x1B[13~");
	KeyCode c = input_get(buffer);

	test_assert_int_eql(c, KEY_F3);
}

void
test_f3_2(void) {
	char buffer[10];
	input_set("\x1BOR");
	KeyCode c = input_get(buffer);

	test_assert_int_eql(c, KEY_F3);
}

void
test_f4_1(void) {
	char buffer[10];
	input_set("\x1B[14~");
	KeyCode c = input_get(buffer);

	test_assert_int_eql(c, KEY_F4);
}

void
test_f4_2(void) {
	char buffer[10];
	input_set("\x1BOS");
	KeyCode c = input_get(buffer);

	test_assert_int_eql(c, KEY_F4);
}

void
test_f5(void) {
	char buffer[10];
	input_set("\x1B[15~");
	KeyCode c = input_get(buffer);

	test_assert_int_eql(c, KEY_F5);
}

void
test_f6(void) {
	char buffer[10];
	input_set("\x1B[17~");
	KeyCode c = input_get(buffer);

	test_assert_int_eql(c, KEY_F6);
}

void
test_f7(void) {
	char buffer[10];
	input_set("\x1B[18~");
	KeyCode c = input_get(buffer);

	test_assert_int_eql(c, KEY_F7);
}

void
test_f8(void) {
	char buffer[10];
	input_set("\x1B[19~");
	KeyCode c = input_get(buffer);

	test_assert_int_eql(c, KEY_F8);
}

void
test_f9(void) {
	char buffer[10];
	input_set("\x1B[20~");
	KeyCode c = input_get(buffer);

	test_assert_int_eql(c, KEY_F9);
}

void
test_f10(void) {
	char buffer[10];
	input_set("\x1B[21~");
	KeyCode c = input_get(buffer);

	test_assert_int_eql(c, KEY_F10);
}

void
test_f11(void) {
	char buffer[10];
	input_set("\x1B[23~");
	KeyCode c = input_get(buffer);

	test_assert_int_eql(c, KEY_F11);
}

void
test_f12(void) {
	char buffer[10];
	input_set("\x1B[24~");
	KeyCode c = input_get(buffer);

	test_assert_int_eql(c, KEY_F12);
}

void
test_control_at(void) {
	char buffer[10];
	input_set("\x00");
	KeyCode c = input_get(buffer);

	test_assert_int_eql(c, KEY_CTRL_AT);
}

void
test_control_a(void) {
	char buffer[10];
	input_set("\x01");
	KeyCode c = input_get(buffer);

	test_assert_int_eql(c, KEY_CTRL_A);
}

void
test_control_z(void) {
	char buffer[10];
	input_set("\x1A");
	KeyCode c = input_get(buffer);

	test_assert_int_eql(c, KEY_CTRL_Z);
}

void
test_control_left_bracket(void) {
	char buffer[10];
	input_set("\x1B");
	KeyCode c = input_get(buffer);

	test_assert_int_eql(c, KEY_CTRL_LEFT_BRACKET);
}

void
test_control_backslash(void) {
	char buffer[10];
	input_set("\x1C");
	KeyCode c = input_get(buffer);

	test_assert_int_eql(c, KEY_CTRL_BACKSLASH);
}

void
test_control_right_bracket(void) {
	char buffer[10];
	input_set("\x1D");
	KeyCode c = input_get(buffer);

	test_assert_int_eql(c, KEY_CTRL_RIGHT_BRACKET);
}

void
test_control_caret(void) {
	char buffer[10];
	input_set("\x1E");
	KeyCode c = input_get(buffer);

	test_assert_int_eql(c, KEY_CTRL_CARET);
}

void
test_control_underscore(void) {
	char buffer[10];
	input_set("\x1F");
	KeyCode c = input_get(buffer);

	test_assert_int_eql(c, KEY_CTRL_UNDERSCORE);
}

void
test_control_questionmark(void) {
	char buffer[10];
	input_set("\x7F");
	KeyCode c = input_get(buffer);

	test_assert_int_eql(c, KEY_CTRL_QUESTIONMARK);
}

void
test_alt_a(void) {
	char buffer[10];
	input_set("\x1B\x61");
	KeyCode c = input_get(buffer);

	test_assert_int_eql(c, KEY_ALT_A);
}

void
test_alt_z(void) {
	char buffer[10];
	input_set("\x1B\x7A");
	KeyCode c = input_get(buffer);

	test_assert_int_eql(c, KEY_ALT_Z);
}

// TODO: test more valid sequences
// TODO: test invalid
int
main(void) {
	test_valid();
	test_space();
	test_esc();
	test_cursor_up_1();
	test_cursor_up_2();
	test_cursor_down_1();
	test_cursor_down_2();
	test_cursor_right_1();
	test_cursor_right_2();
	test_cursor_left_1();
	test_cursor_left_2();
	test_home_1();
	test_home_2();
	test_end_1();
	test_end_2();
	test_home_3();
	test_end_3();
	test_insert();
	test_delete();
	test_page_up();
	test_page_down();
	test_f1_1();
	test_f1_2();
	test_f2_1();
	test_f2_2();
	test_f3_1();
	test_f3_2();
	test_f4_1();
	test_f4_2();
	test_f5();
	test_f6();
	test_f7();
	test_f8();
	test_f9();
	test_f10();
	test_f11();
	test_f12();
	test_control_at();
	test_control_a();
	test_control_z();
	test_control_left_bracket();
	test_control_backslash();
	test_control_right_bracket();
	test_control_caret();
	test_control_underscore();
	test_control_questionmark();
	test_alt_a();
	test_alt_z();

	test_print_message();
	return 0;
}
