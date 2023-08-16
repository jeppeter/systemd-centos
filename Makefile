all:
	ninja -v  -C build

install:
	DESTDIR=$(DESTDIR) ninja -C build install
