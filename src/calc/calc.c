/*
 * cheeseDOS - My x86 DOS
 * Copyright (C) 2025  Connor Thomson
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "calc.h"
#include "string.h"
#include "vga.h"
#include "stdint.h"

#define MAX_DIGITS 128

typedef struct {
    uint32_t digits[MAX_DIGITS];
    int size;
    int sign;
} big32_t;

static void big32_add(const big32_t* a, const big32_t* b, big32_t* result);
static void big32_divmod(const big32_t* a, const big32_t* b, big32_t* quotient, big32_t* remainder);

static void big32_zero(big32_t* num) {
    for (int i = 0; i < MAX_DIGITS; i++) num->digits[i] = 0;
    num->size = 1;
    num->digits[0] = 0;
    num->sign = 1;
}

static void big32_copy(const big32_t* src, big32_t* dest) {
    for (int i = 0; i < src->size; i++) dest->digits[i] = src->digits[i];
    dest->size = src->size;
    dest->sign = src->sign;
}

static int big32_compare(const big32_t* a, const big32_t* b) {
    if (a->size != b->size) return (a->size > b->size) ? 1 : -1;
    for (int i = a->size - 1; i >= 0; i--) {
        if (a->digits[i] != b->digits[i])
            return (a->digits[i] > b->digits[i]) ? 1 : -1;
    }
    return 0;
}

static void big32_parse(big32_t* num, const char* str) {
    big32_zero(num);
    int len = (int)kstrlen(str);
    int i = 0;

    if (str[0] == '-') {
        num->sign = -1;
        i++;
    }

    for (; i < len; i++) {
        char c = str[i];
        if (c < '0' || c > '9') break;

        uint32_t carry = c - '0';
        for (int j = 0; j < num->size || carry; j++) {
            if (j == num->size) num->digits[num->size++] = 0;
            uint64_t val = (uint64_t)num->digits[j] * 10 + carry;
            num->digits[j] = (uint32_t)(val & 0xFFFFFFFF);
            carry = (uint32_t)(val >> 32);
        }
    }

    if (num->size == 0 || (num->size == 1 && num->digits[0] == 0)) {
        num->size = 1;
        num->digits[0] = 0;
        num->sign = 1;
    }
}

static void big32_print(const big32_t* num) {
    big32_t tmp;
    big32_copy(num, &tmp);
    if (tmp.size == 0 || (tmp.size == 1 && tmp.digits[0] == 0)) {
        vga_putchar('0');
        print("\n");
        return;
    }

    int max_dec = MAX_DIGITS * 11;
    uint8_t dec[max_dec];
    for (int i = 0; i < max_dec; ++i) dec[i] = 0;
    int dec_len = 1;

    for (int limb = tmp.size - 1; limb >= 0; --limb) {
        for (int t = 0; t < 32; ++t) {
            int carry = 0;
            for (int i = 0; i < dec_len; ++i) {
                int v = dec[i] * 2 + carry;
                dec[i] = v % 10;
                carry = v / 10;
            }
            while (carry) {
                dec[dec_len++] = carry % 10;
                carry /= 10;
            }
        }

        uint32_t carry32 = tmp.digits[limb];
        int i = 0;
        while (carry32 > 0) {
            if (i >= dec_len) dec[dec_len++] = 0;
            uint32_t adddigit = carry32 % 10;
            uint32_t sum = dec[i] + adddigit;
            dec[i] = sum % 10;
            uint32_t carrynext = sum / 10;
            carry32 /= 10;
            int k = i + 1;
            while (carrynext) {
                if (k >= dec_len) dec[dec_len++] = 0;
                uint32_t s2 = dec[k] + carrynext;
                dec[k] = s2 % 10;
                carrynext = s2 / 10;
                k++;
            }
            i++;
        }
    }

    if (num->sign == -1) vga_putchar('-');
    for (int i = dec_len - 1; i >= 0; --i) vga_putchar('0' + dec[i]);
    print("\n");
}

static void big32_sub(const big32_t* a, const big32_t* b, big32_t* result) {
    if (a->sign != b->sign) {
        big32_t temp_b = *b;
        temp_b.sign *= -1;
        big32_add(a, &temp_b, result);
        return;
    }

    if (big32_compare(a, b) < 0) {
        big32_sub(b, a, result);
        result->sign = -1;
        return;
    }

    big32_zero(result);
    result->sign = a->sign;
    uint64_t borrow = 0;
    for (int i = 0; i < a->size; i++) {
        uint64_t x = a->digits[i];
        uint64_t y = (i < b->size) ? b->digits[i] : 0;
        uint64_t diff = (x - y) - borrow;
        result->digits[i] = (uint32_t)(diff & 0xFFFFFFFF);
        if (diff & 0x100000000ULL) {
            borrow = 1;
        } else {
            borrow = 0;
        }
    }

    result->size = a->size;
    while (result->size > 1 && result->digits[result->size - 1] == 0)
        result->size--;
}

static void big32_add(const big32_t* a, const big32_t* b, big32_t* result) {
    if (a->sign == b->sign) {
        big32_zero(result);
        result->sign = a->sign;
        uint64_t carry = 0;
        int max = (a->size > b->size) ? a->size : b->size;
        for (int i = 0; i < max || carry; i++) {
            uint64_t x = (i < a->size) ? a->digits[i] : 0;
            uint64_t y = (i < b->size) ? b->digits[i] : 0;
            uint64_t sum = x + y + carry;
            result->digits[i] = (uint32_t)(sum & 0xFFFFFFFF);
            carry = sum >> 32;
        }
        result->size = max;
        if (carry)
            result->digits[result->size++] = (uint32_t)carry;
    } else {
        big32_t t = *b;
        t.sign *= -1;
        big32_sub(a, &t, result);
    }
}

static void big32_mul(const big32_t* a, const big32_t* b, big32_t* result) {
    big32_zero(result);
    result->sign = a->sign * b->sign;
    for (int i = 0; i < a->size; i++) {
        uint64_t carry = 0;
        for (int j = 0; j < b->size || carry; j++) {
            if (i + j >= MAX_DIGITS) break;
            uint64_t x = result->digits[i + j];
            uint64_t y = (j < b->size) ? (uint64_t)a->digits[i] * b->digits[j] : 0;
            uint64_t sum = x + y + carry;
            result->digits[i + j] = (uint32_t)(sum & 0xFFFFFFFF);
            carry = sum >> 32;
        }
    }

    result->size = a->size + b->size;
    while (result->size > 1 && result->digits[result->size - 1] == 0)
        result->size--;

    if (result->size == 1 && result->digits[0] == 0)
        result->sign = 1;
}

static void big32_divmod(const big32_t* a, const big32_t* b, big32_t* quotient, big32_t* remainder) {
    big32_zero(quotient);
    big32_zero(remainder);

    if (b->size == 1 && b->digits[0] == 0) return;

    if (big32_compare(a, b) < 0) {
        big32_copy(a, remainder);
        return;
    }

    big32_t current_dividend, temp_remainder, divisor;
    big32_copy(a, &current_dividend);
    big32_copy(b, &divisor);
    current_dividend.sign = divisor.sign = 1;

    for (int i = a->size * 32 - 1; i >= 0; i--) {
        uint64_t carry = 0;
        for (int j = 0; j < remainder->size; j++) {
            uint64_t temp = ((uint64_t)remainder->digits[j] << 1) | carry;
            remainder->digits[j] = (uint32_t)temp;
            carry = temp >> 32;
        }
        if (carry) {
            remainder->digits[remainder->size] = (uint32_t)carry;
            remainder->size++;
        }

        if ((current_dividend.digits[i / 32] >> (i % 32)) & 1) {
            remainder->digits[0] |= 1;
        }

        if (big32_compare(remainder, &divisor) >= 0) {
            big32_sub(remainder, &divisor, &temp_remainder);
            big32_copy(&temp_remainder, remainder);
            quotient->digits[i / 32] |= (1U << (i % 32));
        }
    }

    quotient->size = (a->size * 32 - 1) / 32 + 1;
    while (quotient->size > 1 && quotient->digits[quotient->size - 1] == 0)
        quotient->size--;

    quotient->sign = a->sign * b->sign;
    remainder->sign = a->sign;
}

void calc_command(const char* expr) {
    big32_t a, b, r, mod;
    char op = 0;
    int pos = 0;
    int length = (int)kstrlen(expr);

    while (pos < length && expr[pos] == ' ') pos++;

    int start = pos;
    if (expr[pos] == '-') pos++;
    while (pos < length && expr[pos] >= '0' && expr[pos] <= '9') pos++;
    if (pos == start || (expr[start] == '-' && pos == start + 1)) {
        set_text_color(COLOR_RED, COLOR_BLACK);
        print("Invalid expression\n");
        set_text_color(COLOR_WHITE, COLOR_BLACK);
        return;
    }

    char buf_a[128];
    int len_a = pos - start;
    for (int i = 0; i < len_a; i++) buf_a[i] = expr[start + i];
    buf_a[len_a] = 0;
    big32_parse(&a, buf_a);

    while (pos < length && expr[pos] == ' ') pos++;
    if (pos >= length) {
        set_text_color(COLOR_RED, COLOR_BLACK);
        print("Invalid operator\n");
        set_text_color(COLOR_WHITE, COLOR_BLACK);
        return;
    }

    op = expr[pos++];

    while (pos < length && expr[pos] == ' ') pos++;

    start = pos;
    if (expr[pos] == '-') pos++;
    while (pos < length && expr[pos] >= '0' && expr[pos] <= '9') pos++;
    if (pos == start || (expr[start] == '-' && pos == start + 1)) {
        set_text_color(COLOR_RED, COLOR_BLACK);
        print("Invalid expression\n");
        set_text_color(COLOR_WHITE, COLOR_BLACK);
        return;
    }

    char buf_b[128];
    int len_b = pos - start;
    for (int i = 0; i < len_b; i++) buf_b[i] = expr[start + i];
    buf_b[len_b] = 0;
    big32_parse(&b, buf_b);

    if (op == '+') {
        big32_add(&a, &b, &r);
        big32_print(&r);
    } else if (op == '-') {
        big32_sub(&a, &b, &r);
        big32_print(&r);
    } else if (op == '*') {
        big32_mul(&a, &b, &r);
        big32_print(&r);
    } else if (op == '/') {
        if (b.size == 1 && b.digits[0] == 0) {
            set_text_color(COLOR_RED, COLOR_BLACK);
            print("Error: Division by zero\n");
            set_text_color(COLOR_WHITE, COLOR_BLACK);
            return;
        }
        big32_divmod(&a, &b, &r, &mod);
        big32_print(&r);
    } else if (op == '%') {
        if (b.size == 1 && b.digits[0] == 0) {
            set_text_color(COLOR_RED, COLOR_BLACK);
            print("Error: Division by zero\n");
            set_text_color(COLOR_WHITE, COLOR_BLACK);
            return;
        }
        big32_divmod(&a, &b, &r, &mod);
        big32_print(&mod);
    } else {
        set_text_color(COLOR_RED, COLOR_BLACK);
        print("Unknown operator\n");
        set_text_color(COLOR_WHITE, COLOR_BLACK);
    }
}
