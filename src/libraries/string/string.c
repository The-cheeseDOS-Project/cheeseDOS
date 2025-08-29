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

#include "string.h"

int kstrcmp(const char *s1, const char *s2) {
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return (unsigned char)*s1 - (unsigned char)*s2;
}

int kstrncmp(const char *s1, const char *s2, size_t n) {
    size_t i;
    for (i = 0; i < n && s1[i] && s1[i] == s2[i]; i++);
    if (i == n) return 0;
    return (unsigned char)s1[i] - (unsigned char)s2[i];
}

size_t kstrlen(const char *s) {
    size_t len = 0;
    while (s[len]) len++;
    return len;
}

char *kstrncpy(char *dest, const char *src, size_t n) {
    size_t i;
    for (i = 0; i < n && src[i]; i++) dest[i] = src[i];
    for (; i < n; i++) dest[i] = '\0';
    return dest;
}

char *kstrcpy(char *dest, const char *src) {
    char *ret = dest;
    while ((*dest++ = *src++));
    return ret;
}

const char *kstrchr(const char *s, int c) {
    while (*s) {
        if (*s == (char)c) return s;
        s++;
    }
    if (c == 0) return s;
    return NULL;
}

int str_to_int(const char *str) {
    int result = 0;
    int sign = 1;
    if (*str == '-') {
        sign = -1;
        str++;
    }
    while (*str) {
        if (*str < '0' || *str > '9') break;
        result = result * 10 + (*str - '0');
        str++;
    }
    return sign * result;
}

void *memcpy(void *dest, const void *src, size_t n) {
    unsigned char *d = dest;
    const unsigned char *s = src;
    while (n--) *d++ = *s++;
    return dest;
}

void *memset(void *s, int c, size_t n) {
    unsigned char *p = s;
    while (n--) *p++ = (unsigned char)c;
    return s;
}

char *kstrtok(char *str, const char *delim) {
    static char *next;
    if (str) next = str;

    if (!next) return NULL;

    while (*next && kstrchr(delim, *next)) next++;
    if (!*next) return NULL;

    char *start = next;

    while (*next && !kstrchr(delim, *next)) next++;

    if (*next) {
        *next = '\0';
        next++;
    } else {
        next = NULL;
    }

    return start;
}

void itoa(int value, char *str, int base) {
    char *ptr = str, *ptr1 = str, tmp_char;
    int tmp_value;

    if (base < 2 || base > 36) {
        *str = '\0';
        return;
    }

    if (value < 0 && base == 10) {
        *ptr++ = '-';
        value = -value;
    }

    do {
        tmp_value = value;
        value /= base;
        *ptr++ = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ"[tmp_value % base];
    } while (value);

    *ptr-- = '\0';

    while (ptr1 < ptr) {
        tmp_char = *ptr;
        *ptr-- = *ptr1;
        *ptr1++ = tmp_char;
    }
}

int memcmp(const void *s1, const void *s2, size_t n) {
    const unsigned char *p1 = s1;
    const unsigned char *p2 = s2;
    for (size_t i = 0; i < n; i++) {
        if (p1[i] != p2[i]) {
            return p1[i] - p2[i];
        }
    }
    return 0;
}
