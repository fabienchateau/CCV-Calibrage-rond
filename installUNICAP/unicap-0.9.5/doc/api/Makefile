all: index.html

index.html: main.xml
	xsltproc -o html/$@ --xinclude apitohtml.xsl $^


%.html: %.xml
	xsltproc -o html/$@ apitohtml.xsl $^
