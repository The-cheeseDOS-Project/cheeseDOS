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

#ifndef STRING_H
#define STRING_H

#include "stddef.h"

void *memcpy(void *dest, const void *src, size_t n);
void *memset(void *s, int c, size_t n);
char *kstrtok(char *str, const char *delim);
int kstrcmp(const char *s1, const char *s2);
int kstrncmp(const char *s1, const char *s2, size_t n);
size_t kstrlen(const char *s);
char *kstrncpy(char *dest, const char *src, size_t n);
char *kstrcpy(char *dest, const char *src);
const char *kstrchr(const char *s, int c);
int str_to_int(const char *str);
char *kstrtok(char *str, const char *delim);
void *memset(void *s, int c, size_t n);

#endif
