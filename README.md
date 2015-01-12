
###Zadaniem jest zaimplementowanie prostego systemu plików z jednopoziomowym katalogiem.

Rozmiarem bloku, którzy przyjąłem jest 4kB. Pierwszy blok to superblok, który zawiera informacje o tym ile bloków danych zawartych jest w systemie, oraz ilość dostępnych i-node(czyli plików do utworzenia). Dzięki temu wiemy, z jakimi przesunięciami dostawać się do konkretnych sektorów(i node, dane)
W nim zawarte są również informacje o tym ile jest wolnych i-node oraz bloków na dysku
Następnie w pamięci znajdują się bitmapy ilustrujące zajęte i wolne bloki danych, oraz i-node.(po 1 bloku)
Gdy blok bądź i-node jest wolny, bit ilustrujący jest ustawiony na 0, gdy zajęty - na 1.
W i-node znajduje się informacja o nazwie pliku, informacja o rozmiarze, oraz wskaźniki do bloków pamięci, które zajmuje plik.(stała liczba wskaźników, równa ilości bloków)
####Kopiowanie pliku z minixa na wirtualny system:
Gdy plik będzie kopiowany na dysk, najpierw przeszukujemy tablicę i-node w poszukiwaniu informacji, czy plik o takiej nazwie istnieje. Jeśli istnieje - wykorzystamy ten i-node. Dealokujemy bloki danych zajmowane przez plik, a następnie alokujemy te bloki dla nowego pliku.
Gdy plik nie istnieje - sprawdzamy czy jest wystarczająca ilość wolnych bloków i jest wolny i-node. Gdy jest:
przechodzimy po bitmapie, alokując kolejne bloki danych i zmieniając w bitmapie wartość dla nich na 1, oraz uzupełniając numery bloków w i-node.
####Kopiowanie pliku na minixa z wirtualnego systemu plików:
Sprawdzamy czy jest plik o takiej nazwie, następnie sprawdzamy, czy jest obecnie używany(pozycja w bitmapie).
Jeśli tak - kopiujemy zawartość bloków wskazywanych przez i-node(uważamy na to, aby skopiować tylko tyle, ile zajmuje plik.




