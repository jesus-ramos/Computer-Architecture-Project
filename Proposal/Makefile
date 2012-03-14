DOC = proposal

all:
	pdflatex $(DOC)
	bibtex $(DOC)
	pdflatex $(DOC)
	pdflatex $(DOC)

clean:
	-rm *.pdf *.aux *.log *.blg *.ps *.dvi *.bbl
