all: class-diagram.png collaboration.png communication-sequence-normal.png
%.png: %.dia
	dia -e $@ $<
	-optipng $@ -out tmp$@ && mv tmp$@ $@
