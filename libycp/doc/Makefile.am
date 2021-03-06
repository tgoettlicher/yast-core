#
# Makefile.am for core/libycp/doc
#
SUBDIRS = ycp

htmldir = $(yastdocdir)/ycp

builtin = $(wildcard $(srcdir)/../src/YCPBuiltin*.cc)
builtinxml := $(builtin:%.cc=%.xml)
xmlfiles = combined.xml
# needed for building the overall documentation
xmlfilesseparated := $(foreach xml,$(builtinxml),$(notdir $(xml)))
htmlfiles = $(filter-out html/index.html,$(wildcard html/*.html html/*/*.html))


html_DATA =			\
	html/yast2docs.css	\
	html/index.html		\
	$(htmlfiles)

html/index.html: $(xmlfilesseparated) $(xmlfiles) builtins.xml
	XML_CATALOG_FILES=@XML_CATALOG@ $(XSLTPROC) $(XSLTPROC_FLAGS) --xinclude @STYLESHEET_HTML@ builtins.xml

YDOC := $(shell pkg-config --variable=ybindir yast2-devtools)/ydoc2
RENT := $(shell pkg-config --variable=ybindir yast2-devtools)/replace_entities

$(xmlfiles) : $(builtin)
	$(YDOC) -o $@.tmp $^
	$(XSLTPROC) --stringparam disambiguate yes $(XSLTPROC_FLAGS) @STYLESHEET_YDOC@ $@.tmp > $@
	$(RENT) $@
	rm $@.tmp

$(xmlfilesseparated) : $(builtin)
	$(YDOC) -o $@.tmp ../src/$(subst xml,cc,$@)
	$(XSLTPROC) $(XSLTPROC_FLAGS) @STYLESHEET_YDOC@ $@.tmp > $@
	$(RENT) $@
	rm $@.tmp

html/yast2docs.css: html/index.html
	cp @STYLESHEET_CSS@ html
	cp -a `pkg-config --variable=yast2dir yast2-devtools`/data/docbook/images html

EXTRA_DIST =  builtins.xml

CLEANFILES = $(xmlfiles) $(xmlfilesseparated)

clean-local:
	rm -rf html

install-data-local:
	mkdir -p $(DESTDIR)$(htmldir)
	cp -a $(srcdir)/html/images $(DESTDIR)$(htmldir)

check-manual:
	xmllint --noout --valid builtins.xml
