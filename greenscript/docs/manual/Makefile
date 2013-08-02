MANUAL=GreenScriptManual

all:
	if command -v latex; then latex ${MANUAL}; latex ${MANUAL}; if command -v dvipdf; then dvipdf ${MANUAL}.dvi; fi; fi

clean:
	rm -f ${MANUAL}.aux ${MANUAL}.log ${MANUAL}.toc

distclean: clean
	rm -f *~ ${MANUAL}.dvi ${MANUAL}.pdf
