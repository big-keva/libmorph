all:
	cmake -Bbuild -H.
	cmake --build build --config Debug