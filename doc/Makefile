SRC=./src
USR=./usr

all : tga_usr_doc tga_prg_doc

clean : 
	rm -rf *.pdf *.log *.aux *.dvi *.ps 

tga_usr_doc :
	latex $(SRC)/tga_usr_doc.tex
	dvips tga_usr_doc.dvi -o $(USR)/tga_usr_doc.ps
	
tga_prg_doc :
	doxygen tga_doc_doxy.conf
	doxygen tga_doc_short_doxy.conf

