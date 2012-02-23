DOC = proposal

all:
	latex $(DOC)
	bibtex $(DOC)
	latex $(DOC)
	latex $(DOC)
	dvips $(DOC).dvi
	ps2pdf $(DOC).ps

clean:
	-rm *.pdf *.aux *.log *.blg *.ps *.dvi *.bbl
