/*
 * Copy me if you can.
 * by 20h
 */

#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <libgen.h>

#include "arg.h"

char *argv0;

/*
 * Idea taken from:
 *	http://canonical.org/~kragen/strlen-utf8.html
 */
size_t
utf8strlen(char *s)
{
	size_t i;

	i = 0;
	for (; s[0]; s++) {
		if ((s[0] & 0xc0) != 0x80)
			i++;
	}

	return i;
}

char *
utf8strchr(char *s, char *c)
{
	size_t j, cl;

	cl = strlen(c);
	if (cl == 0)
		return NULL;

	for (j = 0; ; s++) {
		if (j > 6)
			return NULL;
		j++;

		if ((s[0] & 0xc0) != 0x80 || s[0] == '\0') {
			if (cl == j) {
				if (!memcmp(&s[-j], c, cl))
					return &s[-j];
			}
			j = 0;

			if (s[0] == '\0')
				break;
		}
	}

	return NULL;
}

char *
utf8substr(char *s, size_t pos, size_t *length)
{
	size_t i, j, rl;
	char *ret;

	if (*length < 1)
		return NULL;

	ret = NULL;
	rl = 0;
	for (i = 0, j = 0; *length > 0; s++) {
		if (j > 6)
			return NULL;
		j++;

		if (ret != NULL)
			rl++;

		if ((s[0] & 0xc0) != 0x80 || s[0] == '\0') {
			if (i >= pos) {
				if (ret == NULL) {
					ret = &s[-j];
					rl = j;
				}
				(*length)--;
			}
			i++;
			j = 0;

			if (s[0] == '\0')
				break;
		}
	}

	*length = rl;
	return ret;
}

size_t
utf8index(char *s, char *chars)
{
	size_t i, j;
	char c[7];

	j = 0;
	for (i = 0; ; s++) {
		if (j > 6)
			return 0;
		j++;

		if ((s[0] & 0xc0) != 0x80 || s[0] == '\0') {
			memset(c, 0, sizeof(c));
			memmove(c, &s[-j], j);
			if (utf8strchr(chars, c))
				return i;
			i++;
			j = 0;

			if (s[0] == '\0')
				break;
		}
	}

	return 0;
}

void
usage(void)
{
	fprintf(stderr, "usage: %s [substr|index|length] str [args ...]\n",
			basename(argv0));
	exit(1);
}

int
main(int argc, char *argv[])
{
	char *s;
	size_t len;

	argv0 = argv[0];

	if (argc < 3)
		usage();

	switch(argv[1][0]) {
	case 'i':
		if (argc < 4)
			usage();
		printf("%ld\n", utf8index(argv[2], argv[3]));
		break;
	case 'l':
		printf("%ld\n", utf8strlen(argv[2]));
		break;
	case 's':
		if (argc < 5)
			usage();
		len = atoi(argv[4]);
		s = utf8substr(argv[2], atoi(argv[3]), &len);
		if (s == NULL)
			return -1;
		printf("%.*s\n", (int)len, s);
		break;
	default:
		usage();
	};

	return 0;
}

