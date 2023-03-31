# PipeSum

System operacynjy: Linux
Język: C

Program umożliwia przetwarzanie danych liczbowych, które są przekazywane między procesami w formie tekstowej. Proces 1 odczytuje wiersze składające się z liczb oddzielonych znakiem "+" i przekazuje do procesu 2. Proces 2 weryfikuje, czy każde wyrażenie składa się z liczb oraz znaków "+", a następnie przekazuje poprawne dane do procesu 3. Proces 3 otrzymuje dane, wylicza sumę i wyprowadza wynik w osobnych wierszach.

Przykład poprwanych danych wejściowych:
* 3+5
* 23+9

Aby wysłać zatrzymać/wznowić/zakończyć działanie programu należy wysłać z osobnego terminala odpowiednio sygnały do dowolnego procesu:
*SIGUSR1
*SIGUSR2
*SIGINT
