# Makefile for data_parser/v0.0.42 plugin

AUTOMAKE_OPTIONS = foreign

PLUGIN_FLAGS = -module -avoid-version --export-dynamic

AM_CPPFLAGS = -I$(top_srcdir) \
	      -I$(top_srcdir)/src/plugin/data_parser/v0.0.42/

pkglib_LTLIBRARIES = data_parser_v0_0_42.la

data_parser_v0_0_42_la_SOURCES = \
	api.h api.c \
	slurmdb_helpers.h slurmdb_helpers.c \
	events.h events.c \
	alloc.h alloc.c \
	openapi.h openapi.c \
	parsing.h parsing.c \
	parsers.h parsers.c
data_parser_v0_0_42_la_DEPENDENCIES =
data_parser_v0_0_42_la_LDFLAGS = $(PLUGIN_FLAGS)
data_parser_v0_0_42_la_LIBADD =
data_parser_v0_0_42_la_CFLAGS = -DDATA_VERSION=v0.0.42 -DPLUGIN_ID=12004 -DPLUGIN_RELEASED=SLURM_24_11_PROTOCOL_VERSION

force:
$(data_parser_v0_0_42_la_LIBADD) : force
	@cd `dirname $@` && $(MAKE) `basename $@`
