all:
	ninja -v -j1 -C build

install:
	DESTDIR=$(DESTDIR) ninja -C build install
