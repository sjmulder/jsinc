#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <getopt.h>

#ifdef _WIN32
# include <winsock2.h>
#else
# include <arpa/inet.h>
#endif

#define PROG_NAME "jsinc"

#ifdef PROG_VER
# define STR_(x) #x
# define STR(x)  STR_(x)
# define PROG_VER_STR STR(PROG_VER)
#else
# define PROG_VER_STR "dev"
#endif

#define PROG_USAGE                                                 \
"Convert any file into a JavaScript ArrayBuffer.\n\n"              \
"Usage: " PROG_NAME " <options> <file>\n\n"                        \
"Options:\n"                                                       \
"  -v           Print version number and exit.\n"                  \
"  -m <style>   Use module style, see below. Default: global\n"    \
"  -f <format>  Use output format, see below. Default: arraybuf\n" \
"  -p <prefix>  Text to prepend to the output\n"                   \
"  -s <suffix>  Text to append to the output\n"                    \
"  -c           Do not output a generator comment\n\n"             \
"Supported module styles:\n"                                       \
"  global    ; window['<file>'] = ...;\n"                          \
"  amd       ; define('<file>', function() { return  ...; });\n"   \
"  commonjs  module.exports = ...;\n"                              \
"  es6       export default ...;\n"                                \
"  none      ...\n\n"                                              \
"Supported output formats:\n"                                      \
"  arraybuf  ArrayBuffer constructerd using a TypedArray\n"        \
"  array     Array of byte values\n"

struct module_style {
	const char *name;
	const char *prefix_fmt;
	const char *tail;
};

static const struct module_style module_styles[] = {
	{ "none",     "",                                    ""      },
	{ "global",   "; window['%s'] = ",                   ";"     },
	{ "amd",      "; define('%s', function() { return ", "; });" },
	{ "commonjs", "module.exports = ",                   ";"     },
	{ "es6",      "export default ",                     ";"     }
};

enum output_format {
	FORMAT_ARRAYBUF = 1,
	FORMAT_ARRAY
};

static const struct module_style *find_module_style(const char *name)
{
	int i;
	size_t num_styles = sizeof(module_styles) / sizeof(module_styles[0]);

	for (i = 0; i < num_styles; i++) {
		if (!strcmp(module_styles[i].name, name)) {
			return &module_styles[i];
		}
	}

	return NULL;
}

static enum output_format get_output_format(const char *name)
{
	if (!strcmp(name, "arraybuf")) return FORMAT_ARRAYBUF;
	if (!strcmp(name, "array"))    return FORMAT_ARRAY;

	return 0;
}

static void write_arraybuf(FILE *file, const char *comment)
{
	int c, n, i = 0;
	long len;

	printf("new Uint32Array([");
	if (comment) printf("\n\t/* %s */", comment);

	while ((c = fgetc(file)) != EOF) {
		n = c << 24;
		if ((c = fgetc(file)) != EOF) n |= c << 16;
		if ((c = fgetc(file)) != EOF) n |= c << 8;
		if ((c = fgetc(file)) != EOF) n |= c;

		     if (i%6) printf(   ", 0x%08X", (int)htonl(n));
		else if (i)   printf(",\n\t0x%08X", (int)htonl(n));
		else          printf( "\n\t0x%08X", (int)htonl(n));

		i++;
	}

	if (ferror(file)) return;

	len = ftell(file);
	if (len % 4) printf("\n]).buffer.slice(0, %ld)", len);
	else         printf("\n]).buffer");
}

static void write_array(FILE *file, const char *comment)
{
	int c, i = 0;

	printf("[");
	if (comment) printf("\n\t/* %s */", comment);

	while ((c = fgetc(file)) != EOF) {
		     if (i%17) printf(    ",%3d", c);
		else if (i)    printf(",\n\t%3d", c);
		else           printf( "\n\t%3d", c);

		i++;
	}

	if (!ferror(file)) printf("\n]");
}

int main(int argc, char **argv)
{
	const char *filename = NULL;
	const char *prefix = NULL;
	const char *suffix = NULL;
	const char *stylearg = NULL;
	const char *formatarg = NULL;
	const struct module_style *style;
	enum output_format format;
	const char *comment = PROG_NAME "-" PROG_VER_STR;
	int opt;
	FILE *in;

	if (argc < 2) {
		fprintf(stderr, PROG_USAGE);
		exit(EXIT_SUCCESS);
	}

	if (argc > 1 && !strcmp(argv[1], "--version")) {
		printf(PROG_VER_STR "\n");
		exit(EXIT_SUCCESS);
	}

	while ((opt = getopt(argc, argv, "vm:f:p:s:c")) >= 0) {
		switch (opt) {
		case 'v':
			printf(PROG_VER_STR "\n");
			exit(EXIT_SUCCESS);

		case 'm': stylearg = optarg; break;
		case 'f': formatarg = optarg; break;
		case 'p': prefix = optarg; break;
		case 's': suffix = optarg; break;
		case 'c': comment = NULL; break;

		default:
			exit(EXIT_FAILURE);
		}
	}

	if (optind < argc-1) {
		fprintf(stderr, PROG_NAME ": too many arguments given\n");
		exit(EXIT_FAILURE);
	} else if (optind > argc-1) {
		fprintf(stderr, PROG_NAME ": no filename given\n");
		exit(EXIT_FAILURE);
	}

	filename = argv[optind];

	if (!stylearg) stylearg = "global";
	style = find_module_style(stylearg);
	if (!style) {
		fprintf(stderr, PROG_NAME ": unknown module style: %s\n",
			stylearg);
		exit(EXIT_FAILURE);
	}

	if (!formatarg) formatarg = "arraybuf";
	format = get_output_format(formatarg);
	if (!format) {
		fprintf(stderr, PROG_NAME ": unknown output format: %s\n",
			formatarg);
		exit(EXIT_FAILURE);
	}

	in = fopen(filename, "r");
	if (!in) {
		perror(PROG_NAME);
		exit(EXIT_FAILURE);
	}

	if (prefix) printf("%s", prefix);
	else        printf(style->prefix_fmt, filename);


	switch (format) {
	case FORMAT_ARRAYBUF: write_arraybuf(in, comment); break;
	case FORMAT_ARRAY:    write_array(in, comment);    break;

	default:
		fprintf(stderr, PROG_NAME ": don't know how to output in "
			"format %s\n", formatarg);
		exit(EXIT_FAILURE);
	}

	if (ferror(in)) {
		perror("file-js");
		fclose(in);
		exit(EXIT_FAILURE);
	}

	printf("%s\n", suffix ? suffix : style->tail);
	fclose(in);
	return 0;
}
