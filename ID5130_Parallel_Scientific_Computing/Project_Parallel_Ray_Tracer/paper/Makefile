PYTHON ?= python3
TECTONIC ?= tectonic

.PHONY: all plots flowchart pdf clean distclean

all: plots flowchart pdf

plots:
	$(PYTHON) plots/make_plots.py

flowchart:
	$(PYTHON) plots/make_flowchart.py

pdf: report.tex refs.bib
	$(TECTONIC) report.tex

clean:
	rm -f report.aux report.log report.bbl report.blg report.out

distclean: clean
	rm -f report.pdf
	rm -f figures/*.pdf
