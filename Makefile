kilo: kilo.c
	gcc kilo.c -o editor_kilo -Wall -Wextra -pedantic -std=c99
clean:
	rm ./editor_kilo
